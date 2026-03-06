// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "LLMPipelinesNative/IToolCallParser.h"
#include <string>

class FDefaultToolCallParser : public LLMPipelinesNative::IToolCallParser
{
    std::vector<std::string> ToolCallStartToken{"<tool_call>", "<|tool_call|>"};
    std::vector<std::string> ToolCallEndToken{"</tool_call>", "<|/tool_call|>"};
    LLMPipelinesNative::IToolCallParser::EParseState ParseState;
    std::string CurrentToolCallStr;
    std::vector<LLMPipelinesNative::FToolCall> ToolCalls;

  public:
    virtual bool Init() override;
    virtual LLMPipelinesNative::IToolCallParser::EParseState ProcessToken(LLMPipelinesNative::ESentenceCode code,
                                                                          const std::string &Token) override;
    LLMPipelinesNative::FToolCall ParseToolCall(const std::string &JsonStr);
    virtual std::vector<LLMPipelinesNative::FToolCall> GetToolRequests() override;
    virtual ~FDefaultToolCallParser() override;
    virtual void Reset() override;
    EParseState GetCurrentState() override;
};
