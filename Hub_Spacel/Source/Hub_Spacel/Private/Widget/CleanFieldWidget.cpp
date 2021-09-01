// Fill out your copyright notice in the Description page of Project Settings.


#include "CleanFieldWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Util/SimplyUI.h"
#include "Widget/CleanFieldTextWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include <algorithm>

void UCleanFieldWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
    {
        spacelGameState->OnWhoKillWhoDelegate.AddDynamic(this, &UCleanFieldWidget::OnKill);
    }

    Fields = SimplyUI::initSafetyFromName<UUserWidget, UVerticalBox>(this, TEXT("VerticalBox_Field"));
}

void UCleanFieldWidget::OnKill(int32 _killer, int32 _killed)
{
    if (this->TeamColorDataAsset == nullptr) return;

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    TArray<APlayerState*> const& playerStates { world->GetGameState()->PlayerArray };

    FString killerName, killedName {};
    FSlateColor killerColor, killedColor {};

    for (auto playerState : playerStates)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            int32 playerId = spacelPlayerState->PlayerId;
            FString const& name = spacelPlayerState->GetPlayerName();
            if (playerId == _killer)
            {
                killerName = name;
                killerColor = this->TeamColorDataAsset->GetColor<FSlateColor>(spacelPlayerState->R_Team);
            }
            else if (playerId == _killed)
            {
                killedName = name;
                killedColor = this->TeamColorDataAsset->GetColor<FSlateColor>(spacelPlayerState->R_Team);
            }
        }
    }

    if (this->Fields != nullptr)
    {
        if (UCleanFieldTextWidget* cleanFieldText = Cast<UCleanFieldTextWidget>(this->Fields->GetChildAt(m_currentId)))
        {
            cleanFieldText->BP_SetText(killerName, killerColor, killedName, killedColor);
            cleanFieldText->SetVisibility(ESlateVisibility::Visible);
            ++m_currentId;
            m_timer.Add(this->Time);
        }
    }
}

void UCleanFieldWidget::NativeTick(const FGeometry& _myGeometry, float _deltaTime)
{
    Super::NativeTick(_myGeometry, _deltaTime);

    int i { 0 };
    while (i != m_timer.Num())
    {
        m_timer[i] -= _deltaTime;
        if (m_timer[i] <= 0.0f)
        {
            if (this->Fields != nullptr)
            {
                if (UCleanFieldTextWidget* cleanFieldText = Cast<UCleanFieldTextWidget>(this->Fields->GetChildAt(0)))
                {
                    cleanFieldText->SetVisibility(ESlateVisibility::Hidden);
                    this->Fields->ShiftChild(this->Fields->GetChildrenCount() - 1, cleanFieldText);
                    --m_currentId;
                    m_timer.RemoveAt(i);
                    continue;
                }
            }
        }

        ++i;
    }
}