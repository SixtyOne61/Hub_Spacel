// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelWidget.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Player/SpacelPlayerState.h"
#include "Player/GamePlayerController.h"
#include "Player/ModuleComponent.h"
#include "Player/Save/SpacelSaveGame.h"
#include "Player/ShipPawn.h"
#include "Gameplay/SkillComponent.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Util/SimplyUI.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/EffectDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/DitactitialDataAsset.h"
#include "DataAsset/FlyingGameModeDataAsset.h"
#include "DataAsset/KeyDataAsset.h"
#include "DataAsset/MissionDataAsset.h"
#include "Widget/AllyWidget.h"
#include "Widget/SkillWidget.h"
#include "Widget/SkillProgressWidget.h"
#include "Widget/EffectWidget.h"
#include "Widget/ScoreUserWidget.h"
#include "Widget/TutorialUserWidget.h"
#include "Widget/MissionPanelUserWidget.h"
#include "Factory/SpacelFactory.h"
#include "Styling/SlateColor.h"
#include "GameMode/FlyingGameMode.h"
#include "Util/Tag.h"
#include "Util/DebugScreenMessage.h"

void USpacelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    EventTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Event"));
    TimerTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Timer"));
    PingTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Ping"));
    ProtectionProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Protection"));
    SupportProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Support"));
    ProtectionTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Protection"));
    SupportTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Support"));
    SkillBarHorizontalBox = SimplyUI::initSafetyFromName<UUserWidget, UHorizontalBox>(this, TEXT("SkillBar"));
    EffectBarHorizontalBox = SimplyUI::initSafetyFromName<UUserWidget, UHorizontalBox>(this, TEXT("EffectBar"));

    TArray<FName> allyNames { TEXT("Widget_Ally1"), TEXT("Widget_Ally2") };
    SimplyUI::initArray(this, AllyWidgets, allyNames);

    TArray<FName> scoreColorNames { TEXT("Border_ColorTeam1"), TEXT("Border_ColorTeam2"), TEXT("Border_ColorTeam3") };
    SimplyUI::initArray(this, TeamScoreColorWidgets, scoreColorNames);

    TArray<FName> scoreNames{ TEXT("TextBlock_ScoreTeam1"), TEXT("TextBlock_ScoreTeam2"), TEXT("TextBlock_ScoreTeam3") };
    SimplyUI::initArray(this, TeamScoreWidgets, scoreNames);

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetLatestEventHandle, this, &USpacelWidget::SetLatestEvent, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetAverragePlayerLatencyHandle, this, &USpacelWidget::SetAverragePlayerLatency, 1.0f, true, 1.0f);

    this->SetVisibility(ESlateVisibility::Hidden);
    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &USpacelWidget::OnChangeState);
        // mission
        spacelGameState->OnStartMissionDelegate.AddDynamic(this, &USpacelWidget::OnStartMission);
        spacelGameState->OnStartMissionTwoParamDelegate.AddDynamic(this, &USpacelWidget::OnStartMissionTwoParam);
        spacelGameState->OnEndMissionDelegate.AddDynamic(this, &USpacelWidget::OnEndMission);
        spacelGameState->OnResetTimerMissionDelegate.AddDynamic(this, &USpacelWidget::OnResetTimerMission);
    }

    AShipPawn* shipPawn { this->GetOwningPlayerPawn<AShipPawn>() };
    if (shipPawn != nullptr)
    {
        shipPawn->OnEndUpdateMatiereDelegate.AddDynamic(this, &USpacelWidget::OnUpdateMatiere);

        shipPawn->ModuleComponent->OnUpdateCountProtectionDelegate.AddDynamic(this, &USpacelWidget::OnUpdateCountProtection);
        shipPawn->ModuleComponent->OnUpdateCountSupportDelegate.AddDynamic(this, &USpacelWidget::OnUpdateCountSupport);

        shipPawn->OnShowMissionDelegate.AddDynamic(this, &USpacelWidget::OnShowMission);
        shipPawn->OnAddEffectDelegate.AddDynamic(this, &USpacelWidget::OnAddEffect);
        shipPawn->OnRemoveEffectDelegate.AddDynamic(this, &USpacelWidget::OnRemoveEffect);

        shipPawn->OnSendInfoPlayerDelegate.AddDynamic(this, &USpacelWidget::OnSendInfoPlayer);

        RegisterPlayerState();
    }

    if (UMissionPanelUserWidget* panelMission = SimplyUI::initSafetyFromName<UUserWidget, UMissionPanelUserWidget>(this, TEXT("WBP_Mission")))
    {
        if (this->KeyDataAsset != nullptr)
        {
            panelMission->BP_Setup(this->KeyDataAsset->get(this->KeyMissionPanel));
        }
    }
}

