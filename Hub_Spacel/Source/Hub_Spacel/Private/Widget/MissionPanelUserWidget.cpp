// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionPanelUserWidget.h"
#include "Util/SimplyUI.h"
#include "Widget/MissionInfoUserWidget.h"
#include "Components/VerticalBox.h"
#include "DataAsset/TeamColorDataAsset.h"


void UMissionPanelUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    VerticalBox = SimplyUI::initSafetyFromName<UUserWidget, UVerticalBox>(this, TEXT("Mission_VerticalBox"));
}

void UMissionPanelUserWidget::addMission(FMission _mission)
{
    if (this->VerticalBox != nullptr)
    {
        FString name = "Mission";
        name.Append(FString::FromInt((int)_mission.Type));
        if (UMissionInfoUserWidget* missionWidget = CreateWidget<UMissionInfoUserWidget, UVerticalBox>(this->VerticalBox, this->MissionInfoWidgetClass, *name))
        {
            missionWidget->Type = _mission.Type;
            FString title = _mission.MissionTitle;
            title = title.Replace(*FString("%reward%"), *FString::FromInt(_mission.RewardValue));
            title = title.Replace(*FString("%condition%"), *FString::FromInt(_mission.ConditionValue));

            if (this->TeamColorDataAsset != nullptr && !_mission.Team.IsEmpty())
            {
                FColorsType const& info = this->TeamColorDataAsset->GetColorType(_mission.Team);
                title = title.Replace(*FString("%team%"), *info.ShortName);
            }
            missionWidget->SetTitle(title);

            this->VerticalBox->AddChildToVerticalBox(missionWidget);
        }
    }
}

void UMissionPanelUserWidget::removeMission(EMission _type)
{
    if (this->VerticalBox != nullptr)
    {
        FString name = "Mission";
        name.Append(FString::FromInt((int)_type));
        if (UMissionInfoUserWidget* widget = SimplyUI::initSafetyFromName<UUserWidget, UMissionInfoUserWidget>(this, *name))
        {
            this->VerticalBox->RemoveChild(widget);
        }
    }
}