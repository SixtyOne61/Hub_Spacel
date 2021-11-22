// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "TeamStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTeamStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Getteur")
	class UTexture2D* GetLogo(FString const& _team) const;

	UFUNCTION(BlueprintCallable, Category = "Getteur")
	FColor GetColor(FString const& _team) const;

private:
	FColorsType getColorType(FString const& _team) const;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* Colors { nullptr };
};
