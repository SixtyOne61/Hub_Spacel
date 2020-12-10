// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    Undefined = 0,
    Prepare,
    InGame
};

UENUM(BlueprintType)
enum class EUIType : uint8
{
    None UMETA(DisplayName = "None"),
    MainMenu UMETA(DisplayName = "MainMenu"),
};

UENUM(BlueprintType)
enum class ERegionType : uint8
{
    None UMETA(DisplayName = "None"),
    NA UMETA(DisplayName = "NA"),
    EU UMETA(DisplayName = "EU"),
    OC UMETA(DisplayName = "OC"),
    ASIA UMETA(DisplayName = "ASIA"),
};

UENUM(BlueprintType)
enum class ESkillType : uint8
{
    None UMETA(DisplayName = "None"),
    Attack UMETA(DisplayName = "Attack"),
    Protection UMETA(DisplayName = "Protection"),
    Support UMETA(DisplayName = "Support")
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFace : uint8
{
    None = 0,
    Top = 1 << 0,
    Bot = 1 << 1,
    Front = 1 << 2,
    Back = 1 << 3,
    Right = 1 << 4,
    Left = 1 << 5,
};
ENUM_CLASS_FLAGS(EFace);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EShipModuleType : uint8
{
    Empty = 0x00,
    BaseDefault = 0x01,
    EngineDefault = 0x02,
    EngineSmall = 0x04,
    ShellDefault = 0x08,
};