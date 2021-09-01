// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CleanFieldWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UCleanFieldWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaTime) override;
protected:
	UFUNCTION()
	void OnKill(int32 _killer, int32 _killed);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Time { 3.0f };

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset{ nullptr };

	UPROPERTY()
	class UVerticalBox* Fields { nullptr };

	uint8 m_currentId { 0 };
	TArray<float> m_timer { };
};
