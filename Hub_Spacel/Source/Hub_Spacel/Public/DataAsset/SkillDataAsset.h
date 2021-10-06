// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "Styling/SlateColor.h"
#include "SkillDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	class UUniqueSkillDataAsset * getSKill(ESkill _type);

	TArray<class UUniqueSkillDataAsset*> getSkillByInput(EInput _input);

	TArray<class UUniqueSkillDataAsset*> getSkillByType(ESkillType _type);

public:
	UPROPERTY(EditAnywhere)
	TArray<class UUniqueSkillDataAsset*> Skills;

	UPROPERTY(EditAnywhere)
	FString NotEnoughMater { };

	UPROPERTY(EditAnywhere)
	FString InternError { };

	UPROPERTY(EditAnywhere)
	FString CountDown { };
};
