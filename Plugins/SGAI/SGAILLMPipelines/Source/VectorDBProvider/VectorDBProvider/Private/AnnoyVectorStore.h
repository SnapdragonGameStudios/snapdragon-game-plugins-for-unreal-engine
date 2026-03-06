// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#undef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0
#include "IVectorStore.h"
#pragma warning(push)
#pragma warning(disable : 4996)
#include "annoylib.h"
#include "kissrandom.h"
#pragma warning(pop)
#include <string>
#include <vector>

using AnnoyIndex =
    Annoy::AnnoyIndex<int, float, Annoy::DotProduct, Annoy::Kiss32Random, Annoy::AnnoyIndexSingleThreadedBuildPolicy>;

class AnnoyVectorStore : public IVectorStore
{
  private:
    bool bInitialized = false;
    AnnoyIndex *mAnnoyIndex;

  public:
    bool Init(const std::string &) override;
    std::vector<VectorQueryResult> Query(const LLMPipelinesNative::VectorEmbedding &input,
                                         unsigned int count) const override;
    virtual VectorQueryResult Query(const LLMPipelinesNative::VectorEmbedding &input) const override;
    virtual ~AnnoyVectorStore();
};
