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

void UEndMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TArray<FName> winnerWidget { TEXT("WBP_PlayerWinner1"), TEXT("WBP_PlayerWinner2"), TEXT("WBP_PlayerWinner3") };
    SimplyUI::initArray(this, PlayerWinnerWidgets, winnerWidget);

    FillStat();
}

void UEndMenuWidget::FillStat()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

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
                            if (component->HasInit)
                            {
                                if (id < PlayerWinnerWidgets.Num())
                                {
                                    if (UPlayerWinnerWidget* widget = PlayerWinnerWidgets[id])
                                    {
                                        widget->BP_SetStats(component->NbKill, component->TotalScore, component->Precision);
                                        ++id;
                                    }
                                }
                            }
                            else
                            {
                                FTimerHandle handle;
                                world->GetTimerManager().SetTimer(handle, this, &UEndMenuWidget::FillStat, 1.0f, false, 0.0f);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}
