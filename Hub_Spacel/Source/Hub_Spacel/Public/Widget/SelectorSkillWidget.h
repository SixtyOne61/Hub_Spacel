// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SelectorSkillWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USelectorSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USelectorSkillWidget(FObjectInitializer const& _objectInitializer);

protected:
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnMinButtonClicked();

	UFUNCTION()
	void OnMaxButtonClicked();

public:
	UPROPERTY(EditAnywhere)
	ESkillType SkillType { ESkillType::None };

	UPROPERTY(EditAnywhere)
	FSlateColor OffColor {};

	UPROPERTY(EditAnywhere)
	FSlateColor OnColor {};

private:
	UPROPERTY()
	class UButton* MinButton { nullptr };

	UPROPERTY()
	class UButton* MaxButton { nullptr };

	UPROPERTY()
	TArray<class UButton*> Buttons{ nullptr, nullptr, nullptr };

	UPROPERTY()
	TArray<class UImage*> Images { nullptr, nullptr, nullptr };
};
