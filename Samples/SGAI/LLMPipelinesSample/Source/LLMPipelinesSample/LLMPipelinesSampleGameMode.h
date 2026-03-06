// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LLMPipelinesSampleGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ALLMPipelinesSampleGameMode : public AGameModeBase
{
    GENERATED_BODY()

  public:
    /** Constructor */
    ALLMPipelinesSampleGameMode();
};
