#pragma once

#include "CoreMinimal.h"
#include <algorithm>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <vector>

class FAudioBufferQueue
{
  public:
    explicit FAudioBufferQueue(size_t capacity) : buffer(capacity), readIndex(0), writeIndex(0), dataSize(0)
    {
    }

    void reset()
    {
        readIndex = writeIndex = dataSize = 0;
    }

    void ProcessAudio(const uint8_t *data, size_t length)
    {
        std::lock_guard<std::mutex> lock(mutex);

        size_t capacity = buffer.size();
        if (length > capacity)
        {
            data += (length - capacity);
            length = capacity;
        }

        size_t firstChunk = std::min(length, capacity - writeIndex);
        size_t secondChunk = length - firstChunk;

        // copy first chunk
        memcpy(&buffer[writeIndex], data, firstChunk);

        writeIndex = (writeIndex + firstChunk) % capacity;

        if (secondChunk > 0)
        {
            memcpy(&buffer[0], data + firstChunk, length - firstChunk);
            writeIndex = secondChunk;
            if (writeIndex > readIndex)
            {
                readIndex = writeIndex;
            }
        }

        dataSize += length;
        dataAvailable.notify_one();
    }

    void read(uint8_t *dest, size_t length)
    {
        std::unique_lock<std::mutex> lock(mutex);

        dataAvailable.wait(lock, [this, length] { return dataSize >= static_cast<size_t>(length); });

        size_t capacity = buffer.size();

        size_t firstChunk = std::min(length, capacity - readIndex);
        size_t secondChunk = length - firstChunk;

        memcpy(dest, &buffer[readIndex], firstChunk);
        if (secondChunk > 0)
        {
            memcpy(dest + firstChunk, &buffer[0], secondChunk);
        }

        readIndex = (readIndex + length) % capacity;
        dataSize -= length;
    }

  private:
    std::vector<uint8_t> buffer;
    size_t readIndex;
    size_t writeIndex;
    size_t dataSize;
    std::mutex mutex;
    std::condition_variable dataAvailable;
};
