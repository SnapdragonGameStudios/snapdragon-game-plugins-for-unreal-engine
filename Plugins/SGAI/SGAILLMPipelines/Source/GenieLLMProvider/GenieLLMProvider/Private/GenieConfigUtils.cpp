#include "GenieConfigUtils.h"
#include "Util.h"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace GenieLLMInternal
{
namespace
{
std::string fix_paths(const std::string &llmConfig, const std::string &path)
{
    nlohmann::json j = nlohmann::json::parse(llmConfig, nullptr, false);

    if (!j.is_discarded())
    {
        std::vector<nlohmann::json::json_pointer> paths = {
            nlohmann::json::json_pointer("/dialog/tokenizer/path"),
            nlohmann::json::json_pointer("/dialog/engine/backend/extensions"),
            nlohmann::json::json_pointer("/dialog/engine/model/library/model-bin"),
            nlohmann::json::json_pointer("/dialog/engine/model/binary/ctx-bins"),
            nlohmann::json::json_pointer("/dialog/engine/model/binary/lora/adapters/bin-sections"),
            nlohmann::json::json_pointer("/dialog/engine/embedding/lut-path"),

            nlohmann::json::json_pointer("/embedding/tokenizer/path"),
            nlohmann::json::json_pointer("/embedding/engine/backend/extensions"),
            nlohmann::json::json_pointer("/embedding/engine/model/library/model-bin"),
            nlohmann::json::json_pointer("/embedding/engine/model/binary/ctx-bins"),
            nlohmann::json::json_pointer("/embedding/engine/model/binary/lora/adapters/bin-sections"),
            nlohmann::json::json_pointer("/embedding/engine/embedding/lut-path"),
        };

        for (auto &entry : paths)
        {
            if (j.contains(entry))
            {
                for (auto &it : j[entry])
                {
                    if (it != nullptr)
                    {
                        std::string current_value = it.get<std::string>();
                        std::filesystem::path current_path(current_value);
                        if (current_path.is_relative())
                        {
                            it = path + "\\" + current_value;
                        }
                    }
                }
            }
        }
    }
    return j.dump();
}
} // namespace

std::string GetPatchedConfig(const std::string &absolute_filename)
{
    std::filesystem::path configPath(absolute_filename);
    std::string llmConfig = GenieLLMInternal::LoadFileToString(configPath.string());
    llmConfig = fix_paths(llmConfig, configPath.parent_path().string());
    return llmConfig;
}
} // namespace GenieLLMInternal
