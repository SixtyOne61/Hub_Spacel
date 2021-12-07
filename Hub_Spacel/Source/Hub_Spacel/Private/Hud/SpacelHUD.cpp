// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelHUD.h"
#include "Factory/SpacelFactory.h"
#include "Blueprint/UserWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"

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
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &ASpacelHUD::OnChangeState);
    }

    if (AShipPawn* pawn = playerController->GetPawn<AShipPawn>())
    {
        pawn->OnAddEffectClientDelegate.AddDynamic(this, &ASpacelHUD::OnLocalPlayerAddEffect);
        pawn->OnRemoveEffectDelegate.AddDynamic(this, &ASpacelHUD::OnLocalPlayerRemoveEffect);
    }
}

void ASpacelHUD::OnChangeState(EGameState _state)
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

void ASpacelHUD::OnLocalPlayerAddEffect(EEffect _effect)
{
    if (_effect == EEffect::Killed)
    {
        if (this->DeathWidgetClass != nullptr)
        {
            UWorld* world = this->GetWorld();
            if (!ensure(world != nullptr)) return;

            APlayerController* playerController = world->GetFirstPlayerController();
            if (!ensure(playerController != nullptr)) return;

            playerController->bShowMouseCursor = true;

            FInputModeUIOnly  mode{};
            mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
            playerController->SetInputMode(mode);

            SpacelFactory::createWidget<UUserWidget>(world, this->DeathWidgetClass, true);
        }
    }
}

void ASpacelHUD::OnLocalPlayerRemoveEffect(EEffect _effect)
{
    if (_effect == EEffect::Killed)
    {
        UWorld* world = this->GetWorld();
        if (!ensure(world != nullptr)) return;

        APlayerController* playerController = world->GetFirstPlayerController();
        if (!ensure(playerController != nullptr)) return;

        FInputModeGameAndUI mode{};
        mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
        playerController->SetInputMode(mode);

        playerController->bShowMouseCursor = false;
    }
}
