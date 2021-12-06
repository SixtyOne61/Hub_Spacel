// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/FlyingGameModeDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/KeyDataAsset.h"
#include "DataAsset/EditorHackDataAsset.h"
#include "GameState/SpacelGameState.h"
#include "Gameplay/Skill/SkillCountDown.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Widget/InGameInterface/SkillCarrouselWidget.h"
#include "Widget/SkillWidget.h"
#include "Widget/SkillProgressWidget.h"
#include "Widget/InGameInterface/SkillItemWidget.h"
#include "Components/ProgressBar.h"
#include "Util/SimplyUI.h"
#include "Util/Tag.h"

void UInGameWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UInGameWidget::OnChangeState);

        // mission
        spacelGameState->OnStartMissionDelegate.AddDynamic(this, &UInGameWidget::OnStartMission);
        spacelGameState->OnStartMissionTwoParamDelegate.AddDynamic(this, &UInGameWidget::OnStartMissionTwoParam);
        spacelGameState->OnEndMissionDelegate.AddDynamic(this, &UInGameWidget::OnEndMission);
        spacelGameState->OnResetTimerMissionDelegate.AddDynamic(this, &UInGameWidget::OnResetTimerMission);

        // clean field
        spacelGameState->OnWhoKillWhoDelegate.AddDynamic(this, &UInGameWidget::OnKill);
    }

    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        shipPawn->OnEndUpdateMatiereDelegate.AddDynamic(this, &UInGameWidget::BP_OnUpdateMatiere);
        shipPawn->OnSendInfoPlayerDelegate.AddDynamic(this, &UInGameWidget::BP_OnSendInfoPlayer);

        shipPawn->OnAddEffectClientDelegate.AddDynamic(this, &UInGameWidget::BP_OnAddEffect);
        shipPawn->OnRemoveEffectDelegate.AddDynamic(this, &UInGameWidget::BP_OnRemoveEffect);

        if (UModuleComponent* moduleComponent = shipPawn->ModuleComponent)
        {
            moduleComponent->OnUpdateCountProtectionDelegate.AddDynamic(this, &UInGameWidget::BP_OnUpdateCountProtection);
            moduleComponent->OnUpdateCountSupportDelegate.AddDynamic(this, &UInGameWidget::BP_OnUpdateCountSupport);
        }

        // Feedback Scoring
        shipPawn->OnFeedbackScoreDelegate.AddDynamic(this, &UInGameWidget::OnScored);
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
    updateMissionArrowOrientation();
}

void UInGameWidget::OnChangeState(EGameState _state)
{
    if (_state == EGameState::Prepare)
    {
        setupChooseSkill();
        WaitPlayerState();
    }
    else if (_state == EGameState::ChooseSkill)
    {
        if (this->FlyingModeDataAsset != nullptr)
        {
            m_currentTimer = this->FlyingModeDataAsset->RemainingChooseModuleTime;
        }
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
    else if (_state == EGameState::EndGame)
    {
        BP_EndGame();
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

    // ally widget
    UWorld* world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (world->GetGameState() == nullptr) return;
    TArray<APlayerState*> const& playerStates { world->GetGameState()->PlayerArray };

    ASpacelPlayerState* owningPlayerState { Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr) return;

    FString owningPlayerTeam { owningPlayerState->R_Team };

    for (APlayerState* playerState : playerStates)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            if (spacelPlayerState->GetUniqueID() == owningPlayerState->GetUniqueID()) continue;

            if (spacelPlayerState->R_Team.Equals(owningPlayerTeam))
            {
                BP_CreateAllyWidget(spacelPlayerState);
            }
        }
    }
}

