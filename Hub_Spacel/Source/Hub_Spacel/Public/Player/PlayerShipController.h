// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <functional>
#include "PlayerShipController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleRepair, bool, _on);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API APlayerShipController : public APlayerController
{
    GENERATED_BODY()

    friend class URepairUserWidget;

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

    void Restart();
	
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

    /* repair input */
    void toggleRepair();

    /* fast move input */
    void triggerEscapeMode();
    UFUNCTION(Reliable, Server)
    void RPCServerTriggerEscapeMode();

    void returnToMainMenu();

    UFUNCTION()
    void StartGame();

    /* call from UI */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleRepairProtection(bool _on);
    UFUNCTION(Reliable, Server)
    void RPCServerRepairProtection(bool _on);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleRepairSupport(bool _on);
    UFUNCTION(Reliable, Server)
    void RPCServerRepairSupport(bool _on);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleGiveAlly1(bool _on);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleGiveAlly2(bool _on);

private:
    void readInput(int const& _val, float& _in, std::function<void(float)> _fnc);

    void toggleRpcCall(bool const& _val, bool & _toggle, std::function<void(bool)> _rpc);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PercentFlightAttitude = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PercentTurn = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PercentUp = 0.0f;

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnToggleRepair OnToggleRepairDelegate {};

private:
    TOptional<FUnlinearReachGoal> m_speed {};

    bool m_enableFlyingInput { false };
    bool m_toggleRepair { false };
    bool m_toggleProtection { false };
    bool m_toggleSupport { false };
};
