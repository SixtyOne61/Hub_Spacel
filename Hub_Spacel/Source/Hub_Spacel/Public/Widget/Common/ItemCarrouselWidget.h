// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemCarrouselWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UItemCarrouselWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	struct FData
	{
		uint8 m_id;
		FSlateColor m_backgroundColor {};
		FString m_title {};
		FString m_desc {};
		class UTexture2D* m_icon { nullptr };
	} m_data;

public:
	virtual void setupItems(FData const& _data);

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Setup(FSlateColor const& _backgroundColor, FString const& _desc, class UTexture2D * _icon);
};
