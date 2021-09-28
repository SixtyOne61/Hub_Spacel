// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/EffectDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/FlyingGameModeDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/KeyDataAsset.h"
#include "DataAsset/EditorHackDataAsset.h"
#include "GameState/SpacelGameState.h"
#include "Gameplay/Skill/SkillCountDown.h"
#include "Player/SpacelPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/InGameInterface/SkillCarrouselWidget.h"
#include "Widget/SkillWidget.h"
#include "Widget/SkillProgressWidget.h"
#include "Components/ProgressBar.h"
#include "Util/SimplyUI.h"

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
    tickScore();
}

void UInGameWidget::OnChangeState(EGameState _state)
{
    if (_state == EGameState::Prepare)
    {
        m_currentSkillType = ESkillType::Low;
        WaitPlayerState();
    }
    else if (_state == EGameState::LockPrepare)
    {
        BP_GoAnimation();
    }
    else if (_state == EGameState::InGame)
    {
        // update game timer
        if (this->FlyingModeDataAsset != nullptr)
        {
            m_currentTimer = this->FlyingModeDataAsset->RemainingGameTime;

#if WITH_EDITOR
            if (this->HackDataAsset != nullptr)
            {
                if (this->HackDataAsset->UseHack)
                {
                    m_currentTimer = this->HackDataAsset->RemainingGameTime;
                }
            }
#endif
        }

        // prepare game interface
        setupEnnemyTeam();
        setupInGame();
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

        if (this->TeamColorDataAsset != nullptr)
        {
            FColorsType const& teamInfo = this->TeamColorDataAsset->GetColorType(owningPlayerState->R_Team);
            // setup team
            BP_SetupTeam(teamInfo.Logo, teamInfo.SlateColor, owningPlayerState->R_Team);
        }

        // register skill
        owningPlayerState->OnAddSkillUniqueDelegate = std::bind(&UInGameWidget::addSkill, this, std::placeholders::_1);
        owningPlayerState->OnRemoveSkillUniqueDelegate = std::bind(&UInGameWidget::removeSkill, this, std::placeholders::_1);
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

void UInGameWidget::setupInGame()
{
    BP_SetupInGame();
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
#if WITH_EDITOR
            if (this->HackDataAsset != nullptr)
            {
                if (this->HackDataAsset->UseHack)
                {
                    m_currentTimer = this->HackDataAsset->ChooseModuleTime;
                }
            }
#endif
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

void UInGameWidget::setupEnnemyTeam()
{
    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        if(this->TeamColorDataAsset == nullptr) return;

        FString const& localTeam = owningPlayerState->R_Team;

        UWorld* world { this->GetWorld() };
        if (!ensure(world != nullptr)) return;
        if (world->GetGameState() == nullptr) return;

        TSet<FString> uniqueTeams { };

        TArray<APlayerState*> const& playerStates { world->GetGameState()->PlayerArray };
        for (auto playerState : playerStates)
        {
            if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
            {
                uniqueTeams.Add(spacelPlayerState->R_Team);
            }
        }

        uniqueTeams.Remove(localTeam);

        for (auto team : uniqueTeams)
        {
            FColorsType const& teamInfo = this->TeamColorDataAsset->GetColorType(team);
            BP_SetupTeam(teamInfo.Logo, teamInfo.SlateColor, team);
        }
    }
}

void UInGameWidget::tickScore()
{
    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
    {
        FString const& bestTeam = spacelGameState->GetBestTeam();
        TArray<FScore> scores = spacelGameState->R_Scores;
        for (auto score : scores)
        {
            BP_UpdateScore(score.Team, score.Score, bestTeam == score.Team);
        }
    }
}

void UInGameWidget::addSkill(class SkillCountDown* _skill)
{
    if (_skill == nullptr) return;

    if (UUniqueSkillDataAsset const* skill = _skill->getParam())
    {
        if (USkillWidget* skillWidget = SimplyUI::initUnSafeFromName<UUserWidget, USkillWidget>(this, skill->WidgetName))
        {
            if (this->KeyDataAsset != nullptr)
            {
                skillWidget->BP_Setup(skill->BackgroundColorBtn, skill->IconeBtn, this->KeyDataAsset->get(skill->Key));
            }

            if (UProgressBar* progress = SimplyUI::initUnSafeFromName<UUserWidget, UProgressBar>(skillWidget, TEXT("ProgressBar_Skill")))
            {
                _skill->addProgressBar(progress);
            }
        }
    }
}

void UInGameWidget::removeSkill(ESkill _type)
{
    if (this->SkillDataAsset != nullptr)
    {
        if (UUniqueSkillDataAsset* skillParam = this->SkillDataAsset->getSKill(_type))
        {
            if (USkillWidget* skillWidget = SimplyUI::initUnSafeFromName<UUserWidget, USkillWidget>(this, skillParam->WidgetName))
            {
                skillWidget->BP_Remove();
            }
        }
    }
}