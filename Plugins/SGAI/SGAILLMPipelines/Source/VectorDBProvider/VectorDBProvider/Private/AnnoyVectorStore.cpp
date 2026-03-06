// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include "AnnoyVectorStore.h"

using namespace LLMPipelinesNative;

AnnoyVectorStore::~AnnoyVectorStore()
{
}

bool AnnoyVectorStore::Init(const std::string &fileName)
{
    bInitialized = false;
    mAnnoyIndex = new AnnoyIndex(1024);
    if (!fileName.empty())
    {
        bInitialized = mAnnoyIndex->load(fileName.c_str());
    }

    return bInitialized;
}

VectorQueryResult AnnoyVectorStore::Query(const VectorEmbedding &input) const
{
    std::vector<VectorQueryResult> results = Query(input, 1);
    return results[0];
}

std::vector<VectorQueryResult> AnnoyVectorStore::Query(const VectorEmbedding &input, unsigned int count) const
{
    std::vector<VectorQueryResult> results;

    if (bInitialized)
    {
        std::vector<int> nnIndices;
        std::vector<float> nnDistances;
        mAnnoyIndex->get_nns_by_vector(const_cast<const float *>(input.data()), count, -1 /*search_k*/, &nnIndices,
                                       &nnDistances);

        for (unsigned int iResult = 0; iResult < count; ++iResult)
        {
            results.emplace_back(nnDistances[iResult], static_cast<unsigned int>(nnIndices[iResult]));
        }
    }

    return results;
}
