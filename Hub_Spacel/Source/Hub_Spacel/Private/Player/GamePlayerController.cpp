// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerController.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/InputDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "GameMode/FlyingGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Hub_SpacelGameInstance.h"

void AGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAction("Score", IE_Pressed, this, &AGamePlayerController::showScore);
    this->InputComponent->BindAction("Score", IE_Released, this, &AGamePlayerController::hideScore);
}

void AGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (this->IsLocalController())
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
        {
            ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
            if (spacelGameState != nullptr)
            {
                spacelGameState->OnUnlockInputDelegate.AddDynamic(this, &AGamePlayerController::StartGame);
            }

            shipPawn->OnAddEffectDelegate.AddDynamic(this, &AGamePlayerController::OnAddEffect);
            shipPawn->OnRemoveEffectDelegate.AddDynamic(this, &AGamePlayerController::OnRemoveEffect);
        }
    }
}

void AGamePlayerController::OnAddEffect(EEffect _effect)
{
    if (_effect == EEffect::Killed)
    {
        this->UnPossess();
    }
}

void AGamePlayerController::OnRemoveEffect(EEffect _effect)
{
    if (_effect == EEffect::Killed)
    {
        RPCServerPossess();
    }
}

void AGamePlayerController::RPCServerPossess_Implementation()
{
    this->Possess(this->LinkPawn);
}

void AGamePlayerController::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    if (!isAvailable()) return;

    if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
    {
        if (this->IsLocalController())
        {
            FVector hitLoc{ FVector::ZeroVector };
            FVector mouseWorldLocation{}, mouseWorldDirection{};
            TWeakObjectPtr<AActor> hit{};
            bool hasDeproj{}, hasHit{};
            hitResultUnderCursor(hasDeproj, hasHit, hitLoc, mouseWorldLocation, mouseWorldDirection, hit);

            if (hasDeproj)
            {
                this->RPCServerUpdateMouseLocation(mouseWorldLocation, mouseWorldDirection, hitLoc);
                if (hasHit)
                {
                    if (AShipPawn const* hitPawn = Cast<AShipPawn>(hit))
                    {
                        // c'est julie qui a voulu le nom
                        if (ASpacelPlayerState const* yolo = hitPawn->GetPlayerState<ASpacelPlayerState>())
                        {
                            if (yolo->R_Team == shipPawn->Team.ToString())
                            {
                                hitLoc = FVector::ZeroVector;
                            }
                        }
                    }
                }
            }
        }
        else if (this->GetNetMode() == ENetMode::NM_DedicatedServer && shipPawn != nullptr)
        {
            updatePawnData<AShipPawn>(_deltaTime);
        }
    }
}

void AGamePlayerController::RPCServerUpdateMouseLocation_Implementation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
    if (isAvailable())
    {
        updateMouseLocation<AShipPawn>(_loc, _dir, _hitLoc);
    }
}

void AGamePlayerController::RPCServerForward_Implementation(float _val)
{
    if (isAvailable())
    {
        m_data.m_lastForwardInput = _val;
    }
}

void AGamePlayerController::RPCServerHorizontalStraf_Implementation(float _val)
{
    if (isAvailable())
    {
        m_data.m_lastHorizontalStrafInput = _val;
    }
}

void AGamePlayerController::RPCServerVerticalStraf_Implementation(float _val)
{
    if (isAvailable())
    {
        m_data.m_lastVerticalStrafInput = _val;
    }
}

void AGamePlayerController::RPCServerFlightAttitude_Implementation(float _val)
{
    if (isAvailable())
    {
        m_data.m_lastFlightAttitudeInput = _val;
    }
}

void AGamePlayerController::RPCServerFire_Implementation(bool _is)
{
    if (isAvailable())
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
        {
            shipPawn->setFire(_is);
        }
    }
}

void AGamePlayerController::forward(float _value)
{
    if (isAvailable())
    {
        this->RPCServerForward(_value);
    }
}

void AGamePlayerController::horizontalStraf(float _value)
{
    if (isAvailable())
    {
        this->RPCServerHorizontalStraf(_value);
    }
}

void AGamePlayerController::verticalStraf(float _value)
{
    if (isAvailable())
    {
        this->RPCServerVerticalStraf(_value);
    }
}

void AGamePlayerController::flightAttitude(float _value)
{
    if (isAvailable())
    {
        this->RPCServerFlightAttitude(_value);
    }
}

void AGamePlayerController::fireOn()
{
    if (isAvailable())
    {
        this->RPCServerFire(true);
    }
}

void AGamePlayerController::fireOff()
{
    if (isAvailable())
    {
        this->RPCServerFire(false);
    }
}

void AGamePlayerController::lock()
{
    if (isAvailable())
    {
        UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
        spacelGameInstance->OnTryLockDelegate.Broadcast();
    }
}

void AGamePlayerController::showScore()
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr) return;

    shipPawn->OnShowScoreDelegate.Broadcast(true);
}

void AGamePlayerController::hideScore()
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr) return;

    shipPawn->OnShowScoreDelegate.Broadcast(false);
}

void AGamePlayerController::StartGame()
{
    this->RPCServerStartGame();
}

void AGamePlayerController::RPCServerStartGame_Implementation()
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr || shipPawn->PlayerDataAsset == nullptr)
    {
        return;
    }

    auto lb_init = [](UInputDataAsset* _data, DelayValue& _value)
    {
        if (_data != nullptr)
        {
            _value.init(_data->TimeUp, _data->TimeDown, _data->TimeReset);
        }
    };

    lb_init(shipPawn->PlayerDataAsset->ForwardInput, m_data.m_forward);
    lb_init(shipPawn->PlayerDataAsset->HorizontalStrafInput, m_data.m_horizontalStraf);
    lb_init(shipPawn->PlayerDataAsset->VerticalStrafInput, m_data.m_verticalStraf);
    lb_init(shipPawn->PlayerDataAsset->FlightAttitudeInput, m_data.m_flightAttitude);

    R_EnableInput = true;
}

void AGamePlayerController::kill()
{
    this->R_EnableInput = false;
    m_data.reset();

    if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
    {
        shipPawn->RU_PercentSpeed = 0.0f;
        shipPawn->PercentHorizontalStraf = 0.0f;
        shipPawn->PercentVerticalStraf = 0.0f;
        shipPawn->PercentFlightAttitude = 0.0f;
    }
}

void AGamePlayerController::restarted()
{
    this->R_EnableInput = true;
}

bool AGamePlayerController::isAvailable() const
{
    if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
    {
        if (shipPawn->hasEffect(EEffect::Emp)
            || shipPawn->hasEffect(EEffect::BackToGame)) return false;
    }

    return this->R_EnableInput;
}

void AGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGamePlayerController, R_EnableInput);
}
