// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
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
	
	UFUNCTION()
	void FillStat();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetScore(int32 _score, FColor const& _color);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetLocalStat(int _kill, int _precision, int _score, int _skillPoint, int _matiereWin, int _matiereUseForRepair);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetLocalColor(FColor const& _color);

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };

protected:
	UPROPERTY()
	TArray<class UPlayerWinnerWidget*> PlayerWinnerWidgets { };
};
