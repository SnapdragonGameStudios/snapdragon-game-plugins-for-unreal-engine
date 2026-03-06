// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "MinjaFormatter.h"
#include "PromptFormatterInstance.h"
#include <fstream>
#include <sstream>

using namespace LLMPipelinesNative;

namespace PromptFormatter
{
std::unique_ptr<LLMPipelinesNative::IPromptFormatter> CreateInstance()
{
    return std::make_unique<FMinjaFormatter>();
}
} // namespace PromptFormatter

std::string LoadFileToString(std::string filename)
{
    std::stringstream buffer;
    std::ifstream file(filename);
    if (file.is_open())
    {
        buffer << file.rdbuf();
        file.close();
    }
    return buffer.str();
}

bool FMinjaFormatter::Init(const std::string &configPath)
{
    bInitialized = false;

    std::string tokenizerConfig = LoadFileToString(configPath + "\\tokenizer_config.json");
    nlohmann::json j = nlohmann::json::parse(tokenizerConfig, nullptr, false);

    nlohmann::json::json_pointer t("/chat_template");
    nlohmann::json::json_pointer bos("/bos_token");
    nlohmann::json::json_pointer eos("/eos_token");

    if (!j.is_discarded())
    {
        std::string ChatTemplateStr = j.contains(t) && j[t].is_string() ? j[t] : "";
        std::string BosToken = j.contains(bos) && j[bos].is_string() ? j[bos] : "";
        std::string EosToken = j.contains(eos) && j[eos].is_string() ? j[eos] : "";
        try
        {
            this->ChatTemplate = std::make_unique<minja::chat_template>(ChatTemplateStr, BosToken, EosToken);
        }
        catch (const std::exception &e)
        {
            (void)(e);
            return false;
        }

        bInitialized = true;
        return true;
    }

    return false;
}

std::string FMinjaFormatter::Format(const std::string &jsonStr, const std::string &tools)
{
    std::string result = "";
    if (bInitialized)
    {
        minja::chat_template_inputs input;
        input.messages = json::parse(jsonStr);
        if (!tools.empty())
        {
            input.tools = json::parse(tools);
        }
        result = ChatTemplate->apply(input);
    }
    return result;
}
