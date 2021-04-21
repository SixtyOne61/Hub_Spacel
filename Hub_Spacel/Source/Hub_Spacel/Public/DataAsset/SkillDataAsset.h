// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "Styling/SlateColor.h"
#include "SkillDataAsset.generated.h"

USTRUCT()
struct HUB_SPACEL_API FSkill
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ESkill Skill {};

	UPROPERTY(EditAnywhere)
	uint32 CountDown { 0 };

	UPROPERTY(EditAnywhere)
	float FlatDuration { 0.0f };

	UPROPERTY(EditAnywhere)
	class UTexture2D * IconeBtn { nullptr };

	UPROPERTY(EditAnywhere)
	FSlateColor BackgroundColorBtn { };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USkillWidget> SkillWidgetClass { nullptr };

	UPROPERTY(EditAnywhere)
	FKey Key {};

	UPROPERTY(EditAnywhere)
	FName WidgetName {};

	UPROPERTY(EditAnywhere)
	FString TextWhenFail {};
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FSkill getSKill(ESkill _type) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FSkill> Skills;

	UPROPERTY(EditAnywhere)
	uint8 LevelSpecial { 2 };

	UPROPERTY(EditAnywhere)
	uint8 LevelMetaForm { 3 };
};
