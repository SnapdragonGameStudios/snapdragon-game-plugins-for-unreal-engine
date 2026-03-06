// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "BasicChunkStore.h"
#include <fstream>
#include <iostream>

bool BasicChunkStore::Init(const std::string &path)
{
    Chunks.clear();
    std::ifstream ifs{path.c_str(), std::ios::binary};

    // Num Chunks
    size_t nChunks = 0;
    ifs.read(reinterpret_cast<char *>(&nChunks), sizeof(nChunks));

    // Read Chunks
    for (size_t iChunk = 0u; iChunk < nChunks; ++iChunk)
    {
        // Read Chunk Size
        size_t chunkSize = 0;
        ifs.read(reinterpret_cast<char *>(&chunkSize), sizeof(chunkSize));

        // Allocate Chunk
        std::vector<char> current;
        current.resize(chunkSize + 1);
        current[chunkSize] = 0;

        // Read Chunk
        ifs.read(current.data(), chunkSize);
        Chunks.emplace_back(current.data());
    }
    ifs.close();

    return true;
}

std::string BasicChunkStore::Get(unsigned int index) const
{
    std::string result;
    if (index < Chunks.size())
    {
        result = Chunks[index];
    }
    return result;
}
