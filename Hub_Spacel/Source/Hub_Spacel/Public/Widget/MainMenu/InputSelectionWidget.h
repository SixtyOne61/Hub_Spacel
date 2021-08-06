// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputSelectionWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UInputSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup();

	UFUNCTION()
	void OnUpdateInput(EInput _input, FKey _key);
	
public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UKeyDataAsset* KeyDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UHideSkillDataAsset* HideSkillDataAsset{ nullptr };

protected:
	TArray<class UInputWidget*> m_inputWidget;
};
