// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once
#include <string>

class IChunkStore
{
  public:
    virtual bool Init(const std::string &) = 0;
    virtual std::string Get(unsigned int index) const = 0;
    virtual ~IChunkStore() = default;
};
