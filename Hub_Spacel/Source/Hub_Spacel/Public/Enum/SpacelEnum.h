// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define TOFLAG(Enum) (1 << static_cast<uint8>(Enum))

namespace Option
{
    static const FString Requeue { "Requeue" };
}

UENUM(BlueprintType)
enum class EGameState : uint8
{
    Undefined = 0,
    Prepare,
    LockLowModule,
    LockMediumModule,
    LockPrepare,
    InGame,
    EndGame,
    WaitEnd,
};

UENUM(BlueprintType)
enum class ESkillReturn : uint8
{
    InternError,
    Success,
    CountDown,
    Unavailable,
    NoMater,
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
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    Hight UMETA(DisplayName = "Hight"),
    Max
};

UENUM(BlueprintType)
enum class ESkill : uint8
{
    Missile UMETA(DisplayName = "Missile"),
    Farmer UMETA(DisplayName = "Farmer"),
    BulletStun UMETA(DisplayName = "BulletStun"),
    EscapeMode UMETA(DisplayName = "EscapeMode"),
    MetaFormAttack UMETA(DisplayName = "MetaFormAttack"),
    MetaFormProtection UMETA(DisplayName = "MetaFormProtection"),
    MetaFormSupport UMETA(DisplayName = "MetaFormSupport"),
    FireRate UMETA(DisplayName = "FireRate"),
    HeavyProtection UMETA(DisplayName = "HeavyProtection"),
    Speedy UMETA(DisplayName = "Speedy"),
    Shotgun UMETA(DisplayName = "Shotgun"),
    HealPack UMETA(DisplayName = "HealPack"),
    Emergency UMETA(DisplayName = "Emergency"),
    Repair UMETA(DisplayName = "Repair"),
    Max,

    DefaultLow = FireRate,
    DefaultMedium = Missile,
    DefaultHight = MetaFormAttack,
};

UENUM(BlueprintType)
enum class EInput : uint8
{
    PrimaryShot UMETA(DisplayName = "PrimaryShot"),
    SecondaryShot UMETA(DisplayName = "SecondaryShot"),
    Forward UMETA(DisplayName = "Forward"),
    Backward UMETA(DisplayName = "Backward"),
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right"),
    FlightAltitudeLeft UMETA(DisplayName = "FlightAltitudeLeft"),
    FlightAltitudeRight UMETA(DisplayName = "FlightAltitudeRight"),
    ShowMission UMETA(DisplayName = "ShowMission"),
    Lock UMETA(DisplayName = "Lock"),
    EscapeMode UMETA(DisplayName = "EscapeMode"),
    MediumSkill UMETA(DisplayName = "MediumSkill"),
    HightSkill UMETA(DisplayName = "HightSkill"),
    HealSkill UMETA(DisplayName = "HealSkill"),
    RepairProtection UMETA(DisplayName = "RepairProtection"),
    RepairEngine UMETA(DisplayName = "RepairEngine"),
    Emergency UMETA(DisplayName = "Emergency"),
    Repair UMETA(DisplayName = "Repair"),
    Unset UMETA(DisplayName = "Unset"),
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
    None UMETA(DisplayName = "None"),
    Fog UMETA(DisplayName = "Fog"),
    Farmer UMETA(DisplayName = "Farmer"),
    Emp UMETA(DisplayName = "Emp"),
    Killed UMETA(DisplayName = "Killed"),
    EscapeMode UMETA(DisplayName = "EscapeMode"),
    BackToGame UMETA(DisplayName = "BackToGame"),
    MetaFormAttack UMETA(DisplayName = "MetaFormAttack"),
    MetaFormProtection UMETA(DisplayName = "MetaFormProtection"),
    MetaFormSupport UMETA(DisplayName = "MetaFormSupport"),
    Respawned UMETA(DisplayName = "Respawned"),
    StartGame UMETA(DisplayName = "StartGame"),
    Gold UMETA(DisplayName = "Gold"),
    PassiveFireRate UMETA(DisplayName = "PassiveFireRate"),
    PassiveCountDown UMETA(DisplayName = "PassiveCountDown"),
    PassiveSpeed UMETA(DisplayName = "PassiveSpeed"),
    Missile UMETA(DisplayName = "Missile"),
    BulletStun UMETA(DisplayName = "BulletStun"),
    HealZone UMETA(DisplayName = "HealZone"),
};

UENUM(BlueprintType)
enum class EEffectLocation : uint8
{
    Skill UMETA(DisplayName = "Skill"),
    Right UMETA(DisplayName = "Right"),
    Left UMETA(DisplayName = "Left")
};

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
    Emp UMETA(DisplayName = "Emp")
};

UENUM(BlueprintType)
enum class EImpactType : uint8
{
    Hit UMETA(DisplayName = "Hit"),
    Obstacle UMETA(DisplayName = "Obstacle")
};

UENUM(BlueprintType)
enum class EMission : uint8
{
    FirstBlood UMETA(DisplayName = "FirstBlood"),
    ScoreRace UMETA(DisplayName = "ScoreRace"),
    EcartType UMETA(DisplayName = "EcartType"),
    Comet UMETA(DisplayName = "Comet"),
    Pirate UMETA(DisplayName = "Pirate"),
    TakeGold UMETA(DisplayName = "TakeGold"),
    HoldGold UMETA(DisplayName = "HoldGold"),
};

UENUM(BlueprintType)
enum class EMetric : uint8
{
    Precision,
    Kill,
    EmpPoint,
    TankPoint,
    MatiereWin,
    MatiereUseForRepair,
    TotalScore,
    Death,
    Assist,
    Heal,
    Destruction,
};

UENUM(BlueprintType)
enum class EMatiereOrigin : uint8
{
    Heal UMETA(DisplayName = "Heal"),
    Farm UMETA(DisplayName = "Farm"),
    Kill UMETA(DisplayName = "Kill"),
    Lost UMETA(DisplayName = "Lost"),
};

UENUM(BlueprintType)
enum class EBuilderType : uint8
{
    Attack UMETA(DisplayName = "Attack"),
    Protection UMETA(DisplayName = "Protection"),
    Engine UMETA(DisplayName = "Engine"),
};

UENUM(BlueprintType)
enum class EFormType : uint8
{
    Base UMETA(DisplayName = "Base"),
    MetaFormAttack UMETA(DisplayName = "MetaFormAttack"),
    MetaFormProtection UMETA(DisplayName = "MetaFormProtection"),
    MetaFormSupport UMETA(DisplayName = "MetaFormSupport"),
    EscapeMode UMETA(DisplayName = "EscapeMode"),
    Missile UMETA(DisplayName = "Missile"),
    Farmer UMETA(DisplayName = "Farmer"),
    BulletStun UMETA(DisplayName = "BulletStun"),
    HealZone UMETA(DisplayName = "HealZone"),
    Max,
};