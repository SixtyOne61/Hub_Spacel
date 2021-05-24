// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicLobby.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "GameFramework/PlayerController.h"

// Sets default values
APhysicLobby::APhysicLobby()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APhysicLobby::BeginPlay()
{
	Super::BeginPlay();

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &APhysicLobby::OnChangeState);
    }

    SetupOwningTeam();
}

// Called every frame
void APhysicLobby::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APhysicLobby::OnChangeState(EGameState _state)
{
    if (_state == EGameState::InGame)
    {
        this->Destroy();
    }
}

void APhysicLobby::SetupOwningTeam()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (APlayerController const* playerController = UGameplayStatics::GetPlayerController(world, 0))
    {
        if (ASpacelPlayerState* owningPlayerState = playerController->GetPlayerState<ASpacelPlayerState>())
        {
            FString owningPlayerTeam = owningPlayerState->R_Team;
            if (this->Colors != nullptr)
            {
                this->BP_TeamColor(this->Colors->GetColor<FColor>(owningPlayerTeam));
                return;
            }
        }
    }

    FTimerHandle handle;
    world->GetTimerManager().SetTimer(handle, this, &APhysicLobby::SetupOwningTeam, 1.0f, false, 0.0f);
}

