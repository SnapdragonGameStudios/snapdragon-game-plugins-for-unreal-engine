// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "LLMPipelinesModule.h"

#include "IProviderModuleInterface.h"
#include "LLMPipelinesNative/IEmbedder.h"
#include "LLMPipelinesNative/ILLM.h"
#include "LLMPipelinesNative/IVectorDB.h"

#define LOCTEXT_NAMESPACE "FLLMPipelinesModule"

void FLLMPipelinesModule::StartupModule()
{
}

void FLLMPipelinesModule::ShutdownModule()
{
}

IProviderModuleInterface::~IProviderModuleInterface() = default;

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLLMPipelinesModule, LLMPipelines)
