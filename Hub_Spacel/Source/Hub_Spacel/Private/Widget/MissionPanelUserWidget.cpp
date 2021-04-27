// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionPanelUserWidget.h"
#include "Util/SimplyUI.h"
#include "Widget/MissionInfoUserWidget.h"
#include "Components/VerticalBox.h"


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
            missionWidget->SetTitle(_mission.MissionTitle);

            this->VerticalBox->AddChildToVerticalBox(missionWidget);
        }
    }
}