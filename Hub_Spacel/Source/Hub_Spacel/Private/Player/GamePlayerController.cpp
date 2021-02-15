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

void AGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAxis("Forward", this, &AGamePlayerController::forward);
    this->InputComponent->BindAxis("HorizontalStraf", this, &AGamePlayerController::horizontalStraf);
    this->InputComponent->BindAxis("VerticalStraf", this, &AGamePlayerController::verticalStraf);
    this->InputComponent->BindAxis("FlightAttitude", this, &AGamePlayerController::flightAttitude);

    this->InputComponent->BindAction("EscapeMode", IE_Pressed, this, &AGamePlayerController::triggerEscapeMode);
    this->InputComponent->BindAction("Fire", IE_Pressed, this, &AGamePlayerController::fireOn);
    this->InputComponent->BindAction("Fire", IE_Released, this, &AGamePlayerController::fireOff);
    this->InputComponent->BindAction("ReturnToMainMenu", IE_Pressed, this, &AGamePlayerController::returnToMainMenu);
    this->InputComponent->BindAction("RepairProtection", IE_Pressed, this, &AGamePlayerController::repairProtection);
    this->InputComponent->BindAction("RepairSupport", IE_Pressed, this, &AGamePlayerController::repairSupport);
    this->InputComponent->BindAction("GiveAlly1", IE_Pressed, this, &AGamePlayerController::giveAlly1);
    this->InputComponent->BindAction("GiveAlly2", IE_Pressed, this, &AGamePlayerController::giveAlly2);
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
                spacelGameState->OnStartGameDelegate.AddDynamic(this, &AGamePlayerController::StartGame);
            }
        }
    }
}

void AGamePlayerController::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    if (!this->R_EnableInput) return;

    AShipPawn* shipPawn { Cast<AShipPawn>(this->GetPawn()) };
    if (this->IsLocalController())
    {
        FVector mouseWorldLocation{}, mouseWorldDirection{};
        if (this->DeprojectMousePositionToWorld(mouseWorldLocation, mouseWorldDirection))
        {
            FVector hitLoc { FVector::ZeroVector };
            FHitResult hit;
            if (this->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery_MAX, false, hit))
            {
                hitLoc = hit.Location;
            }

            this->RPCServerUpdateMouseLocation(mouseWorldLocation, mouseWorldDirection, hitLoc);
        }
    }
    else if (this->GetNetMode() == ENetMode::NM_DedicatedServer && shipPawn != nullptr)
    {
        shipPawn->RU_PercentSpeed = m_data.m_forward.addValue(m_data.m_lastForwardInput, _deltaTime);
        shipPawn->PercentHorizontalStraf = m_data.m_horizontalStraf.addValue(m_data.m_lastHorizontalStrafInput, _deltaTime);
        shipPawn->PercentVerticalStraf = m_data.m_verticalStraf.addValue(m_data.m_lastVerticalStrafInput, _deltaTime);
        shipPawn->PercentFlightAttitude = m_data.m_flightAttitude.addValue(m_data.m_lastFlightAttitudeInput, _deltaTime);
    }
}

void AGamePlayerController::RPCServerUpdateMouseLocation_Implementation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (this->R_EnableInput && shipPawn != nullptr)
    {
        shipPawn->lookAt(_loc, _dir, _hitLoc);
    }
}

void AGamePlayerController::RPCServerForward_Implementation(float _val)
{
    if (this->R_EnableInput)
    {
        m_data.m_lastForwardInput = _val;
    }
}

void AGamePlayerController::RPCServerHorizontalStraf_Implementation(float _val)
{
    if (this->R_EnableInput)
    {
        m_data.m_lastHorizontalStrafInput = _val;
    }
}

void AGamePlayerController::RPCServerVerticalStraf_Implementation(float _val)
{
    if (this->R_EnableInput)
    {
        m_data.m_lastVerticalStrafInput = _val;
    }
}

void AGamePlayerController::RPCServerFlightAttitude_Implementation(float _val)
{
    if (this->R_EnableInput)
    {
        m_data.m_lastFlightAttitudeInput = _val;
    }
}

void AGamePlayerController::RPCServerTriggerEscapeMode_Implementation()
{
    if (this->R_EnableInput)
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
        {
            shipPawn->TriggerEscapeMode();
        }
    }
}

void AGamePlayerController::RPCServerFire_Implementation(bool _is)
{
    if (this->R_EnableInput)
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
        {
            shipPawn->setFire(_is);
        }
    }
}

void AGamePlayerController::RPCServerRepairProtection_Implementation()
{
    if (this->R_EnableInput)
    {
        AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
        if (shipPawn == nullptr)
        {
            return;
        }

        shipPawn->OnRepairProtectionDelegate.Broadcast();
    }
}

void AGamePlayerController::RPCServerRepairSupport_Implementation()
{
    if (this->R_EnableInput)
    {
        AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
        if (shipPawn == nullptr)
        {
            return;
        }

        shipPawn->OnRepairSupportDelegate.Broadcast();
    }
}

void AGamePlayerController::forward(float _value)
{
    if (!this->R_EnableInput) return;
    this->RPCServerForward(_value);
}

void AGamePlayerController::horizontalStraf(float _value)
{
    if (!this->R_EnableInput) return;
    this->RPCServerHorizontalStraf(_value);
}

void AGamePlayerController::verticalStraf(float _value)
{
    if (!this->R_EnableInput) return;
    this->RPCServerVerticalStraf(_value);
}

void AGamePlayerController::flightAttitude(float _value)
{
    if (!this->R_EnableInput) return;
    this->RPCServerFlightAttitude(_value);
}

void AGamePlayerController::triggerEscapeMode()
{
    if (!this->R_EnableInput) return;
    this->RPCServerTriggerEscapeMode();
}

void AGamePlayerController::fireOn()
{
    if (!this->R_EnableInput) return;
    this->RPCServerFire(true);
}

void AGamePlayerController::fireOff()
{
    if (!this->R_EnableInput) return;
    this->RPCServerFire(false);
}

void AGamePlayerController::returnToMainMenu()
{
    FString levelName{ "MainMenu" };
    UGameplayStatics::OpenLevel(this->GetWorld(), FName(*levelName), false, "");
}

void AGamePlayerController::repairProtection()
{
    if (!this->R_EnableInput) return;
    this->RPCServerRepairProtection();
}

void AGamePlayerController::repairSupport()
{
    if (!this->R_EnableInput) return;
    this->RPCServerRepairSupport();
}

void AGamePlayerController::giveAlly1()
{

}

void AGamePlayerController::giveAlly2()
{

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

void AGamePlayerController::Restart()
{
    AFlyingGameMode* flyingGameMode = Cast<AFlyingGameMode>(UGameplayStatics::GetGameMode(this->GetWorld()));
    if (flyingGameMode == nullptr)
    {
        return;
    }

    ASpacelPlayerState* spacelPlayerState = this->GetPlayerState<ASpacelPlayerState>();
    if (spacelPlayerState)
    {
        flyingGameMode->Restart(this, spacelPlayerState->PlayerStartTransform);
    }
}

void AGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGamePlayerController, R_EnableInput);
}
