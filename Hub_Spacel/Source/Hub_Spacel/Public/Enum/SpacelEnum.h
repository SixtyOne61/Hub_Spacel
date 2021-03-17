// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    Undefined = 0,
    Prepare,
    LockPrepare,
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

UENUM(BlueprintType)
enum class ESkill : uint8
{
    RepairProtection UMETA(DisplayName = "RepairProtection"),
    RepairSupport UMETA(DisplayName = "RepairSupport"),
    GiveAlly1 UMETA(DisplayName = "GiveAlly1"),
    GiveAlly2 UMETA(DisplayName = "GiveAlly2"),
    SpecialAttack UMETA(DisplayName = "SpecialAttack"),
    SpecialProtection UMETA(DisplayName = "SpecialProtection"),
    SpecialSupport UMETA(DisplayName = "SpecialSupport"),
    EscapeMode UMETA(DisplayName = "EscapeMode"),
    MetaFormAttack UMETA(DisplayName = "MetaFormAttack"),
    MetaFormProtection UMETA(DisplayName = "MetaFormAttack"),
    MetaFormSupport UMETA(DisplayName = "MetaFormAttack"),
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
enum class EEffect : uint8
{
    None = 0,
    TargetLock = 1 << 0,
    Fog = 1 << 1,
    Shield = 1 << 2,
    Emp = 1 << 3,
    Killed = 1 << 4,
    EscapeMode = 1 << 5
};
ENUM_CLASS_FLAGS(EEffect);

UENUM(BlueprintType)
enum class ECountDown : uint8
{
    Available UMETA(DisplayName = "Available"),
    Ing UMETA(DisplayName = "Ing"),
    CountDown UMETA(DisplayName = "CountDown")
};

UENUM(BlueprintType)
enum class EScoreType : uint8
{
    Kill UMETA(DisplayName = "Kill"),
    Hit UMETA(DisplayName = "Hit"),
    Tank UMETA(DisplayName = "Tank")
};