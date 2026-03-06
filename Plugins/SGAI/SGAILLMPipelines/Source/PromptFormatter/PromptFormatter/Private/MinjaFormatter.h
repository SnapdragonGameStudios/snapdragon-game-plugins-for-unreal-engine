// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "LLMPipelinesNative/IPromptFormatter.h"
#include <memory>
#pragma warning(push)
#pragma warning(disable : 4101)
#include "minja/chat-template.hpp"
#pragma warning(pop)

class FMinjaFormatter : public LLMPipelinesNative::IPromptFormatter
{
  public:
    bool Init(const std::string &path) override;
    std::string Format(const std::string &json, const std::string &tools) override;

  private:
    std::unique_ptr<minja::chat_template> ChatTemplate;
    bool bInitialized = false;
};
