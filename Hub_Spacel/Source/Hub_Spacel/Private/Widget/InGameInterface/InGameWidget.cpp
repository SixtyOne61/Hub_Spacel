// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Kismet/GameplayStatics.h"

void UInGameWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UInGameWidget::OnChangeState);
    }
}

void UInGameWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UInGameWidget::OnChangeState(EGameState _state)
{
    if (_state == EGameState::Prepare)
    {
        WaitPlayerState();
    }
}

void UInGameWidget::WaitPlayerState()
{
    ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState());
    if (owningPlayerState == nullptr)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &UInGameWidget::WaitPlayerState, 0.5f, false, 0.0f);
    }
    else
    {
        spawnLobby3D(owningPlayerState);
        setupColor(owningPlayerState);
    }
}

void UInGameWidget::spawnLobby3D(class ASpacelPlayerState const* _owningPlayerState)
{
    if (APawn* pawn = _owningPlayerState->GetPawn())
    {
        TArray<UActorComponent*> const& actors = pawn->GetComponentsByTag(USceneComponent::StaticClass(), "Lobby");
        if (actors.Num() != 0)
        {
            if (USceneComponent* comp = Cast<USceneComponent>(actors[0]))
            {
                FTransform const& transform = comp->GetComponentTransform();
                if (AActor* actor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), this->LobbyClass, transform)))
                {
                    UGameplayStatics::FinishSpawningActor(actor, transform);
                }
            }
        }
    }
}

void UInGameWidget::setupColor(class ASpacelPlayerState const* _owningPlayerState)
{
    FString owningPlayerTeam = _owningPlayerState->R_Team;
    if (this->TeamColorDataAsset != nullptr)
    {
        FSlateColor const& color = this->TeamColorDataAsset->GetColor<FSlateColor>(owningPlayerTeam);
        this->BP_SetupOutline(color);
    }
}