void USpacelWidget::RegisterPlayerState()
{
    if(AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        if (ASpacelPlayerState* playerState = shipPawn->GetPlayerState<ASpacelPlayerState>())
        {
            playerState->OnAddSkillUniqueDelegate = std::bind(&USpacelWidget::addSkill, this, std::placeholders::_1);
            return;
        }
    }

    FTimerHandle handle;
    this->GetWorld()->GetTimerManager().SetTimer(handle, this, &USpacelWidget::RegisterPlayerState, 1.0f, false);
}

void USpacelWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (world)
    {
        world->GetTimerManager().ClearTimer(SetLatestEventHandle);
        world->GetTimerManager().ClearTimer(SetAverragePlayerLatencyHandle);
        world->GetTimerManager().ClearTimer(SetSpeedHandle);
        world->GetTimerManager().ClearTimer(ShowDitactitialHandle);
    }
    Super::NativeDestruct();
}

void USpacelWidget::NativeTick(const FGeometry& _myGeometry, float _deltaTime)
{
    Super::NativeTick(_myGeometry, _deltaTime);
    UpdateScore();
    updateArrow();
}

void USpacelWidget::updateArrow()
{
    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        TArray<UActorComponent*> components = shipPawn->GetComponentsByTag(USceneComponent::StaticClass(), Tags::Arrow);
        for (auto component : components)
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

void USpacelWidget::OnStartMissionTwoParam(EMission _type, FName const& _team)
{
    if (*this->Team == _team)
    {
        OnStartMission(_type);
    }
}

void USpacelWidget::OnStartMission(EMission _type)
{
    if(this->MissionDataAsset == nullptr) return;

    if (m_currentMission.Num() == 0)
    {
        // appear mission panel
        BP_ShowMissionPanel();
    }

    m_currentMission.Add(_type);

    if (UMissionPanelUserWidget* panelMission = SimplyUI::initSafetyFromName<UUserWidget, UMissionPanelUserWidget>(this, TEXT("WBP_Mission")))
    {
        panelMission->addMission(this->MissionDataAsset->getMission(_type));
    }

    if (_type == EMission::Pirate)
    {
        // find pirate
        InitTargetArrow(Tags::Pirate);
    }
    else if (_type == EMission::Comet)
    {
        // find comet
        InitTargetArrow(Tags::Comet);
    }
    else if (_type == EMission::TakeGold)
    {
        // find gold
        InitTargetArrow(Tags::Gold);
    }
}

void USpacelWidget::InitTargetArrow(FName const& _tag)
{
    TArray<AActor*> out;
    UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), _tag, out);

    for (auto act : out)
    {
        if (act != nullptr && !act->IsPendingKill())
        {
            m_arrowTarget = act;
            return;
        }
    }

    FTimerDelegate timerDelegate;
    timerDelegate.BindUFunction(this, FName("InitTargetArrow"), _tag);

    UWorld* world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    FTimerHandle handle;
    world->GetTimerManager().SetTimer(handle, timerDelegate, 1.0f, false, 0.0f);
}

