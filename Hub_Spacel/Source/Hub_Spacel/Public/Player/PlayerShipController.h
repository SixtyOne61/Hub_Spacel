// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <functional>
#include "PlayerShipController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API APlayerShipController : public APlayerController
{
    GENERATED_BODY()

    class FUnlinearReachGoal
    {
    public:
        FUnlinearReachGoal(FUnlinearReachGoal const&) = delete;
        FUnlinearReachGoal& operator=(FUnlinearReachGoal const&) = delete;

        FUnlinearReachGoal(FUnlinearReachGoal&&) = default;
        FUnlinearReachGoal& operator=(FUnlinearReachGoal &&) = default;

        FUnlinearReachGoal(APlayerShipController* _owner, float _reachTimeUp, float _reachTimeDown)
            : m_owner(_owner)
            , m_reachTimeUp(_reachTimeUp)
            , m_reachTimeDown(_reachTimeDown)
        {
            if (!ensure(m_owner != nullptr)) {}
            if (!ensure(m_reachTimeUp != 0.0f)) {}
            if (!ensure(m_reachTimeDown != 0.0f)) {}
        }

        float addValue(float _value, float _currentPercent);

    private:
        APlayerShipController* m_owner { nullptr };
        const float m_reachTimeUp {};
        const float m_reachTimeDown {};

        TArray<TPair<float, float>> m_values;
        float m_lastValue {};
        float m_duration {};
        float m_startTime {};
        float m_start {};
    };

public:
    virtual void SetupInputComponent() override;

    virtual void BeginPlay() override;
	
protected:
    /* call when input speed change */
    virtual void speed(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetSpeed(float _val);

    /* call when input flight attitude change */
    virtual void flightAttitude(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetFlightAttitude(float _val);

    /* call when input turn change */
    virtual void turn(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetTurn(float _val);

    /* call when input up change */
    virtual void up(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetUp(float _val);

    /* call when fire input is used */
    void fireOn();
    void fireOff();

    UFUNCTION(Reliable, Server)
    void RPCServerFire(bool _on);

    void returnToMainMenu();

    UFUNCTION()
    void StartGame();

private:
    void readInput(int const& _val, float& _in, std::function<void(float)> _fnc);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PercentFlightAttitude = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PercentTurn = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PercentUp = 0.0f;

private:
    TOptional<FUnlinearReachGoal> m_speed {};

    bool m_enableFlyingInput { false };
};
