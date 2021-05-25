// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateColor.h"
#include "Enum/SpacelEnum.h"
#include "UniqueSkillDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UUniqueSkillDataAsset : public UDataAsset
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
	uint32 Value { 0 };

	UPROPERTY(EditAnywhere)
	class UTexture2D* IconeBtn { nullptr };

	UPROPERTY(EditAnywhere)
	class UTexture2D* IconeLarge { nullptr };

	UPROPERTY(EditAnywhere)
	FSlateColor BackgroundColorBtn { };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USkillWidget> SkillWidgetClass{ nullptr };

	UPROPERTY(EditAnywhere)
	FKey Key {};

	UPROPERTY(EditAnywhere)
	FName WidgetName {};

	UPROPERTY(EditAnywhere)
	FString TextWhenFail {};

	UPROPERTY(EditAnywhere)
	FSlateColor BackgroundColorLobby {};

	UPROPERTY(EditAnywhere)
	FString Desc {};
};
