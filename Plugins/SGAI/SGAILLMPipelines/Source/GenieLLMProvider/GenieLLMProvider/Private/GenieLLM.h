// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "GenieDialog.h"
#include "LLMPipelinesNative/ILLM.h"
#include <unordered_map>

namespace GenieLLMInternal
{

void __QueryCallback(const char *responseStr, const GenieDialog_SentenceCode_t sentenceCode, const void *genie);

class GenieLLMContext : public LLMPipelinesNative::ILLMModelContext
{
  private:
    friend void __QueryCallback(const char *responseStr, const GenieDialog_SentenceCode_t sentenceCode,
                                const void *genie);

  private:
    LLMPipelinesNative::TokenCallbackFunctionType OnTokenCallback = nullptr;
    GenieDialog_Handle_t m_dialogHandle = nullptr;
    GenieSampler_Handle_t m_samplerHandle = nullptr;
    GenieLog_Handle_t m_loggerHandle = nullptr;

  public:
    bool Init(const std::map<std::string, std::string> &configurations) override;
    void Destroy() override;
    bool Query(const std::string &, const LLMPipelinesNative::TokenCallbackFunctionType &) override;
    void Reset() override;
    void Abort() override;
    bool Prefill(const std::string &) override;
    bool Save(const std::string &) override;
    bool Load(const std::string &) override;
    virtual ~GenieLLMContext() override;

  protected:
    bool ApplyLoRA(const std::string &EngineRole, const std::string &AdapterName);
    bool SetLoRAStrength(const std::string &EngineRole, const std::unordered_map<std::string, float> &AlphaValues);
};

class GenieLLMModel : public LLMPipelinesNative::ILLMModel
{
  public:
    virtual ~GenieLLMModel() = default;
    virtual std::unique_ptr<LLMPipelinesNative::ILLMModelContext> CreateContext(
        const std::map<std::string, std::string> &) override;
    virtual bool Init(const std::map<std::string, std::string> &configurations) override;
    virtual void Destroy() override;
    virtual std::map<std::string, std::string> GetConfiguration() override;

  private:
    std::map<std::string, std::string> Configuration;
};

} // namespace GenieLLMInternal
