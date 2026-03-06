// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "GenieLLM.h"
#include "GenieConfigUtils.h"
#include "GenieLogUtils.h"
#include "Util.h"

using namespace LLMPipelinesNative;

namespace GenieLLMInternal
{

// ** helpers **

static ESentenceCode getCode(const GenieDialog_SentenceCode_t sentenceCode)
{
    if (sentenceCode == GENIE_DIALOG_SENTENCE_BEGIN)
        return SENTENCECODE_BEGIN;
    if (sentenceCode == GENIE_DIALOG_SENTENCE_COMPLETE)
        return SENTENCECODE_COMPLETE;
    if (sentenceCode == GENIE_DIALOG_SENTENCE_ABORT)
        return SENTENCECODE_ABORT;
    if (sentenceCode == GENIE_DIALOG_SENTENCE_END)
        return SENTENCECODE_END;
    if (sentenceCode == GENIE_DIALOG_SENTENCE_CONTINUE)
        return SENTENCECODE_CONTINUE;
    return SENTENCECODE_ABORT;
}

void __QueryCallback(const char *responseStr, const GenieDialog_SentenceCode_t sentenceCode, const void *genie)
{
    const std::string response = responseStr;
    const GenieLLMContext *genieObj = reinterpret_cast<const GenieLLMContext *>(genie);
    if (genieObj != nullptr)
    {
        if (genieObj->OnTokenCallback != nullptr)
        {
            genieObj->OnTokenCallback(getCode(sentenceCode), response);
        }
    }
}

// ** GenieLLMContext implementation **

GenieLLMContext::~GenieLLMContext()
{
    GenieLLMContext::Destroy();
    OnTokenCallback = nullptr;
    m_dialogHandle = nullptr;
    m_samplerHandle = nullptr;
    m_loggerHandle = nullptr;
}

void GenieLLMContext::Abort()
{
    if (m_dialogHandle)
    {
        GenieDialog_signal(m_dialogHandle, GenieDialog_Action_t::GENIE_DIALOG_ACTION_ABORT);
    }
}

bool GenieLLMContext::Init(const std::map<std::string, std::string> &configurations)
{
#if BUILD_FOR_UE
    FScopedDllDirectory ScopedDllDir; // Implemented with FPlatformProcess::PushDllDirectory
#endif

    if (m_dialogHandle == nullptr)
    {
        GenieDialogConfig_Handle_t configHandle = nullptr;

        std::string config = "";
        GenieLog_Level_t logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_VERBOSE;
        bool bLog = false;

        if (configurations.find("config") != configurations.end())
        {
            config = GenieLLMInternal::GetPatchedConfig(configurations.at("config") + "\\" + "llm.json");
        }
        if (configurations.find("logging") != configurations.end())
        {
            bLog = true;
            logLevel = GenieLLMInternal::getLogLevel(configurations.at("logging"));
        }

        if (GENIE_STATUS_SUCCESS == GenieDialogConfig_createFromJson(config.c_str(), &configHandle))
        {
            if (bLog && (GENIE_STATUS_SUCCESS ==
                         GenieLog_create(nullptr, GenieLLMInternal::__GenieLogCallback, logLevel, &m_loggerHandle)))
            {
                GenieDialogConfig_bindLogger(configHandle, m_loggerHandle);
            }

            if (GENIE_STATUS_SUCCESS == GenieDialog_create(configHandle, &m_dialogHandle))
            {
                if (configurations.find("lora") != configurations.end())
                {
                    ApplyLoRA("primary", configurations.at("lora"));
                    std::unordered_map<std::string, float> alphaval;
                    if (configurations.find("lora-strength") != configurations.end())
                    {
                        alphaval.insert(
                            std::pair<std::string, float>("alpha0", std::stof(configurations.at("lora-strength"))));
                    }
                    SetLoRAStrength("primary", alphaval);
                }
            }
            else
            {
                LOG("Failed to create Genie dialog!");
                m_dialogHandle = nullptr;
            }
            GenieDialogConfig_free(configHandle);
        }
        else
        {
            LOG("GenieLLMContext: Failed to parse config file.");
        }
        return (m_dialogHandle != nullptr);
    }

    return false;
}

bool GenieLLMContext::Load(const std::string &file)
{
    bool bResult = false;

    if (m_dialogHandle)
    {
        if (GENIE_STATUS_SUCCESS == GenieDialog_restore(m_dialogHandle, file.c_str()))
        {
            bResult = true;
        }
        else
        {
            LOG("Failed to Load");
        }
    }
    return bResult;
}

bool GenieLLMContext::Save(const std::string &file)
{
    bool bResult = false;

    if (m_dialogHandle)
    {
        if (GENIE_STATUS_SUCCESS == GenieDialog_save(m_dialogHandle, file.c_str()))
        {
            bResult = true;
        }
        else
        {
            LOG("Failed to Save");
        }
    }
    return bResult;
}

bool GenieLLMContext::Prefill(const std::string &Prompt)
{
    bool bResult = false;
    if (m_dialogHandle)
    {
        if (GENIE_STATUS_SUCCESS == GenieDialog_query(m_dialogHandle, Prompt.c_str(),
                                                      GenieDialog_SentenceCode_t::GENIE_DIALOG_SENTENCE_BEGIN,
                                                      __QueryCallback, nullptr))
        {
            bResult = true;
        }
        else
        {
            LOG("Failed to prefill");
        }
    }
    return bResult;
}

bool GenieLLMContext::Query(const std::string &Prompt, const TokenCallbackFunctionType &OnTokensFn)
{
    bool bResult = false;

    if (m_dialogHandle)
    {
        this->OnTokenCallback = OnTokensFn;
        if (GENIE_STATUS_SUCCESS == GenieDialog_query(m_dialogHandle, Prompt.c_str(),
                                                      GenieDialog_SentenceCode_t::GENIE_DIALOG_SENTENCE_COMPLETE,
                                                      __QueryCallback, this))
        {
            bResult = true;
        }
        else
        {
            LOG("Failed to query");
        }
    }
    return bResult;
}

void GenieLLMContext::Destroy()
{
    // Aborts, then frees the Dialog and Logger handles
    if (m_dialogHandle != nullptr)
    {
        GenieDialog_signal(m_dialogHandle, GenieDialog_Action_t::GENIE_DIALOG_ACTION_ABORT);

        if (GENIE_STATUS_SUCCESS != GenieDialog_free(m_dialogHandle))
        {
            LOG("Failed to free the dialog.");
        }

        m_dialogHandle = nullptr;
    }

    if (m_loggerHandle != nullptr)
    {
        GenieLog_free(m_loggerHandle);
        m_loggerHandle = nullptr;
    }
}

void GenieLLMContext::Reset()
{
    if (m_dialogHandle)
    {
        if (GENIE_STATUS_SUCCESS != GenieDialog_reset(m_dialogHandle))
        {
            LOG("Failed to reset.");
        }
    }
}

bool GenieLLMContext::ApplyLoRA(const std::string &EngineRole, const std::string &AdapterName)
{
    int32_t status = GenieDialog_applyLora(m_dialogHandle, EngineRole.c_str(), AdapterName.c_str());
    if (GENIE_STATUS_SUCCESS != status)
    {
        LOG("Failed to apply the LoRA adapter.");
        return false;
    }
    return true;
}

bool GenieLLMContext::SetLoRAStrength(const std::string &EngineRole,
                                      const std::unordered_map<std::string, float> &AlphaValues)
{
    for (auto it = AlphaValues.begin(); it != AlphaValues.end(); it++)
    {
        int32_t status = GenieDialog_setLoraStrength(m_dialogHandle, EngineRole.c_str(), it->first.c_str(), it->second);
        if (GENIE_STATUS_SUCCESS != status)
        {
            LOG("Failed to set the LoRA alpha strength.");
            return false;
        }
    }

    return true;
}

// ** GenieLLMModel implementation **

bool GenieLLMModel::Init(const std::map<std::string, std::string> &config)
{
    this->Configuration = config;
    if (config.find("config") != config.end())
    {
        return true;
    }
    return false;
}

void GenieLLMModel::Destroy()
{
    this->Configuration.clear();
}

std::unique_ptr<ILLMModelContext> GenieLLMModel::CreateContext(const std::map<std::string, std::string> &config)
{
    std::map<std::string, std::string> ContextConfiguration = Configuration;
    for (const auto &pair : config)
    {
        ContextConfiguration.insert(pair);
    }

    std::unique_ptr<GenieLLMContext> ctxt = std::make_unique<GenieLLMContext>();
    if ((ctxt != nullptr) && !ctxt->Init(ContextConfiguration))
    {
        ctxt.reset();
    }
    return ctxt;
}

std::map<std::string, std::string> GenieLLMModel::GetConfiguration()
{
    return Configuration;
}

} // namespace GenieLLMInternal
