// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/EffectDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
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

    if (USkillCarrouselWidget* carrousel = SimplyUI::initSafetyFromName<UUserWidget, USkillCarrouselWidget>(this, TEXT("WBP_SkillCarrousel")))
    {
        carrousel->OnChangeCarrouselDelegate.AddDynamic(this, &UInGameWidget::OnChangeCarrousel);
        carrousel->OnHoverCarrouselDelegate.AddDynamic(this, &UInGameWidget::OnHoverCarrousel);
    }
}

void UInGameWidget::NativeDestruct()
{
    Super::NativeDestruct();
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
}

void UInGameWidget::OnHoverCarrousel(ESkill _skillId, ESkillType _type)
{
    if (this->SkillDataAsset == nullptr) return;

    if (auto uniqueSkillDataAsset = this->SkillDataAsset->getSKill(_skillId))
    {
        BP_SetupSkill(_type, uniqueSkillDataAsset->IconeBtn, uniqueSkillDataAsset->BackgroundColorBtn);
    }
}
