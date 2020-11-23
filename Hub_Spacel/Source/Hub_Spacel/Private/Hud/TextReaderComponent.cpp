// Fill out your copyright notice in the Description page of Project Settings.
#include "TextReaderComponent.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/HAL/PlatformFileManager.h"

// Sets default values for this component's properties
UTextReaderComponent::UTextReaderComponent()
{
}

FString UTextReaderComponent::ReadFile(FString const& _filePath)
{
    FString const& directoryPath { FPaths::ProjectContentDir() };
    FString const& fullPath { directoryPath + "/" + _filePath };
    FString result {};
    IPlatformFile& file { FPlatformFileManager::Get().GetPlatformFile() };

    if (file.FileExists(*fullPath))
    {
        FFileHelper::LoadFileToString(result, *fullPath);
    }

    return result;
}


