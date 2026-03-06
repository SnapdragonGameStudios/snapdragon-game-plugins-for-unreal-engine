// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace LLMPipelinesNative
{
enum ESentenceCode
{
    SENTENCECODE_COMPLETE = 0,
    /// The string is the beginning of the query/response.
    SENTENCECODE_BEGIN = 1,
    ///
    SENTENCECODE_CONTINUE = 2,
    /// The string is a part of the query/response and not the beginning or end.
    SENTENCECODE_END = 3,
    /// The query has been aborted.
    SENTENCECODE_ABORT = 4,
};

typedef void (*OnInitialized)(bool);
typedef void (*OnTokensGeneratedCallbackFn)(ESentenceCode code, const std::string &, const void *data);
using TokenCallbackFunctionType = std::function<void(ESentenceCode code, const std::string &)>;

class ILLMModelContext
{
  public:
    virtual ~ILLMModelContext() = default;
    virtual bool Init(const std::map<std::string, std::string> &configurations) = 0;
    virtual void Destroy() = 0;
    virtual bool Query(const std::string &, const TokenCallbackFunctionType &) = 0;
    virtual void Reset() = 0;
    virtual bool Prefill(const std::string &) = 0;
    virtual bool Save(const std::string &) = 0;
    virtual void Abort() = 0;
    virtual bool Load(const std::string &) = 0;
};

class ILLMModel
{
  public:
    virtual ~ILLMModel() = default;
    virtual std::unique_ptr<ILLMModelContext> CreateContext(const std::map<std::string, std::string> &) = 0;
    virtual bool Init(const std::map<std::string, std::string> &configurations) = 0;
    virtual void Destroy() = 0;
    virtual std::map<std::string, std::string> GetConfiguration() = 0;
};
} // namespace LLMPipelinesNative
