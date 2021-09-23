// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillItemWidget : public UUserWidget
{
	GENERATED_BODY()
	

	struct FData
	{
		uint8 m_id;
		FSlateColor m_backgroundColor{};
		FString m_title{};
		FString m_desc{};
		class UTexture2D* m_icon { nullptr };
	} m_data;

public:
	void setupItems(FData const& _data);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_Setup(FSlateColor const& _backgroundColor, FString const& _title, FString const& _desc, class UTexture2D* _icon);
};
