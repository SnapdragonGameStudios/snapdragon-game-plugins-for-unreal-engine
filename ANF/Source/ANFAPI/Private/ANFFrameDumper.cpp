//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFFrameDumper.h"
#include "ANFPreProcessor.h"

#include "HAL/Platform.h"
#include "SceneTextureParameters.h"
#include "ScenePrivate.h"
#include "ANFConfig.h"
#include "LegacyScreenPercentageDriver.h"
#include "PlanarReflectionSceneProxy.h"
#include "Serialization/MemoryImage.h"
#include "Serialization/MemoryLayout.h"
#include "FXSystem.h"
#if ENGINE_MINOR_VERSION > 1
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"

void FANFFrameDumper::Execute(
	FRHICommandListImmediate& RHICmdList,
	uint32_t frameNumber,
	FRHITexture* inputColor,
	FRHITexture* inputDepthRaw,
	FRHITexture* inputMotion,
	FRHITexture* outputColor,
	FVector4f jitterInfo,
    FMatrix44f clipToPrevClip,
	FString nameSuffix)
{
    FRHITexture* inputDepth = FANFPreProcessor::Get()->GetDepth32Texture(nameSuffix == FString(TEXT("FG")));
    if (!inputDepth)
    {
        inputDepth = inputDepthRaw;
    }

    RHICmdList.SubmitCommandsAndFlushGPU();

    FGPUFenceRHIRef dumpFences[1] = {};
        dumpFences[0] = RHICreateGPUFence(TEXT("CopyFence"));

        dumpFences[0]->Clear();
    RHICmdList.Transition(FRHITransitionInfo(inputColor, ERHIAccess::Unknown, ERHIAccess::CopySrc));
    RHICmdList.Transition(FRHITransitionInfo(inputDepth, ERHIAccess::Unknown, ERHIAccess::CopySrc));
    RHICmdList.Transition(FRHITransitionInfo(inputMotion, ERHIAccess::Unknown, ERHIAccess::CopySrc));
    RHICmdList.Transition(FRHITransitionInfo(outputColor, ERHIAccess::Unknown, ERHIAccess::CopySrc));

        auto CreateReadbackTexture = [](const FRHITexture* pSrcTexture, const TCHAR* textureName) -> FTextureRHIRef
            {
#if ENGINE_MINOR_VERSION > 0
                const FRHITextureCreateDesc outputDesc =
                    FRHITextureCreateDesc::Create2D(textureName, pSrcTexture->GetSizeX(), pSrcTexture->GetSizeY(), pSrcTexture->GetFormat())
                    .SetFlags(ETextureCreateFlags::CPUReadback | ETextureCreateFlags::HideInVisualizeTexture);
                return RHICreateTexture(outputDesc);
#else
                FRHIResourceCreateInfo CreateInfo = FRHIResourceCreateInfo(TEXT("ReadBack Texture"));
                return FTextureRHIRef(RHICreateTexture2D(pSrcTexture->GetSizeXYZ().X, pSrcTexture->GetSizeXYZ().Y
                    , pSrcTexture->GetFormat(), pSrcTexture->GetNumMips(), pSrcTexture->GetNumSamples(), TexCreate_CPUReadback | TexCreate_HideInVisualizeTexture, CreateInfo).GetReference());
#endif
            };

        FTextureRHIRef sceneColorReadbackTexture = CreateReadbackTexture(inputColor, TEXT("SceneColorReadbackTexture"));
        FTextureRHIRef sceneDepthReadbackTexture = CreateReadbackTexture(inputDepth, TEXT("SceneDepthReadbackTexture"));
        FTextureRHIRef sceneVelocityReadbackTexture = CreateReadbackTexture(inputMotion, TEXT("SceneVelocityReadbackTexture"));
        FTextureRHIRef upscalarOutputReadbackTexture = CreateReadbackTexture(outputColor, TEXT("OutputColorReadbackTexture"));


        dumpFences[0]->Clear();
    RHICmdList.CopyTexture(inputColor, sceneColorReadbackTexture, FRHICopyTextureInfo());
    RHICmdList.CopyTexture(inputDepth, sceneDepthReadbackTexture, FRHICopyTextureInfo());
    RHICmdList.CopyTexture(inputMotion, sceneVelocityReadbackTexture, FRHICopyTextureInfo());
    RHICmdList.CopyTexture(outputColor, upscalarOutputReadbackTexture, FRHICopyTextureInfo());
    RHICmdList.WriteGPUFence(dumpFences[0]);

        auto writeImageDataToFile = [](FRHICommandListImmediate& RHICmdList, const FTextureRHIRef& img, const FGPUFenceRHIRef& fence, const FString baseFileName, const FString nameSuffix, uint32_t frameIndex)
            {
                auto getPackedImageData = [](FRHICommandListImmediate& RHICmdList, const FTextureRHIRef& img, const FGPUFenceRHIRef& fence, void** ppBuffer, size_t* pBufferSize)
                    {
                        check(pBufferSize);
                        check(ppBuffer);

                        uint8* RawBuffer;
                        int32 RowPitchInPixels;
                        int32 Height;
#if ENGINE_MINOR_VERSION > 0
                        FIntVector Dimensions = img->GetDesc().GetSize();
                        EPixelFormat Format = img->GetDesc().Format;
#else
                        FIntVector Dimensions = img->GetSizeXYZ();
                        EPixelFormat Format = img->GetFormat();
#endif
                        RHICmdList.MapStagingSurface(img, fence, (void*&)RawBuffer, RowPitchInPixels, Height);
                        check(RowPitchInPixels >= Dimensions.X);
                        check(Height == Dimensions.Y);
                        check(RawBuffer);

                        const int32 SrcPitch = RowPitchInPixels * GPixelFormats[Format].BlockBytes;
                        const int32 DstPitch = Dimensions.X * GPixelFormats[Format].BlockBytes;
                        const int32 FinalBufferSize = Dimensions.Y * Dimensions.X * GPixelFormats[Format].BlockBytes;

                        *pBufferSize = (size_t)FinalBufferSize;
                        *ppBuffer = FMemory::Malloc(FinalBufferSize);

                        uint8* pTypedBuffer = (uint8*)*ppBuffer;

                        check(*ppBuffer);

                        for (int32 YIndex = 0; YIndex < Dimensions.Y; YIndex++)
                        {
                            const int32 DestIndex = YIndex * DstPitch;
                            const int32 SourceIndex = YIndex * SrcPitch;
                            FMemory::Memcpy(&pTypedBuffer[DestIndex], (const uint8*)&RawBuffer[SourceIndex], DstPitch);
                        }

                        RHICmdList.UnmapStagingSurface(img);
                    };

                auto writeFileData = [](void* pBuffer, size_t bufferSize, const FString baseFileName, const FString nameSuffix, uint32_t frameIndex)
                    {
                        FString numberedFileName = baseFileName + TEXT("_") + FString::FromInt(frameIndex) + TEXT(".raw");
                        FString SubDir = TEXT("/ANF") + nameSuffix + TEXT("Dumps");
                        FString filePath = FPaths::ProjectSavedDir() + SubDir;
                        FString finalFileName = FPaths::Combine(filePath, numberedFileName);
                        FText PathError;
                        if (!FPaths::ValidatePath(finalFileName, &PathError))
                        {
                            UE_LOG(LogRHI, Error, TEXT("FAILED TO WRITE UPSCALAR DUMP"));
                            return;
                        }
                        FArchive* fileArchive = IFileManager::Get().CreateFileWriter(*finalFileName);
                        if (!fileArchive)
                        {
                            UE_LOG(LogRHI, Error, TEXT("ANF frame dump: failed to create file archive for %s"), *finalFileName);
                            return;
                        }
                        fileArchive->Serialize(pBuffer, bufferSize);
                        fileArchive->Close();
                        delete fileArchive;
                    };

                void* imageData = nullptr;
                size_t imageSize = 0;

                getPackedImageData(RHICmdList, img, fence, &imageData, &imageSize);
                writeFileData(imageData, imageSize, baseFileName, nameSuffix, frameIndex);
                FMemory::Free(imageData);
            };

        writeImageDataToFile(RHICmdList, sceneColorReadbackTexture, dumpFences[0], TEXT("inputColor"), nameSuffix, frameNumber);
        writeImageDataToFile(RHICmdList, sceneDepthReadbackTexture, dumpFences[0], TEXT("inputDepth"), nameSuffix, frameNumber);
        writeImageDataToFile(RHICmdList, sceneVelocityReadbackTexture, dumpFences[0], TEXT("inputVelocity"), nameSuffix, frameNumber);
        writeImageDataToFile(RHICmdList, upscalarOutputReadbackTexture, dumpFences[0], TEXT("outputColor"), nameSuffix, frameNumber);


        auto appendImageInfo = [](FString* pStr, const FString imgName, const FTextureRHIRef& img)
            {
#if ENGINE_MINOR_VERSION > 0
                FIntVector Dimensions = img->GetDesc().GetSize();
                EPixelFormat Format = img->GetDesc().Format;
#else
                FIntVector Dimensions = img->GetSizeXYZ();
                EPixelFormat Format = img->GetFormat();
#endif
                (*pStr) += (imgName + TEXT(": "));
                (*pStr) += (FString::FromInt(Dimensions.X) + TEXT("x") + FString::FromInt(Dimensions.Y) + TEXT(" "));
                (*pStr) += GPixelFormats[Format].Name;
                (*pStr) += TEXT("\n");
            };

        FString infoString;
        appendImageInfo(&infoString, TEXT("inputColor"), sceneColorReadbackTexture);
        appendImageInfo(&infoString, TEXT("inputDepth"), sceneDepthReadbackTexture);
        appendImageInfo(&infoString, TEXT("inputVelocity"), sceneVelocityReadbackTexture);
        appendImageInfo(&infoString, TEXT("outputColor"), upscalarOutputReadbackTexture);

        infoString += TEXT("Current Jitter: (") + FString::SanitizeFloat(jitterInfo.X) + TEXT(", ") + FString::SanitizeFloat(jitterInfo.Y) + TEXT(")\n");
        infoString += TEXT("Previous Jitter: (") + FString::SanitizeFloat(jitterInfo.Z) + TEXT(", ") + FString::SanitizeFloat(jitterInfo.W) + TEXT(")\n");

        infoString += TEXT("********************** ClipToPrevClip **********************\n");
        infoString += TEXT("ClipToPrevClip:\n");
        infoString += clipToPrevClip.ToString() + TEXT("\n");
        infoString += TEXT("*************************************************************\n");

        // Write Text Data
        {
            FString numberedFileName = TEXT("anfFrameInfo_") + FString::FromInt(frameNumber) + TEXT(".txt");
            FString SubDir = TEXT("/ANF") + nameSuffix + TEXT("Dumps");
            FString filePath = FPaths::ProjectSavedDir() + SubDir;
            FString finalFileName = FPaths::Combine(filePath, numberedFileName);
            FText PathError;
            if (!FPaths::ValidatePath(finalFileName, &PathError))
            {
                UE_LOG(LogRHI, Error, TEXT("FAILED TO WRITE UPSCALAR INFO FILE"));
                return;
            }
            FArchive* fileArchive = IFileManager::Get().CreateFileWriter(*finalFileName);
            if (!fileArchive)
            {
                UE_LOG(LogRHI, Error, TEXT("ANF frame dump: failed to create info file archive for %s"), *finalFileName);
                return;
            }
            fileArchive->Serialize(TCHAR_TO_ANSI(*infoString), infoString.Len());
            fileArchive->Close();
            delete fileArchive;
        }
}