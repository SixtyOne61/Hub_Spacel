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
#include "Components/WidgetInteractionComponent.h"

void UTargetUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TargetButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Button_Target"));
    TargetImage = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("Image_Target"));

    if(hideAlly()) return;

    setTargetImage(BaseColor);

    if (UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(GetGameInstance()))
    {
        spacelGameInstance->OnTargetPlayerDelegate.AddDynamic(this, &UTargetUserWidget::OnTargetPlayer);
        spacelGameInstance->OnTryLockDelegate.AddDynamic(this, &UTargetUserWidget::OnTryLock);
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

    adjust();

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

void UTargetUserWidget::adjust()
{
    if(m_isSameTeam) return;

    UWorld* const world { this->GetWorld() };
    AShipPawn* pawn { Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(world, 0)) };

    if(this->Owner == nullptr || pawn == nullptr || pawn->WidgetTargetComponent == nullptr
        || this->TargetButton == nullptr || this->TargetImage == nullptr) return;

    float dist { FVector::Dist(pawn->GetActorLocation(), this->Owner->GetActorLocation()) };
    float interactionDist = pawn->WidgetTargetComponent->InteractionDistance;

    if (dist > interactionDist
        && this->TargetButton->GetVisibility() == ESlateVisibility::Visible)
    {
        SimplyUI::setVisibility({ ESlateVisibility::Hidden },
            std::make_tuple(this->TargetButton, this->TargetImage));
    }
    else if (dist <= interactionDist
        && this->TargetButton->GetVisibility() == ESlateVisibility::Hidden)
    {
        SimplyUI::setVisibility({ ESlateVisibility::Visible },
            std::make_tuple(this->TargetButton, this->TargetImage));
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

    if (spacelLocalPlayerState->R_Team == spacelOwnerPlayerState->R_Team)
    {
        // hide widget
        this->SetVisibility(ESlateVisibility::Hidden);
        m_isSameTeam = true;
        return true;
    }

    return false;
}

void UTargetUserWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UTargetUserWidget::OnTargetPlayer(int32 _playerId, bool _lock)
{
    if (_lock && m_state == EState::StateLock)
    {
        if (this->Owner == nullptr) return;
        if (AShipPawn* ownerPawn = Cast<AShipPawn>(this->Owner->GetParentActor()))
        {
            if (ASpacelPlayerState* playerState = ownerPawn->GetPlayerState<ASpacelPlayerState>())
            {
                if (playerState->PlayerId != _playerId)
                {
                    m_state = EState::StateNormal;
                }
            }
        }
    }
}

void UTargetUserWidget::OnTryLock()
{
    if (m_state == EState::StateHover
        || m_state == EState::StateLock)
    {
        OnPressed();
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
    if(m_isSameTeam) return;
    if (this->Owner == nullptr) return;

    if (AShipPawn* localPawn = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this->GetWorld(), 0)))
    {
        if (AShipPawn* ownerPawn = Cast<AShipPawn>(this->Owner->GetParentActor()))
        {
            if (!ownerPawn->hasEffect(EEffect::Fog))
            {
                if (ASpacelPlayerState* playerState = ownerPawn->GetPlayerState<ASpacelPlayerState>())
                {
                    if (m_state == EState::StateLock)
                    {
                        m_state = EState::StateNormal;
                        localPawn->lockTarget(playerState->PlayerId, false);
                    }
                    else
                    {
                        m_state = EState::StateLock;
                        localPawn->lockTarget(playerState->PlayerId, true);
                    }
                }
            }
        }
    }
}

void UTargetUserWidget::OnHovered()
{
    m_isHovered = true;
}

void UTargetUserWidget::showTarget(bool _show)
{
    if (m_isSameTeam) return;

    if (_show)
    {
        this->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        this->SetVisibility(ESlateVisibility::Hidden);
        if (m_state != EState::StateNormal)
        {
            m_state = EState::StateNormal;
            m_isHovered = false;
        }
    }
}