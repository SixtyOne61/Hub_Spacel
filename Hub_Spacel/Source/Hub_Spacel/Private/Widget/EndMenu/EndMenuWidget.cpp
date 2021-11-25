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
#include "Enum/SpacelEnum.h"

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

        for (auto player : spacelGameInstance->MetricPlayersData)
        {
            BP_AddPlayer(player);
        }
    }
}

void UEndMenuWidget::BackToMainMenu() const
{
    UGameplayStatics::OpenLevel(this->GetWorld(), "MainMenu");
}

void UEndMenuWidget::Requeue() const
{
    UGameplayStatics::OpenLevel(this->GetWorld(), "MainMenu", true, Option::Requeue);
}