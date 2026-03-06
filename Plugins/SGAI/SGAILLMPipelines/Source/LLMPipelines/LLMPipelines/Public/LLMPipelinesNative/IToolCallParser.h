// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "LLMPipelinesNative/ILLM.h"
#include <string>

namespace LLMPipelinesNative
{
/**
 * Parsed tool call structure
 * Lightweight structure for passing tool call data
 */
struct FToolCall
{
    /** Name of the tool to call */
    std::string ToolName;

    /** Arguments as JSON string */
    std::string ArgumentsJson;

    FToolCall() = default;

    FToolCall(const std::string &InName, const std::string &InArgs) : ToolName(InName), ArgumentsJson(InArgs)
    {
    }

    /** Check if this is a valid tool call */
    bool IsValid() const
    {
        return !ToolName.empty();
    }
};

class IToolCallParser
{
  public:
    /** Parser state machine states */
    enum class EParseState
    {
        NoToolCall,     // Not currently in a tool call
        ToolCallExists, // Currently parsing tool call content
        Unknown         // Tool call completed, waiting for next
    };

    virtual bool Init() = 0;
    virtual void Reset() = 0;
    virtual IToolCallParser::EParseState ProcessToken(LLMPipelinesNative::ESentenceCode code,
                                                      const std::string &Token) = 0;
    virtual std::vector<FToolCall> GetToolRequests() = 0;
    virtual ~IToolCallParser() = default;
    virtual EParseState GetCurrentState() = 0;
};
} // namespace LLMPipelinesNative
