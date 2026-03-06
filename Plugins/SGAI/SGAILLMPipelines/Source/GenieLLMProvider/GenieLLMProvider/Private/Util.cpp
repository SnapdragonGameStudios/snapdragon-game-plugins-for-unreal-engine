// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Util.h"
#include <fstream>
#include <sstream>
#include <string>

namespace GenieLLMInternal
{
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
} // namespace GenieLLMInternal
