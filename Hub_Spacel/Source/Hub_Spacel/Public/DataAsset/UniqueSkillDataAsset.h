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
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESkill Skill {};

	UPROPERTY(EditAnywhere)
	uint32 CountDown { 0 };

	UPROPERTY(EditAnywhere)
	float FlatDuration { 0.0f };

	UPROPERTY(EditAnywhere)
	uint32 Value { 0 };

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	class UTexture2D* IconeBtn { nullptr };

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FSlateColor BackgroundColorBtn { };

	UPROPERTY(EditAnywhere)
	FKey Key {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName WidgetName {};

	UPROPERTY(EditAnywhere)
	FString TextWhenFail {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FSlateColor BackgroundColorLobby {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString Desc {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString Title {};

	UPROPERTY(EditAnywhere)
	EInput InputType {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESkillType SkillType {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString VerboseEffect {};

	UPROPERTY(EditAnywhere)
	uint32 MatiereNeeded { 0 };

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	USoundBase* OnClickSound {};
};
