// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetUserWidget.h"
#include "Util/SimplyUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Hub_SpacelGameInstance.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UTargetUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TargetButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Button_Target"));
    TargetImage = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("Image_Target"));

    if(hideAlly()) return;

    setTargetImage(BaseColor);

    if (UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(GetGameInstance()))
    {
        spacelGameInstance->OnUnTargetPlayerDelegate.AddDynamic(this, &UTargetUserWidget::OnUnTargetPlayer);
        spacelGameInstance->OnTargetPlayerDelegate.AddDynamic(this, &UTargetUserWidget::OnTargetPlayer);
    }

    m_state = EState::StateNormal;
    m_state.init({ std::bind(&UTargetUserWidget::onChangeStateNormal, this),
                    std::bind(&UTargetUserWidget::onChangeStateHover, this),
                    std::bind(&UTargetUserWidget::onChangeStateLock, this) });

    // bind event from button
    TargetButton->OnPressed.AddDynamic(this, &UTargetUserWidget::OnPressed);
    TargetButton->OnHovered.AddDynamic(this, &UTargetUserWidget::OnHovered);
}

void UTargetUserWidget::NativeTick(const FGeometry& _myGeometry, float _deltaTime)
{
    Super::NativeTick(_myGeometry, _deltaTime);

    if (m_isHovered)
    {
        if (m_state == EState::StateNormal)
        {
            m_state = EState::StateHover;
        }
        m_isHovered = false;
    }
    else if (!m_isHovered && m_state == EState::StateHover)
    {
        m_state = EState::StateNormal;
    }
}

bool UTargetUserWidget::hideAlly()
{
    if(this->Owner == nullptr) return false;

    AShipPawn* ownerPawn { Cast<AShipPawn>(this->Owner->GetParentActor()) };
    if (ownerPawn == nullptr) return false;

    UWorld* const world { this->GetWorld() };
    if (!ensure(world != nullptr)) return false;

    /* check team */

    // team of local player
    APawn* pawn { UGameplayStatics::GetPlayerPawn(world, 0) };
    if (pawn == nullptr) return false;

    ASpacelPlayerState const* spacelLocalPlayerState { pawn->GetPlayerState<ASpacelPlayerState>() };
    if (spacelLocalPlayerState == nullptr) return false;

    // team of this owner pawn
    ASpacelPlayerState const* spacelOwnerPlayerState { ownerPawn->GetPlayerState<ASpacelPlayerState>() };
    if (spacelOwnerPlayerState == nullptr) return false;

    if (spacelLocalPlayerState->Team == spacelOwnerPlayerState->Team)
    {
        // hide widget
        this->SetVisibility(ESlateVisibility::Hidden);
        return true;
    }

    return false;
}

void UTargetUserWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UTargetUserWidget::OnUnTargetPlayer(AActor* _target)
{
    if (_target == nullptr || m_state != EState::StateLock) return;

    // find player id of target
    int32 playerTargetId { AShipPawn::getPlayerIdFromTarget(_target) };
    int32 playerId { AShipPawn::getPlayerIdFromTarget(this->Owner) };

    if (playerTargetId == playerId)
    {
        m_state = EState::StateNormal;
    }
}

void UTargetUserWidget::OnTargetPlayer(AActor* _target)
{
    if (_target == nullptr) return;

    // find player id of target
    int32 playerTargetId { AShipPawn::getPlayerIdFromTarget(_target) };
    int32 playerId { AShipPawn::getPlayerIdFromTarget(this->Owner) };

    if (playerTargetId == playerId && m_state != EState::StateLock)
    {
        m_state = EState::StateLock;
    }
    else if (playerTargetId != playerId && m_state == EState::StateLock)
    {
        m_state = EState::StateNormal;
    }
}

void UTargetUserWidget::setTargetImage(FLinearColor const& _color)
{
    if (this->TargetImage != nullptr)
    {
        this->TargetImage->SetColorAndOpacity(_color);
    }
}

void UTargetUserWidget::onChangeStateNormal()
{
    setTargetImage(this->BaseColor);
}

void UTargetUserWidget::onChangeStateHover()
{
    setTargetImage(this->HoverColor);
}

void UTargetUserWidget::onChangeStateLock()
{
    setTargetImage(this->LockColor);
}

void UTargetUserWidget::OnPressed()
{
    AShipPawn* ownerPawn{ this->GetOwningPlayerPawn<AShipPawn>() };
    if (ownerPawn == nullptr) return;

    if (UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance()))
    {
        if (m_state == EState::StateLock)
        {
            spacelGameInstance->OnUnTargetPlayerDelegate.Broadcast(this->Owner);
        }
        else
        {
            spacelGameInstance->OnTargetPlayerDelegate.Broadcast(this->Owner);
        }
    }
}

void UTargetUserWidget::OnHovered()
{
    m_isHovered = true;
}