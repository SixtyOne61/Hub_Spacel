// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UScoreUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void initScoreArray();
	void updateScore();

protected:
	void NativeConstruct() override;

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset{ nullptr };

private:
	UPROPERTY()
	class UVerticalBox* ScoresVerticalBox { nullptr };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class URankingUserWidget> RankingWidgetClass { nullptr };
};
