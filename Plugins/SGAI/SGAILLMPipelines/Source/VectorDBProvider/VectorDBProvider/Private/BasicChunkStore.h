// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once
#include "IChunkStore.h"
#include <vector>

class BasicChunkStore : public IChunkStore
{
    std::vector<std::string> Chunks;

  public:
    bool Init(const std::string &) override;
    std::string Get(unsigned int index) const override;
};