void UInGameWidget::setupChooseSkill()
{
    if (this->SkillDataAsset == nullptr) return;

    auto lb = [&](ESkillType _type)
    {
        auto const& skills = this->SkillDataAsset->getSkillByType(_type);
        for (UUniqueSkillDataAsset* skill : skills)
        {
            if (skill != nullptr)
            {
                USkillItemWidget* _out { nullptr };
                BP_AddSkillToChoose(skill->OnClickSound, skill->Skill, _type, skill->IconeBtn, skill->BackgroundColorBtn, skill->Title, skill->Desc, skill->VerboseEffect, _out);
                if (_out != nullptr)
                {
                    _out->OnHoverSkillDelegate.AddDynamic(this, &UInGameWidget::BP_OnHoverSkill);
                    _out->OnChooseSkillDelegate.AddDynamic(this, &UInGameWidget::BP_OnChooseSkill);
                    _out->OnUnHoverSkillDelegate.AddDynamic(this, &UInGameWidget::BP_OnUnHoverSkill);
                }
            }
        }
    };

    lb(ESkillType::Low);
    lb(ESkillType::Medium);
    lb(ESkillType::Hight);
}

/*void UInGameWidget::OnChangeCarrousel(ESkill _skillId, ESkillType _type)
{
    m_currentSkillType = (ESkillType)((uint8)_type + 1);
    setupDefaultSkill();
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

    if (auto* uniqueSkillDataAsset = this->SkillDataAsset->getSKill(_skillId))
    {
        BP_SetupSkill(_type, uniqueSkillDataAsset->IconeBtn, uniqueSkillDataAsset->BackgroundColorBtn, uniqueSkillDataAsset->VerboseEffect);
    }
}*/

void UInGameWidget::tickTimer(float _deltaSeconde)
{
    m_currentTimer -= _deltaSeconde;

    if (m_currentTimer <= 0.0f)
    {
        m_currentTimer = 0.0f;
    }

    int min = (int)m_currentTimer / 60;
    int sec = (int)m_currentTimer % 60;
    FString minStr = min < 10 ? "0" + FString::FromInt(min) : FString::FromInt(min);
    FString secStr = sec < 10 ? "0" + FString::FromInt(sec) : FString::FromInt(sec);

    BP_UpdateTimer(minStr + ":" + secStr, min == 0 && sec < 30);
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
        for (auto* playerState : playerStates)
        {
            if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
            {
                uniqueTeams.Add(spacelPlayerState->R_Team);
            }
        }

        uniqueTeams.Remove(localTeam);

        for (auto const& team : uniqueTeams)
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
        for (auto const& score : scores)
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

            if (UProgressBar* progress = SimplyUI::initUnSafeFromName<UUserWidget, UProgressBar>(skillWidget, TEXT("PB_Skill")))
            {
                _skill->addProgressBar(progress);
            }

            _skill->setSkillWidget(skillWidget);

            if (skill->MatiereNeeded != 0)
            {
                skillWidget->BP_SetupMatere(skill->MatiereNeeded);
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

void UInGameWidget::OnStartMission(EMission _type)
{
    if (this->MissionDataAsset != nullptr)
    {
        startMission(this->MissionDataAsset->getMissionModify(_type));
    }
}

void UInGameWidget::OnStartMissionTwoParam(EMission _type, FName const& _team, FName const& _targetTeam)
{
    if (this->MissionDataAsset != nullptr)
    {
        if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
        {
            FString const& localTeam = owningPlayerState->R_Team;
            if (*localTeam == _team)
            {
                if (FMission* mission = this->MissionDataAsset->getMissionModify(_type))
                {
                    mission->Team = _targetTeam.ToString();
                    startMission(mission);
                }
            }
        }
    }
}

void UInGameWidget::startMission(FMission* _mission)
{
    if(_mission == nullptr) return;

    FString desc = _mission->MissionDesc;
    desc = desc.Replace(*FString("%reward%"), *FString::FromInt(_mission->RewardValue));
    desc = desc.Replace(*FString("%condition%"), *FString::FromInt(_mission->ConditionValue));
    desc = desc.Replace(*FString("%time%"), *(FString::FromInt(_mission->DurationValue) + "s"));
    if (this->TeamColorDataAsset != nullptr && !_mission->Team.IsEmpty())
    {
        FColorsType const& info = this->TeamColorDataAsset->GetColorType(_mission->Team);
        desc = desc.Replace(*FString("%team%"), *info.ShortName);
    }

    _mission->MissionDesc = desc;
    BP_StartMission(*_mission);

    if (_mission->Type == EMission::Pirate)
    {
        // find pirate
        InitMissionArrow(Tags::Pirate);
    }
    else if (_mission->Type == EMission::Comet)
    {
        // find comet
        InitMissionArrow(Tags::Comet);
    }
    else if (_mission->Type == EMission::TakeGold)
    {
        // find gold
        InitMissionArrow(Tags::Gold);
    }
}

void UInGameWidget::OnEndMission(EMission _type, bool _succeed, FName _succeedForTeam)
{
    if (this->MissionDataAsset != nullptr)
    {
        if (FMission* mission = this->MissionDataAsset->getMissionModify(_type))
        {
            mission->IsSucceed = _succeed;

            if (!_succeedForTeam.IsNone())
            {
                if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
                {
                    // override for passive
                    mission->IsSucceed = shipPawn->Team == _succeedForTeam;
                }
            }

            BP_OnEndMission(*mission);
        }
    }

    if (_type == EMission::Pirate || _type == EMission::Comet || _type == EMission::TakeGold)
    {
        m_arrowTarget = nullptr;
    }
}

void UInGameWidget::OnResetTimerMission(EMission _type)
{
    BP_OnResetTimerMission(*this->MissionDataAsset->getMissionModify(_type));
}

void UInGameWidget::InitMissionArrow(FName const& _tag)
{
    TArray<AActor*> out;
    UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), _tag, out);

    for (auto* act : out)
    {
        if (act != nullptr && !act->IsPendingKill())
        {
            m_arrowTarget = act;
            return;
        }
    }

    FTimerDelegate timerDelegate;
    timerDelegate.BindUFunction(this, FName("InitMissionArrow"), _tag);

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    FTimerHandle handle;
    world->GetTimerManager().SetTimer(handle, timerDelegate, 1.0f, false, 0.0f);
}

