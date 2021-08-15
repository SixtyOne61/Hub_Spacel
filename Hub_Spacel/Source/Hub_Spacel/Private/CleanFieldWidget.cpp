// Fill out your copyright notice in the Description page of Project Settings.


#include "CleanFieldWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SpacelPlayerState.h"

void UCleanFieldWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
    {
        spacelGameState->OnWhoKillWhoDelegate.AddDynamic(this, &UCleanFieldWidget::OnKill);
    }
}

void UCleanFieldWidget::OnKill(int32 _killer, int32 _killed)
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    TArray<APlayerState*> const& playerStates { world->GetGameState()->PlayerArray };

    FString killerName, killedName {};

    for (auto playerState : playerStates)
    {
        if (playerState != nullptr)
        {
            int32 playerId = playerState->PlayerId;
            FString const& name = playerState->GetPlayerName();
            if (playerId == _killer)
            {
                killerName = name;
            }
            else if (playerId == _killed)
            {
                killedName = name;
            }
        }
    }
}