void USpacelWidget::OnEndMission(EMission _type)
{
    if (m_currentMission.Contains(_type))
    {
        m_currentMission.Remove(_type);

        if (m_currentMission.Num() == 0)
        {
            // hide mission panel
            HideMissionPanel();
        }

        if (UMissionPanelUserWidget* panelMission = SimplyUI::initSafetyFromName<UUserWidget, UMissionPanelUserWidget>(this, TEXT("WBP_Mission")))
        {
            panelMission->removeMission(_type);
        }
    }

    if (_type == EMission::Pirate || _type == EMission::Comet || _type == EMission::TakeGold)
    {
        // reset
        m_arrowTarget = nullptr;
    }
}

void USpacelWidget::OnResetTimerMission(EMission _type)
{
    if (m_currentMission.Contains(_type))
    {
        if (UMissionPanelUserWidget* panelMission = SimplyUI::initSafetyFromName<UUserWidget, UMissionPanelUserWidget>(this, TEXT("WBP_Mission")))
        {
            panelMission->resetTimer(_type);
        }
    }
}

void USpacelWidget::OnChangeState(EGameState _state)
{
    if (_state == EGameState::LockPrepare)
    {
        this->SetVisibility(ESlateVisibility::Visible);

        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;
        TArray<APlayerState*> const& playerStates{ world->GetGameState()->PlayerArray };

        ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
        if (owningPlayerState == nullptr) return;
        FString owningPlayerTeam{ owningPlayerState->R_Team };

        Team = owningPlayerTeam;

        //set background color for ranking
        if (this->TeamColorDataAsset != nullptr)
        {
            SetBackgroundTeamColor(this->TeamColorDataAsset->GetColor<FSlateColor>(Team));
        }

        int i{ 0 };
        for (APlayerState* playerState : playerStates)
        {
            if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
            {
                if (spacelPlayerState->GetUniqueID() == owningPlayerState->GetUniqueID()) continue;

                if (spacelPlayerState->R_Team.Equals(owningPlayerTeam)
                    && this->AllyWidgets[i] != nullptr)
                {
                    this->AllyWidgets[i]->setWatcher(spacelPlayerState);
                    this->AllyWidgets[i]->Visibility = ESlateVisibility::Visible;
                    ++i;
                }
            }
        }

        if(this->GameModeDataAsset == nullptr) return;

        // timer for start animation
        float firstDelay = this->GameModeDataAsset->EndModuleTime / 4;
        float inRate = (this->GameModeDataAsset->EndModuleTime - firstDelay) / 2.0f;
        world->GetTimerManager().SetTimer(this->RedLightAnimationHandle, this, &USpacelWidget::RedLight, inRate, true, firstDelay);
    }
    else if (_state == EGameState::InGame)
    {
        BP_UnlockInput();
    }
    else if (_state == EGameState::EndGame)
    {
        BP_EndGame();
    }
}

void USpacelWidget::RedLight()
{
    BP_RedLight(m_currentIdRedLight);
    ++m_currentIdRedLight;
}

void USpacelWidget::addSkill(class SkillCountDown * _skill)
{
    if (this->SkillBarHorizontalBox == nullptr) return;
    if(_skill == nullptr) return;

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

void USpacelWidget::UpdateScore()
{
    if(this->TeamColorDataAsset == nullptr) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
    {
        TArray<FScore> scores = spacelGameState->R_Scores;

        for (int i {0} ; i < scores.Num() ; ++i)
        {
            if (i < this->TeamScoreWidgets.Num())
            {
                this->TeamScoreWidgets[i]->SetText(FText::FromString(FString::FromInt(scores[i].Score)));
            }

            if (i < this->TeamScoreColorWidgets.Num())
            {
                this->TeamScoreColorWidgets[i]->SetBrushColor(this->TeamColorDataAsset->GetColor<FColor>(scores[i].Team));
            }

        }
    }
}

void USpacelWidget::SetLatestEvent()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    ASpacelGameState* spacelGameState { Cast<ASpacelGameState>(world->GetGameState()) };
    if (!ensure(spacelGameState != nullptr)) return;

    FString latestEvent { spacelGameState->R_LatestEvent };

    if (latestEvent.Len() > 0)
    {
        if (latestEvent.Equals("GameEnded"))
        {
            FString winningTeam { spacelGameState->R_WinningTeam };

            if (this->EventTextBlock)
            {
                this->EventTextBlock->SetText(FText::FromString(winningTeam + " won!"));
            }
        }
        else if(latestEvent.Contains("Timer"))
        {
            if (this->TimerTextBlock)
            {
                latestEvent = latestEvent.Replace(TEXT("Timer"), TEXT(""));
                this->TimerTextBlock->SetText(FText::FromString(latestEvent));
            }
        }
    }
}

