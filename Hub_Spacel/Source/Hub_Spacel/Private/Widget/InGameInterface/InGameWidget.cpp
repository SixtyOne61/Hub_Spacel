// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/EffectDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/FlyingGameModeDataAsset.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyUI.h"
#include "Widget/InGameInterface/SkillCarrouselWidget.h"

void UInGameWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UInGameWidget::OnChangeState);
    }

    this->CarrouselWidget = SimplyUI::initSafetyFromName<UUserWidget, USkillCarrouselWidget>(this, TEXT("WBP_SkillCarrousel"));
    if (this->CarrouselWidget != nullptr)
    {
        this->CarrouselWidget->OnChangeCarrouselDelegate.AddDynamic(this, &UInGameWidget::OnChangeCarrousel);
        this->CarrouselWidget->OnHoverCarrouselDelegate.AddDynamic(this, &UInGameWidget::OnHoverCarrousel);
    }

    if (this->FlyingModeDataAsset != nullptr)
    {
        m_currentTimer = this->FlyingModeDataAsset->RemainingChooseModuleTime;
    }
}

void UInGameWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UInGameWidget::NativeTick(const FGeometry& _myGeometry, float _deltaSeconde)
{
    Super::NativeTick(_myGeometry, _deltaSeconde);

    tickTimer(_deltaSeconde);
}

void UInGameWidget::OnChangeState(EGameState _state)
{
    if (_state == EGameState::Prepare)
    {
        m_currentSkillType = ESkillType::Low;
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
        // must be low skill
        BP_SetupSkillCarrousel(m_currentSkillType);
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

void UInGameWidget::OnChangeCarrousel(ESkill _skillId, ESkillType _type)
{
    m_currentSkillType = (ESkillType)((uint8)_type + 1);
    BP_SetupSkillCarrousel(m_currentSkillType);

    // go to next state
    m_internState = (EInternState)((uint8)m_internState + 1);

    if (this->FlyingModeDataAsset != nullptr)
    {
        switch (m_internState)
        {
        case EInternState::ChooseLow:
        case EInternState::ChooseMedium:
        case EInternState::ChooseHight:
            m_currentTimer += this->FlyingModeDataAsset->RemainingChooseModuleTime;
            break;
        case EInternState::Go:
            m_currentTimer += this->FlyingModeDataAsset->EndModuleTime;
            break;
        }
    }
}

void UInGameWidget::OnHoverCarrousel(ESkill _skillId, ESkillType _type)
{
    if (this->SkillDataAsset == nullptr) return;

    if (auto uniqueSkillDataAsset = this->SkillDataAsset->getSKill(_skillId))
    {
        BP_SetupSkill(_type, uniqueSkillDataAsset->IconeBtn, uniqueSkillDataAsset->BackgroundColorBtn);
    }
}

void UInGameWidget::tickTimer(float _deltaSeconde)
{
    m_currentTimer -= _deltaSeconde;

    if (m_currentTimer <= 0.0f)
    {
        m_currentTimer = 0.0f;
        switch (m_internState)
        {
        case EInternState::ChooseLow:
            this->CarrouselWidget->OnChooseSkill(ESkill::DefaultLow, ESkillType::Low);
            break;

        case EInternState::ChooseMedium:
            this->CarrouselWidget->OnChooseSkill(ESkill::DefaultMedium, ESkillType::Medium);
            break;

        case EInternState::ChooseHight:
            this->CarrouselWidget->OnChooseSkill(ESkill::DefaultHight, ESkillType::Hight);
            break;

        case EInternState::Go:
            break;

        case EInternState::InGame:
            break;

        default:
            break;
        }
    }

    int min = (int)m_currentTimer / 60;
    int sec = (int)m_currentTimer % 60;
    FString minStr = min < 10 ? "0" + FString::FromInt(min) : FString::FromInt(min);
    FString secStr = sec < 10 ? "0" + FString::FromInt(sec) : FString::FromInt(sec);

    BP_UpdateTimer(minStr + ":" + secStr);
}