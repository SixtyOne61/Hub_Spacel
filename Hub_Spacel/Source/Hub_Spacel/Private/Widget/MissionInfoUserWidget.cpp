// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionInfoUserWidget.h"

void UMissionInfoUserWidget::startTimer(int8 _duration)
{
    if (_duration != 0)
    {
        Duration = _duration;
        m_currentTimer = Duration;

        BP_SetTimer(Title + " " + FString::FromInt(m_currentTimer) + "s");

        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        world->GetTimerManager().ClearAllTimersForObject(this);

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &UMissionInfoUserWidget::UpdateTimer, 1.0f, false);
    }
    else
    {
        BP_SetTitle(Title);
    }
}

void UMissionInfoUserWidget::UpdateTimer()
{
    m_currentTimer -= 1;
    BP_SetTimer(Title + " " + FString::FromInt(m_currentTimer) + "s");

    if (m_currentTimer > 0)
    {
        UWorld* world { this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &UMissionInfoUserWidget::UpdateTimer, 1.0f, false);
    }
}