void USpacelWidget::SetAverragePlayerLatency()
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    float totalPlayerLatency{ 0.0f };
    for (float playerLatency : spacelGameInstance->PlayerLatencies)
    {
        totalPlayerLatency += playerLatency;
    }

    int32 num = spacelGameInstance->PlayerLatencies.Num();
    if (totalPlayerLatency > 0.0f && num != 0)
    {
        float averagePlayerLatency { totalPlayerLatency / num };
        FString pingString{ FString::FromInt(FMath::RoundToInt(averagePlayerLatency)) + " ms" };
        if (!ensure(this->PingTextBlock != nullptr)) return;
        this->PingTextBlock->SetText(FText::FromString(pingString));
    }
}

void USpacelWidget::OnUpdateCountProtection(int32 _value, int32 _max)
{
    if (_max > 0)
    {
        UpdatePercent(this->ProtectionProgressBar, this->ProtectionTextBlock, _value, _max);
    }
}

void USpacelWidget::OnUpdateCountSupport(int32 _value, int32 _max)
{
    if (_max > 0)
    {
        UpdatePercent(this->SupportProgressBar, this->SupportTextBlock, _value, _max);
    }
}

void USpacelWidget::OnShowMission(bool _show)
{
    if (UMissionPanelUserWidget* panelMission = SimplyUI::initSafetyFromName<UUserWidget, UMissionPanelUserWidget>(this, TEXT("WBP_Mission")))
    {
        panelMission->showMission(_show);
    }
}

void USpacelWidget::OnAddEffect(EEffect _type)
{
    if(this->EffectBarHorizontalBox == nullptr) return;
    if(this->EffectWidgetClass == nullptr) return;
    if(this->EffectDataAsset == nullptr) return;

    FString name = "Effect";
    name.Append(FString::FromInt((int)_type));
    UEffectWidget* effectWidget = CreateWidget<UEffectWidget, UHorizontalBox>(this->EffectBarHorizontalBox, this->EffectWidgetClass, *name);
    if (effectWidget != nullptr)
    {
        effectWidget->Effect = _type;
        FEffect const& effectParam = this->EffectDataAsset->getEffect(_type);
        effectWidget->SetEffect(effectParam.BackgroundColor, effectParam.Icone);
        this->EffectBarHorizontalBox->AddChildToHorizontalBox(effectWidget);
    }
}

void USpacelWidget::OnRemoveEffect(EEffect _type)
{
    if (this->EffectBarHorizontalBox == nullptr) return;

    TArray<UWidget*> widgets = this->EffectBarHorizontalBox->GetAllChildren();
    for (UWidget* widget : widgets)
    {
        if (UEffectWidget * effect = Cast<UEffectWidget>(widget))
        {
            if (effect->Effect == _type)
            {
                this->EffectBarHorizontalBox->RemoveChild(widget);
                if (this->EffectDataAsset != nullptr)
                {
                    FEffect const& param = this->EffectDataAsset->getEffect(_type);
                    OnRemoveEffectFx(param.Icone);
                }
                break;
            }
        }
    }

    if (_type == EEffect::Killed)
    {
        OnRespawn();
    }
}
