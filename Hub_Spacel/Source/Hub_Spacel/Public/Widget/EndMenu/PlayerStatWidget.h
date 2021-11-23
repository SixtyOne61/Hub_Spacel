// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "PlayerStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPlayerStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Getteur")
	class UTexture2D* GetLogo(ESkill _skill) const;

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset{ nullptr };
};
