// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "EffectWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEffectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetEffect(EEffect _effect);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_Setup(class UTexture2D * _icone, FLinearColor _backgroundColor, FString const& _title, EEffectLocation _location);

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UEffectDataAsset* EffectDataAsset { nullptr };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffect Effect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectLocation Location;
};