void UInGameWidget::updateMissionArrowOrientation()
{
    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        TArray<UActorComponent*> components = shipPawn->GetComponentsByTag(USceneComponent::StaticClass(), Tags::Arrow);
        for (auto* component : components)
        {
            if (USceneComponent* sceneComponent = Cast<USceneComponent>(component))
            {
                if (m_arrowTarget != nullptr && !m_arrowTarget->IsPendingKill())
                {
                    FRotator rot = UKismetMathLibrary::FindLookAtRotation(sceneComponent->GetComponentLocation(), m_arrowTarget->GetActorLocation());
                    sceneComponent->SetWorldRotation(rot.Quaternion());
                    sceneComponent->SetVisibility(true);
                }
                else
                {
                    sceneComponent->SetVisibility(false);
                }
                break;
            }
        }
    }
}

void UInGameWidget::OnScored(EScoreType _type, int32 _value)
{
    FString value = "+";
    value += FString::FromInt(_value);

    FVector2D mousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this->GetWorld());
    BP_OnScored(_type, value, mousePosition);
}

void UInGameWidget::OnKill(int32 _killer, int32 _killed)
{
    if (this->TeamColorDataAsset == nullptr) return;

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    TArray<APlayerState*> const& playerStates{ world->GetGameState()->PlayerArray };

    FString killerName, killedName{};
    FSlateColor killerColor, killedColor{};

    for (auto* playerState : playerStates)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            int32 playerId = spacelPlayerState->PlayerId;
            FString const& name = spacelPlayerState->GetPlayerName();
            if (playerId == _killer)
            {
                killerName = name;
                killerColor = this->TeamColorDataAsset->GetColor<FSlateColor>(spacelPlayerState->R_Team);
            }
            else if (playerId == _killed)
            {
                killedName = name;
                killedColor = this->TeamColorDataAsset->GetColor<FSlateColor>(spacelPlayerState->R_Team);
            }
        }
    }

    BP_CreateKillField(killerName, killerColor, killedName, killedColor);
}