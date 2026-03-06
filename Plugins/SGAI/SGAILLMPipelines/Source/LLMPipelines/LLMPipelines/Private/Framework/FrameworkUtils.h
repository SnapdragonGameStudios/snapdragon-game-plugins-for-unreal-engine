// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

std::map<std::string, std::string> TMapToStdMap(const TMap<FString, FString> &InTMap);

std::vector<float> TArrayToStdVector(const TArray<float> &Embedding);

TArray<float> StdVectorToTArray(const std::vector<float> &Embedding);

template <typename TVoidVoidFunc> void Execute(TVoidVoidFunc &&func)
{
    // Execute on background thread
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, MoveTemp(func));
}

template <typename TVoidVoidFunc> void OnGameThread(TVoidVoidFunc &&func)
{
    AsyncTask(ENamedThreads::GameThread, MoveTemp(func));
}
