#include "Framework/FrameworkUtils.h"

std::vector<float> TArrayToStdVector(const TArray<float> &Embedding)
{
    std::vector<float> Result;
    for (float Value : Embedding)
    {
        Result.push_back(Value);
    }
    return Result;
}

TArray<float> StdVectorToTArray(const std::vector<float> &Embedding)
{
    TArray<float> EmbeddingVec;
    if (!Embedding.empty())
    {
        EmbeddingVec.Append(Embedding.data(), Embedding.size());
    }
    return EmbeddingVec;
}

std::map<std::string, std::string> TMapToStdMap(const TMap<FString, FString> &InTMap)
{
    std::map<std::string, std::string> OutStdMap;

    for (auto &Pair : InTMap)
    {
        std::string value = Pair.Value.IsEmpty() ? "" : TCHAR_TO_UTF8(*Pair.Value);
        OutStdMap.insert({TCHAR_TO_UTF8(*Pair.Key), value});
    }
    return OutStdMap;
}
