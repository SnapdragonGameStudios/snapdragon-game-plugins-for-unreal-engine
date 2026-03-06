// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Framework/ToolCallHelpers.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FString UToolCallHelpers::GetToolArgument(const FString &ArgumentsJson, const FString &FieldName, bool &bSuccess)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ArgumentsJson);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField(FieldName))
        {
            bSuccess = true;
            return JsonObject->GetStringField(FieldName);
        }
    }

    bSuccess = false;
    return TEXT("");
}
