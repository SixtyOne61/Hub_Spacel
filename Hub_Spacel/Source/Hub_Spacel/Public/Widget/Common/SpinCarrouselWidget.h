// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Common/ItemCarrouselWidget.h"
#include "SpinCarrouselWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USpinCarrouselWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void setupItems(TArray<UItemCarrouselWidget::FData> const& _data);

	uint8 getIdSelected() const;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetTeamColor(FSlateColor _color);

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetDesc(FString const& _desc);

	UFUNCTION(BlueprintCallable)
	void SpinRight();

	UFUNCTION(BlueprintCallable)
	void SpinLeft();

	void setDesc();

protected:
	TArray<class UItemCarrouselWidget*> Items {};
	class UGridPanel* GridCarrousel { nullptr };
};

