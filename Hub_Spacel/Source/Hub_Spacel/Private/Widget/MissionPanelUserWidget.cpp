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
    PanelTitle = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_PanelTitle"));
}

void UMissionPanelUserWidget::addMission(FMission const& _mission)
{
    if (this->VerticalBox != nullptr)
    {
        FString name = "Mission";
        name.Append(FString::FromInt((int)_mission.Type));
        if (UMissionInfoUserWidget* missionWidget = CreateWidget<UMissionInfoUserWidget, UVerticalBox>(this->VerticalBox, this->MissionInfoWidgetClass, *name))
        {
            missionWidget->Type = _mission.Type;
            missionWidget->Title = _mission.MissionTitle;
            missionWidget->startTimer(_mission.DurationValue);

            FString desc = _mission.MissionDesc;
            desc = desc.Replace(*FString("%reward%"), *FString::FromInt(_mission.RewardValue));
            desc = desc.Replace(*FString("%condition%"), *FString::FromInt(_mission.ConditionValue));
            desc = desc.Replace(*FString("%time%"), *(FString::FromInt(_mission.DurationValue) + "s"));
            if (this->TeamColorDataAsset != nullptr && !_mission.Team.IsEmpty())
            {
                FColorsType const& info = this->TeamColorDataAsset->GetColorType(_mission.Team);
                desc = desc.Replace(*FString("%team%"), *info.ShortName);
            }
            missionWidget->BP_SetDesc(desc);

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

void UMissionPanelUserWidget::showMission(bool _show)
{
    if (this->VerticalBox != nullptr)
    {
        TArray<UWidget*> children = this->VerticalBox->GetAllChildren();
        for (auto* child : children)
        {
            if (UMissionInfoUserWidget* missionWidget = Cast<UMissionInfoUserWidget>(child))
            {
                missionWidget->BP_ShowDesc(_show);
            }
        }
    }
}

void UMissionPanelUserWidget::resetTimer(EMission _type)
{
    if (this->VerticalBox != nullptr)
    {
        FString name = "Mission";
        name.Append(FString::FromInt((int)_type));
        if (UMissionInfoUserWidget* widget = SimplyUI::initSafetyFromName<UUserWidget, UMissionInfoUserWidget>(this, *name))
        {
            widget->startTimer(widget->Duration);
        }
    }
}