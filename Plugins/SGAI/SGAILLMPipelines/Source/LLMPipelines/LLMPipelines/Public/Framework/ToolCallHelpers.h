// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LLMPipelinesTypes.h"
#include "ToolCallHelpers.generated.h"

/**
 * Blueprint function library for LLM tool call utilities
 */
UCLASS()
class LLMPIPELINES_API UToolCallHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

  public:
    /**
     * Parse tool arguments JSON to get a specific string field
     * @param ArgumentsJson The JSON arguments string
     * @param FieldName The field name to extract
     * @param bSuccess Whether the field was found and extracted successfully
     * @return The field value, or empty string if not found
     */
    UFUNCTION(BlueprintPure, Category = "LLM|Tool Calls", meta = (DisplayName = "Get Tool Argument"))
    static FString GetToolArgument(const FString &ArgumentsJson, const FString &FieldName, bool &bSuccess);
};
