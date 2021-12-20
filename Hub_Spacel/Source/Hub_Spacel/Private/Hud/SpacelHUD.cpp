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

        pawn->OnFeedbackScoreDelegate.AddDynamic(this, &ASpacelHUD::OnScored);
    }
}

void ASpacelHUD::OnChangeState(EGameState _state)
{
    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    if (_state == EGameState::InGame)
    {
        switchToGameAndUI();
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
            switchToUIOnly();

            UWorld* world = this->GetWorld();
            if (!ensure(world != nullptr)) return;
            SpacelFactory::createWidget<UUserWidget>(world, this->DeathWidgetClass, true);
        }
    }
}

void ASpacelHUD::OnLocalPlayerRemoveEffect(EEffect _effect)
{
    if (_effect == EEffect::Killed)
    {
        switchToGameAndUI();
    }
}

void ASpacelHUD::switchToGameAndUI()
{
    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    APlayerController* playerController = world->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    playerController->bShowMouseCursor = false;

    FInputModeGameAndUI mode{};
    mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
    playerController->SetInputMode(mode);
}

void ASpacelHUD::switchToUIOnly()
{
    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    APlayerController* playerController = world->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    playerController->bShowMouseCursor = true;

    FInputModeUIOnly mode{};
    mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
    playerController->SetInputMode(mode);
}

void ASpacelHUD::Tick(float _deltaSeconde)
{
    Super::Tick(_deltaSeconde);

    BP_UpdateAlpha(m_alpha);
    if (m_alpha != 0.0f)
    {
        m_alpha = FMath::Lerp(1.0f, 0.0f, m_timer / 0.42f);
        m_timer += _deltaSeconde;
    }
}

void ASpacelHUD::OnScored(EScoreType _type, int32 _value)
{
    if (_type == EScoreType::Hit)
    {
        m_alpha = 1.0f;
        m_timer = 0.0f;
    }
}