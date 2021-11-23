// Fill out your copyright notice in the Description page of Project Settings.


#include "EndMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "Player/MetricComponent.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "Util/SimplyUI.h"
#include "Widget/EndMenu/PlayerWinnerWidget.h"
#include "Hub_SpacelGameInstance.h"

void UEndMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    populate();
}

void UEndMenuWidget::populate()
{
    if (auto spacelGameInstance = this->GetGameInstance<UHub_SpacelGameInstance>())
    {
        for (auto score : spacelGameInstance->ScoresData)
        {
            BP_AddTeam(score.Team, score.Score);
        }

        for (auto player : spacelGameInstance->PlayersData)
        {
            BP_AddPlayer(player.Team.ToString(), player.PlayerName.ToString(), player.LowSkill, player.MediumSkill, player.HightSkill);
        }
    }
}

/*void UEndMenuWidget::FillStat()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if(TeamColorDataAsset == nullptr) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
    {
        FString const& bestTeam = spacelGameState->GetBestTeam();
        int32 score = spacelGameState->GetScore(bestTeam);

        FColor color = TeamColorDataAsset->GetColor<FColor>(bestTeam);
        BP_SetScore(score, color);

        for (auto widget : PlayerWinnerWidgets)
        {
            widget->BP_SetColor(color);
        }

        int8 id { 0 };
        TArray<APlayerState*> const& playerStates { world->GetGameState()->PlayerArray };
        for (auto playerState : playerStates)
        {
            if (playerState != nullptr)
            {
                if (AShipPawn* pawn = playerState->GetPawn<AShipPawn>())
                {
                    if (pawn->Team == *bestTeam)
                    {
                        if (UMetricComponent* component = Cast<UMetricComponent>(pawn->GetComponentByClass(UMetricComponent::StaticClass())))
                        {
                            if (id < PlayerWinnerWidgets.Num())
                            {
                                if (UPlayerWinnerWidget* widget = PlayerWinnerWidgets[id])
                                {
                                    widget->BP_SetStats(component->NbKill, component->TotalScore, component->Precision);
                                    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
                                    {
                                        widget->BP_SetName(spacelPlayerState->GetPlayerName());
                                    }
                                    ++id;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // fill our stat
    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        BP_SetLocalColor(TeamColorDataAsset->GetColor<FColor>(shipPawn->Team.ToString()));
        if (UMetricComponent* component = Cast<UMetricComponent>(shipPawn->GetComponentByClass(UMetricComponent::StaticClass())))
        {
            if (component->HasInit)
            {
                BP_SetLocalStat(component->NbKill, component->Precision, component->TotalScore, FMath::Max(component->EmpPoint, component->TankPoint), component->MatiereWin, component->MatiereUseForRepair);
            }
        }
    }

    FTimerHandle handle;
    world->GetTimerManager().SetTimer(handle, this, &UEndMenuWidget::FillStat, 1.0f, false, 0.0f);
}*/
