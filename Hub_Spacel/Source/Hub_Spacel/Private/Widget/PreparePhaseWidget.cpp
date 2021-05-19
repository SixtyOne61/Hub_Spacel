// Fill out your copyright notice in the Description page of Project Settings.

#include "PreparePhaseWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Util/SimplyUI.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/PlayerCardWidget.h"
#include "Widget/SelectorSKillWidget.h"
#include "DataAsset/TeamColorDataAsset.h"

void UPreparePhaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    RemainingSkillPointTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_RemainingSkillPoint"));
    TimeTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Time"));
    BonusTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("BonusList_Txt"));

    TArray<FName> skillName { TEXT("SelectorAttack"), TEXT("SelectorProtection"), TEXT("SelectorSupport") };
    SimplyUI::initArray(this, SelectorSkillWidget, skillName);
    for (USelectorSkillWidget* skillWidget : SelectorSkillWidget)
    {
        skillWidget->OnUpdateSkillDelegate.AddDynamic(this, &UPreparePhaseWidget::UpdateBonus);
    }

    TArray<FName> playerCardName { TEXT("Ally1"), TEXT("Ally2") };
    SimplyUI::initArray(this, PlayerCardWidget, playerCardName);

    if (!ensure(this->TimeTextBlock != nullptr)) return;
    this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->RemainingTime)));

    RegisterEvent();

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        //spacelGameState->OnStartPrepareDelegate.AddDynamic(this, &UPreparePhaseWidget::StartPrepare);
        //spacelGameState->OnLockPrepareDelegate.AddDynamic(this, &UPreparePhaseWidget::LockPrepare);
        //spacelGameState->OnStartGameDelegate.AddDynamic(this, &UPreparePhaseWidget::StartGame);
    }

    SetupOwningTeam();

    m_isLock = false;
}

void UPreparePhaseWidget::RegisterEvent()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        //owningPlayerState->OnUpdateRemainingSkillPointDelegate.AddDynamic(this, &UPreparePhaseWidget::UpdateRemainingSkillPoint);
        this->UpdateRemainingSkillPoint();
    }
    else
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &UPreparePhaseWidget::RegisterEvent, 1.0f, false);
    }
}

void UPreparePhaseWidget::UpdateRemainingSkillPoint()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        if (!ensure(this->RemainingSkillPointTextBlock != nullptr)) return;
        //this->RemainingSkillPointTextBlock->SetText(FText::FromString(FString::FromInt(owningPlayerState->getRemainingSkillPoint())));
    }
}

void UPreparePhaseWidget::SetTime()
{
    this->RemainingTime--;
    if (!ensure(this->TimeTextBlock != nullptr)) return;
    this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->RemainingTime)));

    if (this->RemainingTime == 0)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        world->GetTimerManager().ClearTimer(this->TimeHandle);
    }

    updatePlayerInfo();
}

void UPreparePhaseWidget::updatePlayerInfo()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    TArray<APlayerState*> const& playerStates{ world->GetGameState()->PlayerArray };

    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr) return;
    FString owningPlayerTeam{ owningPlayerState->R_Team };

    int i {0};
    for (APlayerState* playerState : playerStates)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            if(spacelPlayerState->GetUniqueID() == owningPlayerState->GetUniqueID()) continue;

            if (spacelPlayerState->R_Team.Equals(owningPlayerTeam)
                && this->PlayerCardWidget[i] != nullptr)
            {
                this->PlayerCardWidget[i]->Setup(spacelPlayerState);
                ++i;
            }
        }
    }
}

void UPreparePhaseWidget::LockPrepare()
{
    m_isLock = true;

    for (auto* selectorWidget : this->SelectorSkillWidget)
    {
        selectorWidget->setLock(true);
    }

    this->OnLock();
}

void UPreparePhaseWidget::StartPrepare()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(TimeHandle, this, &UPreparePhaseWidget::SetTime, 1.0f, true, 0.0f);
}

void UPreparePhaseWidget::StartGame()
{
    this->RemoveFromViewport();

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().ClearTimer(this->TimeHandle);
}

void UPreparePhaseWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().ClearTimer(this->TimeHandle);

    Super::NativeDestruct();
}

void UPreparePhaseWidget::SetupOwningTeam()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &UPreparePhaseWidget::SetupOwningTeam, 1.0f, false, 0.0f);
        return;
    }

    FString owningPlayerTeam{ owningPlayerState->R_Team };
    if (this->Colors != nullptr)
    {
        this->SetupOutline(this->Colors->GetColor<FSlateColor>(owningPlayerTeam));
    }
}

void UPreparePhaseWidget::UpdateBonus(ESkillType _type, uint8 _level)
{
    /*if (this->TreeSkillDesc != nullptr)
    {
        ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState());
        if (owningPlayerState == nullptr) return;

        FString descStr {};
        auto lb = [&](ESkillType _genType)
        {
            uint8 level = 0;//_genType == _type ? _level : owningPlayerState->getSkillPoint(_genType);
            FDesc const& desc = this->TreeSkillDesc->getDesc(_genType);

            for (int i = 0; i < level; ++i)
            {
                descStr.Append(desc.DescByLevel[i]);
                descStr.Append("\n");
            }
        };

        //lb(ESkillType::Attack);
        //lb(ESkillType::Protection);
        //lb(ESkillType::Support);

        if (this->BonusTextBlock != nullptr)
        {
            this->BonusTextBlock->SetText(FText::FromString(descStr));
        }
    }*/
}
