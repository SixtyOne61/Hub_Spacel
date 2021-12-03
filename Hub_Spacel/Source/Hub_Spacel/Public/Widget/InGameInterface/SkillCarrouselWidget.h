// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SkillCarrouselWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChangeCarrousel, ESkill, _skillId, ESkillType, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHoverCarrousel, ESkill, _skillId, ESkillType, _type);

/**
 *  DEPRECATED
 */
UCLASS()
class HUB_SPACEL_API USkillCarrouselWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UInGameWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Setup")
	void SetupCarrousel(ESkillType _type);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Carrousel")
	void BP_CleanCarrousel();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Carrousel")
	void BP_CreateSkillItemCarrousel(class USkillItemWidget*& _widget);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Carrousel")
	void BP_UpdateState(FString const& _txt);

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, BlueprintAssignable)
	FOnChangeCarrousel OnChangeCarrouselDelegate {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, BlueprintAssignable)
	FOnHoverCarrousel OnHoverCarrouselDelegate {};

protected:
	UFUNCTION()
	void OnChooseSkill(ESkill _skillId, ESkillType _type);

	UFUNCTION()
	void OnHoverSkill(ESkill _skillId, ESkillType _type);

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };

	UPROPERTY(Category = "Hack", EditAnywhere, BlueprintReadWrite)
	class UEditorHackDataAsset* HackDataAsset { nullptr };
};
