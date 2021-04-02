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
#include "Player/ShipPawn.h"
#include "Gameplay/SkillComponent.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyUI.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/EffectDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "Widget/AllyWidget.h"
#include "Widget/SkillWidget.h"
#include "Widget/EffectWidget.h"
#include "Widget/ScoreUserWidget.h"
#include "Factory/SpacelFactory.h"
#include "Styling/SlateColor.h"

void USpacelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MatiereTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Matiere"));
    EventTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Event"));
    PingTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Ping"));
    ProtectionProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Protection"));
    SupportProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Support"));
    ScoreWidget = SimplyUI::initSafetyFromName<UUserWidget, UScoreUserWidget>(this, TEXT("WBP_Score"));
    SkillBarHorizontalBox = SimplyUI::initSafetyFromName<UUserWidget, UHorizontalBox>(this, TEXT("SkillBar"));
    EffectBarHorizontalBox = SimplyUI::initSafetyFromName<UUserWidget, UHorizontalBox>(this, TEXT("EffectBar"));

    TArray<FName> allyNames { TEXT("Widget_Ally1"), TEXT("Widget_Ally2") };
    SimplyUI::initArray(this, AllyWidgets, allyNames);

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetLatestEventHandle, this, &USpacelWidget::SetLatestEvent, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetAverragePlayerLatencyHandle, this, &USpacelWidget::SetAverragePlayerLatency, 1.0f, true, 1.0f);

    this->SetVisibility(ESlateVisibility::Hidden);
    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnStartGameDelegate.AddDynamic(this, &USpacelWidget::StartGame);
    }

    AShipPawn* shipPawn { this->GetOwningPlayerPawn<AShipPawn>() };
    if (shipPawn != nullptr)
    {
        shipPawn->OnEndUpdateMatiereDelegate.AddDynamic(this, &USpacelWidget::OnUpdateMatiere);

        shipPawn->ModuleComponent->OnUpdateCountProtectionDelegate.AddDynamic(this, &USpacelWidget::OnUpdateCountProtection);
        shipPawn->ModuleComponent->OnUpdateCountSupportDelegate.AddDynamic(this, &USpacelWidget::OnUpdateCountSupport);

        shipPawn->OnShowScoreDelegate.AddDynamic(this, &USpacelWidget::OnShowScore);
        shipPawn->OnAddEffectDelegate.AddDynamic(this, &USpacelWidget::OnAddEffect);
        shipPawn->OnRemoveEffectDelegate.AddDynamic(this, &USpacelWidget::OnRemoveEffect);
    }
}

void USpacelWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (world)
    {
        world->GetTimerManager().ClearTimer(SetLatestEventHandle);
        world->GetTimerManager().ClearTimer(SetAverragePlayerLatencyHandle);
        world->GetTimerManager().ClearTimer(SetSpeedHandle);
    }
    Super::NativeDestruct();
}

void USpacelWidget::NativeTick(const FGeometry& _myGeometry, float _deltaTime)
{
    Super::NativeTick(_myGeometry, _deltaTime);
    UpdateScore();
}

void USpacelWidget::StartGame()
{
    this->SetVisibility(ESlateVisibility::Visible);

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    TArray<APlayerState*> const& playerStates{ world->GetGameState()->PlayerArray };

    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr) return;
    FString owningPlayerTeam{ owningPlayerState->Team };

    Team = owningPlayerTeam;

    //set background color for ranking
    if (this->TeamColorDataAsset != nullptr)
    {
        SetBackgroundRanking(this->TeamColorDataAsset->GetColor<FSlateColor>(Team));
    }

    // create score array
    if (this->ScoreWidget != nullptr)
    {
        this->ScoreWidget->initScoreArray();
    }

    int i{ 0 };
    for (APlayerState* playerState : playerStates)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            if (spacelPlayerState->GetUniqueID() == owningPlayerState->GetUniqueID()) continue;

            if (spacelPlayerState->Team.Equals(owningPlayerTeam)
                && this->AllyWidgets[i] != nullptr)
            {
                this->AllyWidgets[i]->setWatcher(spacelPlayerState);
                this->AllyWidgets[i]->Visibility = ESlateVisibility::Visible;
                ++i;
            }
        }
    }

    // create skill bar
    if (this->SkillBarHorizontalBox == nullptr) return;
    if (this->SkillWidgetClass == nullptr) return;
    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        if(shipPawn->SkillComponent == nullptr) return;
        TArray<TUniquePtr<SkillCountDown>> const& skills = shipPawn->SkillComponent->getSkills();
        for (auto const& skillPtr : skills)
        {
            FSkill const& skill = skillPtr.Get()->getParam();
            FString name = "Skill";
            name.Append(FString::FromInt((int)skill.Skill));
            USkillWidget* skillWidget = CreateWidget<USkillWidget, UHorizontalBox>(this->SkillBarHorizontalBox, this->SkillWidgetClass, *name);
            skillWidget->SetSkill(skill.BackgroundColorBtn, skill.IconeBtn);
            this->SkillBarHorizontalBox->AddChildToHorizontalBox(skillWidget);

            UProgressBar * progress = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(skillWidget, TEXT("ProgressBar_Skill"));
            skillPtr->addProgressBar(progress);
        }
    }
}

void USpacelWidget::UpdateScore()
{
    if(this->ScoreWidget == nullptr) return;
    this->ScoreWidget->updateScore();

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        TArray<FScore> scores = spacelGameState->R_Scores;

        // search ranking
        scores.Sort([](FScore const& _s1, FScore const& _s2)
        {
            return _s1.Score > _s2.Score;
        });

        uint8 rank = 1;
        for (FScore const& score : scores)
        {
            if (score.Team == this->Team)
            {
                SetRanking(rank);
                break;
            }
            ++rank;
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
        else
        {
            if (this->EventTextBlock)
            {
                this->EventTextBlock->SetText(FText::FromString(latestEvent));
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
        FString pingString{ "Ping: " + FString::FromInt(FMath::RoundToInt(averagePlayerLatency)) + "ms" };
        if (!ensure(this->PingTextBlock != nullptr)) return;
        this->PingTextBlock->SetText(FText::FromString(pingString));
    }
}

void USpacelWidget::OnUpdateMatiere(int32 _value)
{
    if (this->MatiereTextBlock != nullptr)
    {
        this->MatiereTextBlock->SetText(FText::FromString(FString::FromInt(_value)));
    }
}

void USpacelWidget::OnUpdateCountProtection(int32 _value, int32 _max)
{
    if (_max > 0)
    {
        updatePercent(this->ProtectionProgressBar, (float)_value / (float)_max);
    }
}

void USpacelWidget::OnUpdateCountSupport(int32 _value, int32 _max)
{
    if (_max > 0)
    {
        updatePercent(this->SupportProgressBar, (float)_value / (float)_max);
    }
}

void USpacelWidget::updatePercent(UProgressBar* _progressBar, float _percent)
{
    if (_progressBar != nullptr)
    {
        _progressBar->SetPercent(_percent);
    }
}

void USpacelWidget::OnShowScore(bool _show)
{
    setVisibility(this->ScoreWidget, _show);
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
                break;
            }
        }
    }
}