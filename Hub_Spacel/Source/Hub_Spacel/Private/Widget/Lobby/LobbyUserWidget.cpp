// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUserWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/Common/SpinCarrouselWidget.h"
#include "Widget/Common/ItemCarrouselWidget.h"
#include "Util/SimplyUI.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "Player/SpacelPlayerState.h"

void ULobbyUserWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &ULobbyUserWidget::StartLobby);
    }

    Carrousel = SimplyUI::initSafetyFromName<UUserWidget, USpinCarrouselWidget>(this, TEXT("WBP_SpinCarrousel"));
}

void ULobbyUserWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void ULobbyUserWidget::StartLobby(EGameState _state)
{
    if (_state == EGameState::Prepare)
    {
        SetupOwningTeam();
        // setup carrousel with low module
        setupSkill(this->LowSkill);
    }
    else if (_state == EGameState::LockLowModule)
    {
        // save low module choice
        saveSkillChoosen(this->Carrousel->getIdSelected(), ESkillType::Low);
        // setup carrousel with medium module
        setupSkill(this->MediumSkill);
    }
    else if (_state == EGameState::LockMediumModule)
    {
        // save medium module choice
        saveSkillChoosen(this->Carrousel->getIdSelected(), ESkillType::Medium);
        // setup carrousel with hight module
        setupSkill(this->HightSkill);
    }
    else if (_state == EGameState::LockPrepare)
    {
        // save hight module choice
        saveSkillChoosen(this->Carrousel->getIdSelected(), ESkillType::Hight);
    }
    else if (_state == EGameState::InGame)
    {
        this->RemoveFromViewport();
    }
}

void ULobbyUserWidget::setupSkill(TArray<ESkill> const& _skills)
{
    if(this->SkillDataAsset == nullptr) return;

    TArray<UItemCarrouselWidget::FData> datas {};
    for (ESkill type : _skills)
    {
        if (UUniqueSkillDataAsset const* skill = this->SkillDataAsset->getSKill(type))
        {
            uint8 id = (uint8)type;
            datas.Add({ id, skill->BackgroundColorLobby });
        }
    }

    this->Carrousel->setupItems(datas);
}

void ULobbyUserWidget::saveSkillChoosen(uint8 _id, ESkillType _type)
{
    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        owningPlayerState->RPCAddSkill(_id, _type);
    }
}

void ULobbyUserWidget::SetupOwningTeam()
{
    ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState());
    if (owningPlayerState == nullptr)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &ULobbyUserWidget::SetupOwningTeam, 1.0f, false, 0.0f);
    }
    else
    {
        FString owningPlayerTeam = owningPlayerState->R_Team;
        if (this->Colors != nullptr)
        {
            this->BP_SetupOutline(this->Colors->GetColor<FSlateColor>(owningPlayerTeam));
        }
    }

}