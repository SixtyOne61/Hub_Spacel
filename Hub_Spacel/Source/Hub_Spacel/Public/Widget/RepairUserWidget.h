// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <functional>
#include "RepairUserWidget.generated.h"

/**
 * 
 */
 // TO DO DEPRECATED
UCLASS()
class HUB_SPACEL_API URepairUserWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	struct SButtonLinkInformation
	{
		class UButton* Button { nullptr };
		bool IsPressed { false };
		FKey InputKey {};
		std::function<void(bool)> Callback {};
	};

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

	void NativeTick(const FGeometry& _myGeometry, float _deltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	FLinearColor BaseBackgroundColor {};

	UPROPERTY(EditAnywhere)
	FLinearColor PressedBackgroundColor {};

private:
	TArray<SButtonLinkInformation> ButtonLinks {};
};
