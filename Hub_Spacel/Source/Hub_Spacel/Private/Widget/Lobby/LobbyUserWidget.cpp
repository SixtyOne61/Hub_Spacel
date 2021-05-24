// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUserWidget.h"
#include "GameState/SpacelGameState.h"
#include "GameMode/FlyingGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/Common/SpinCarrouselWidget.h"
#include "Widget/Common/ItemCarrouselWidget.h"
#include "Util/SimplyUI.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/FlyingGameModeDataAsset.h"
#include "Player/SpacelPlayerState.h"
#include "Components/TextBlock.h"

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
    TimeTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Time"));
}

void ULobbyUserWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().ClearTimer(this->TimeHandle);

    Super::NativeDestruct();
}

void ULobbyUserWidget::setTimer(int _timer)
{
    if (!ensure(this->TimeTextBlock != nullptr)) return;

    this->Time = _timer;
    this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->Time)));
}

void ULobbyUserWidget::SetTime()
{
    if (this->Time > 0)
    {
        --this->Time;
        if (this->Time < 10)
        {
            FString str = "0";
            str.Append(FString::FromInt(this->Time));
            this->TimeTextBlock->SetText(FText::FromString(str));
        }
        else
        {
            this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->Time)));
        }
    }
}

void ULobbyUserWidget::spawnLobby3D()
{
    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        if (APawn* pawn = owningPlayerState->GetPawn())
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
}

void ULobbyUserWidget::StartLobby(EGameState _state)
{
    if (_state == EGameState::Prepare)
    {
        spawnLobby3D();
        SetupOwningTeam();
        // setup carrousel with low module
        setupSkill(this->LowSkill);
        if (this->GameModeDataAsset != nullptr)
        {
            setTimer(this->GameModeDataAsset->RemainingChooseModuleTime);
        }

        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        world->GetTimerManager().SetTimer(TimeHandle, this, &ULobbyUserWidget::SetTime, 1.0f, true, 0.0f);
    }
    else if (_state == EGameState::LockLowModule)
    {
        // save low module choice
        saveSkillChoosen(this->Carrousel->getIdSelected(), ESkillType::Low);
        // setup carrousel with medium module
        setupSkill(this->MediumSkill);
        if (this->GameModeDataAsset != nullptr)
        {
            setTimer(this->GameModeDataAsset->RemainingChooseModuleTime);
        }
    }
    else if (_state == EGameState::LockMediumModule)
    {
        // save medium module choice
        saveSkillChoosen(this->Carrousel->getIdSelected(), ESkillType::Medium);
        // setup carrousel with hight module
        setupSkill(this->HightSkill);
        if (this->GameModeDataAsset != nullptr)
        {
            setTimer(this->GameModeDataAsset->RemainingChooseModuleTime);
        }
    }
    else if (_state == EGameState::LockPrepare)
    {
        // save hight module choice
        saveSkillChoosen(this->Carrousel->getIdSelected(), ESkillType::Hight);
        if (this->GameModeDataAsset != nullptr)
        {
            setTimer(this->GameModeDataAsset->EndModuleTime);
        }
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
            datas.Add({ id, skill->BackgroundColorLobby, skill->Desc });
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