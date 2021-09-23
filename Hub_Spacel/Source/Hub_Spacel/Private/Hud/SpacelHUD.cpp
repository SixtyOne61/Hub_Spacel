// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelHUD.h"
#include "Factory/SpacelFactory.h"
#include "Blueprint/UserWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"

ASpacelHUD::ASpacelHUD()
{
}

void ASpacelHUD::BeginPlay()
{
    Super::BeginPlay();

    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    APlayerController* playerController = world->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    playerController->bShowMouseCursor = true;

    // add user widget to viewport
    SpacelFactory::createWidget<UUserWidget>(world, this->InGameWidgetClass, false);

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &ASpacelHUD::StartGame);
    }
}

void ASpacelHUD::StartGame(EGameState _state)
{
    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    if (_state == EGameState::InGame)
    {
        APlayerController* playerController = world->GetFirstPlayerController();
        if (!ensure(playerController != nullptr)) return;

        playerController->bShowMouseCursor = false;
        FInputModeGameAndUI mode{};
        mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
        playerController->SetInputMode(mode);
    }
    else if (_state == EGameState::EndGame)
    {
        SpacelFactory::createWidget<UUserWidget>(world, this->EndMenuWidgetClass, false);
    }
}