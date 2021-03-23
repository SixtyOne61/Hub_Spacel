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

    this->InputComponent->BindAxis("Forward", this, &AGamePlayerController::forward);
    this->InputComponent->BindAxis("HorizontalStraf", this, &AGamePlayerController::horizontalStraf);
    this->InputComponent->BindAxis("VerticalStraf", this, &AGamePlayerController::verticalStraf);
    this->InputComponent->BindAxis("FlightAttitude", this, &AGamePlayerController::flightAttitude);
    this->InputComponent->BindAxis("Skill", this, &AGamePlayerController::skill);

    this->InputComponent->BindAction("Fire", IE_Pressed, this, &AGamePlayerController::fireOn);
    this->InputComponent->BindAction("Fire", IE_Released, this, &AGamePlayerController::fireOff);
    this->InputComponent->BindAction("ReturnToMainMenu", IE_Pressed, this, &AGamePlayerController::returnToMainMenu);
    this->InputComponent->BindAction("Lock", IE_Pressed, this, &AGamePlayerController::lock);
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
                spacelGameState->OnStartGameDelegate.AddDynamic(this, &AGamePlayerController::StartGame);
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

bool AGamePlayerController::GetHitResultUnderCursor(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& HitResult, AActor* _ignoreActor)
{
    ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
    bool bHit = false;
    if (LocalPlayer && LocalPlayer->ViewportClient)
    {
        FVector2D MousePosition;
        if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
        {
            FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(ClickableTrace), bTraceComplex);
            CollisionQueryParams.AddIgnoredActor(_ignoreActor);
            bHit = GetHitResultAtScreenPosition(MousePosition, TraceChannel, CollisionQueryParams, HitResult);
        }
    }

    if (!bHit)	//If there was no hit we reset the results. This is redundant but helps Blueprint users
    {
        HitResult = FHitResult();
    }

    return bHit;
}

void AGamePlayerController::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    if (!isAvailable()) return;

    AShipPawn* shipPawn { Cast<AShipPawn>(this->GetPawn()) };
    if (this->IsLocalController())
    {
        FVector mouseWorldLocation{}, mouseWorldDirection{};
        if (this->DeprojectMousePositionToWorld(mouseWorldLocation, mouseWorldDirection))
        {
            FVector hitLoc { FVector::ZeroVector };

            FHitResult hit;
            if (this->GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, false, hit, shipPawn))
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
    if (isAvailable() && shipPawn != nullptr)
    {
        shipPawn->lookAt(_loc, _dir, _hitLoc);
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

void AGamePlayerController::RPCServerSkill_Implementation(float _slot)
{
    if (isAvailable())
    {
        AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
        if (shipPawn == nullptr)
        {
            return;
        }

        shipPawn->useSkill(_slot);
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

void AGamePlayerController::skill(float _slot)
{
    if (!isAvailable() || _slot == 0.0f)
    {
        return;
    }
    this->RPCServerSkill(_slot - 1.0f);

    // for local feedback (count down etc)
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    // behaviour of each skill is disable on client
    shipPawn->useSkill(_slot - 1.0f);
}

void AGamePlayerController::returnToMainMenu()
{
    FString levelName{ "MainMenu" };
    UGameplayStatics::OpenLevel(this->GetWorld(), FName(*levelName), false, "");
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
        if (shipPawn->hasEffect(EEffect::Emp)) return false;
    }

    return this->R_EnableInput;
}

void AGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGamePlayerController, R_EnableInput);
}
