// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SkillCarrouselWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillCarrouselWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Setup")
	void SetupCarrousel(ESkillType _type);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Carrousel")
	void BP_CleanCarrousel();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Carrousel")
	void BP_CreateSkillItemCarrousel(class USkillItemWidget*& _widget);

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };
};
