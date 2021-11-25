// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "Hub_SpacelGameInstance.h"
#include "EndMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEndMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

	/* call on native construct to create child widget */
	void populate();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_AddTeam(FString const& _team, int32 _score);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_AddPlayer(FMetricData const& _metric);
	
	UFUNCTION(BlueprintCallable)
	void BackToMainMenu() const;

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };
};
