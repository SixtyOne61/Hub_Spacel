// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/MissionDataAsset.h"
#include "MissionPanelUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMissionPanelUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct() override;

public:
	void addMission(FMission const& _mission);
	void removeMission(EMission _type);
	void showMission(bool _show);
	void resetTimer(EMission _type);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Setup(class UTexture2D* _name);

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UMissionInfoUserWidget> MissionInfoWidgetClass { nullptr };

	UPROPERTY()
	class UVerticalBox* VerticalBox { nullptr };

	UPROPERTY()
	class UTextBlock* PanelTitle { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset{ nullptr };
};
