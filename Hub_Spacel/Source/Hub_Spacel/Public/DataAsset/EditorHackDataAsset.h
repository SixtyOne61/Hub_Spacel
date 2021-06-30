// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "EditorHackDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEditorHackDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseHack { true };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ChooseModuleTime { 5 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkill LowSkillId { ESkill::FireRate };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkill MediumSkillId { ESkill::Emp };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkill HightSkillId { ESkill::MetaFormSupport };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool MissionSucceedImmediately { true };
};
