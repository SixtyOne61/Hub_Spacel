// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShipController.h"
#include "ShipPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Util/SimplyMath.h"
#include "DataAsset/PlayerDataAsset.h"
#include "GameState/SpacelGameState.h"

float APlayerShipController::FUnlinearReachGoal::addValue(float _value, float _currentPercent)
{
    auto lb_reset = [&]()
    {
        // reset
        m_values.Empty();
        m_lastValue = 0.0f;
        m_duration = 0.0f;
        m_start = SimplyMath::Lerp(0.0f, 1.0f, _currentPercent);
    };

    auto lb_add = [&]()
    {
        if (m_values.Num() == 0)
        {
            if (!ensure(m_owner != nullptr)) return;
            UWorld const* world { m_owner->GetWorld() };
            m_startTime = UGameplayStatics::GetTimeSeconds(world);
        }
        m_values.Add(TPair<float, float>(m_duration, _value));
        m_lastValue = _value;
    };

    auto lb_addDuration = [&]()
    {
        if(!ensure(m_owner != nullptr)) return;
        UWorld const* world { m_owner->GetWorld() };
        m_duration = (UGameplayStatics::GetTimeSeconds(world) - m_startTime);
    };

    auto lb_moy = [&]()
    {
        m_values.RemoveAll([&](auto const& _pair){
            return m_duration - _pair.Key > 2.0f; // keep value in last 2s
        });

        if (m_values.Num() == 0)
        {
            return 0.0f;
        }

        float moy { };
        for (auto const& value : m_values)
        {
            moy += value.Value;
        }

        return moy / m_values.Num();
    };

    if (FMath::IsNearlyZero(_value))
    {
        lb_reset();
        return _currentPercent;
    }

    if (_value > 0.0f)
    {
        if (m_lastValue < 0.0f)
        {
            lb_reset();
        }

        lb_add();
        lb_addDuration();
        float moy = lb_moy();

        return SimplyMath::InvLerp(0.0f, 1.0f, m_start + (m_duration * moy) / m_reachTimeUp);
    }
    else
    {
        if (m_lastValue > 0.0f)
        {
            lb_reset();
        }

        lb_add();
        lb_addDuration();
        float moy = lb_moy(); // will be negativ

        return SimplyMath::InvLerp(0.0f, 1.0f, m_start + (m_duration * moy) / m_reachTimeDown);
    }
}

void APlayerShipController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // joystick left
    this->InputComponent->BindAxis("Speed", this, &APlayerShipController::speed);
    this->InputComponent->BindAxis("FlightAttitude", this, &APlayerShipController::flightAttitude);
    // joystick right
    this->InputComponent->BindAxis("Turn", this, &APlayerShipController::turn);
    this->InputComponent->BindAxis("Up", this, &APlayerShipController::up);

    // shoot
    this->InputComponent->BindAction("Fire", IE_Pressed, this, &APlayerShipController::fireOn);
    this->InputComponent->BindAction("Fire", IE_Released, this, &APlayerShipController::fireOff);

    // extra
    this->InputComponent->BindAction("ReturnToMainMenu", IE_Pressed, this, &APlayerShipController::returnToMainMenu);
}

void APlayerShipController::BeginPlay()
{
    Super::BeginPlay();

    if (this->IsLocalController())
    {
        ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
        if (spacelGameState != nullptr)
        {
            spacelGameState->OnStartGameDelegate.AddDynamic(this, &APlayerShipController::StartGame);
        }
    }
}

void APlayerShipController::StartGame()
{
    this->m_enableFlyingInput = true;
}

void APlayerShipController::speed(float _val)
{
    if (this->m_enableFlyingInput)
    {
        // TO DO : make this better, we can keep _val here, and send only relevant value
        // in server, on tick continue to increase or decrease percent
        this->RPCServerSetSpeed(_val);
    }
}

void APlayerShipController::RPCServerSetSpeed_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr || shipPawn->PlayerDataAsset == nullptr)
    {
        return;
    }

    if (!m_speed.IsSet())
    {
        // init
        m_speed = TOptional<FUnlinearReachGoal>(FUnlinearReachGoal { this, shipPawn->PlayerDataAsset->ReachTimeUpSpeed, shipPawn->PlayerDataAsset->ReachTimeDownSpeed });
    }

    shipPawn->R_PercentSpeed = m_speed.GetValue().addValue(_val, shipPawn->R_PercentSpeed);
}

void APlayerShipController::flightAttitude(float _val)
{
    if (this->m_enableFlyingInput)
    {
        readInput(_val, this->PercentFlightAttitude, std::bind(&APlayerShipController::RPCServerSetFlightAttitude, this, std::placeholders::_1));
    }
}

void APlayerShipController::RPCServerSetFlightAttitude_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->RU_PercentFlightAttitude = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentFlightAttitude();
}

void APlayerShipController::turn(float _val)
{
    if (this->m_enableFlyingInput)
    {
        readInput(_val, this->PercentTurn, std::bind(&APlayerShipController::RPCServerSetTurn, this, std::placeholders::_1));
    }
}

void APlayerShipController::RPCServerSetTurn_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->RU_PercentTurn = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentTurn();
}

void APlayerShipController::up(float _val)
{
    if (this->m_enableFlyingInput)
    {
        readInput(_val, this->PercentUp, std::bind(&APlayerShipController::RPCServerSetUp, this, std::placeholders::_1));
    }
}

void APlayerShipController::RPCServerSetUp_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->RU_PercentUp = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentUp();
}

void APlayerShipController::fireOn()
{
    if (this->m_enableFlyingInput)
    {
        this->RPCServerFire(true);
    }
}

void APlayerShipController::fireOff()
{
    if (this->m_enableFlyingInput)
    {
        this->RPCServerFire(false);
    }
}

void APlayerShipController::RPCServerFire_Implementation(bool _on)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->m_isFire = _on;
}

void APlayerShipController::readInput(int const& _val, float& _in, std::function<void(float)> _fnc)
{
    Util::Optional<float> newPercent { };
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        if (_in != 0.0f)
        {
            newPercent = 0.0f;
        }
    }
    else if (_val > 0.0f)
    {
        if (_in < 0.0f)
        {
            newPercent = 0.0f;
        }
        newPercent = FMath::Clamp(_in + _val, 0.0f, 100.0f);
    }
    else if (_val < 0.0f)
    {
        if (_in > 0.0f)
        {
            newPercent = 0.0f;
        }
        newPercent = FMath::Clamp(_in + _val, -100.0f, 0.0f);
    }

    if (newPercent.hasValue())
    {
        _in = newPercent.value();
        _fnc(_in);
    }
}

void APlayerShipController::returnToMainMenu()
{
    FString levelName { "MainMenu" };
    UGameplayStatics::OpenLevel(this->GetWorld(), FName(*levelName), false, "");
}