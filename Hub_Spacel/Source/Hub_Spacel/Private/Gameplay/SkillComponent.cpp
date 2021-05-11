// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Hub_SpacelGameInstance.h"
#include <functional>

USkillComponent::USkillComponent()
    : UPlayerActorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USkillComponent::setupSkill()
{
    if(this->SkillDataAsset == nullptr) return;

    ENetMode mode = this->GetNetMode();

    auto callbackSucced = std::bind(&USkillComponent::RPCClientSucced, this, std::placeholders::_1);
    auto callbackFailed = std::bind(&USkillComponent::RPCClientFailed, this, std::placeholders::_1, std::placeholders::_2);

    // order is important
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::EscapeMode), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::RepairProtection), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::RepairSupport), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::GiveAlly1), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::GiveAlly2), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::NinePack), get(), mode, callbackSucced, callbackFailed));

    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        if (spacelPlayerState->OnAddSkillUniqueDelegate != nullptr)
        {
            for (auto const& skill : m_skills)
            {
                spacelPlayerState->OnAddSkillUniqueDelegate(skill.Get());
            }
        }
    }

    if (AShipPawn* pawn = get<AShipPawn>())
    {
        pawn->OnEndMissionDelegate.AddDynamic(this, &USkillComponent::OnMissionEnd);
    }
}

void USkillComponent::OnMissionEnd(EMission _type)
{
    if(_type != EMission::FirstBlood && _type != EMission::ScoreRace) return;

    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        if (this->SkillDataAsset == nullptr) return;

        ENetMode mode = this->GetNetMode();

        auto callbackSucced = std::bind(&USkillComponent::RPCClientSucced, this, std::placeholders::_1);
        auto callbackFailed = std::bind(&USkillComponent::RPCClientFailed, this, std::placeholders::_1, std::placeholders::_2);

        // last in first out so 3 - 4
        TArray<FKey> defaultKeyboard = this->DefaultKeyboard;

        auto lb = [&](ESkillType _skilltype, ESkill _skill, uint8 _level, FKey _key)
        {
            if (spacelPlayerState->getSkillPoint(_skilltype) >= _level)
            {
                FSkill skill = this->SkillDataAsset->getSKill(_skill);
                skill.Key = _key;
                m_skills.Add(MakeUnique<SkillCountDown>(skill, get(), mode, callbackSucced, callbackFailed));

                if (spacelPlayerState->OnAddSkillUniqueDelegate != nullptr)
                {
                    spacelPlayerState->OnAddSkillUniqueDelegate(m_skills.Last().Get());
                }
            }
        };

        if (_type == EMission::FirstBlood)
        {
            uint8 levelSpecial = this->SkillDataAsset->LevelSpecial;
            lb(ESkillType::Attack, ESkill::SpecialAttack, levelSpecial, this->DefaultKeyboard[0]);
            lb(ESkillType::Protection, ESkill::SpecialProtection, levelSpecial, this->DefaultKeyboard[0]);
            lb(ESkillType::Support, ESkill::SpecialSupport, levelSpecial, this->DefaultKeyboard[0]);
        }
        else if (_type == EMission::ScoreRace)
        {
            uint8 levelMetaForm = this->SkillDataAsset->LevelMetaForm;
            lb(ESkillType::Attack, ESkill::MetaFormAttack, levelMetaForm, this->DefaultKeyboard[1]);
            lb(ESkillType::Protection, ESkill::MetaFormProtection, levelMetaForm, this->DefaultKeyboard[1]);
            lb(ESkillType::Support, ESkill::MetaFormSupport, levelMetaForm, this->DefaultKeyboard[1]);
        }
    }
}

void USkillComponent::RPCServerUseSkill_Implementation(ESkill _skill)
{
    for (auto& skill : m_skills)
    {
        if (skill.IsValid())
        {
            if (skill.Get()->getParam().Skill == _skill)
            {
                skill.Get()->use(get()->GetWorld());
            }
        }
    }
}

void USkillComponent::RPCClientSucced_Implementation(ESkill _skill)
{
    for (auto& skill : m_skills)
    {
        if (skill.IsValid())
        {
            if (skill.Get()->getParam().Skill == _skill)
            {
                // no behaviour will be triggered, only UI
                skill.Get()->use(get()->GetWorld());
            }
        }
    }
}

FSkill const& USkillComponent::getSkill(ESkill _skill) const
{
    for (auto& skill : m_skills)
    {
        if (skill.IsValid())
        {
            if (skill.Get()->getParam().Skill == _skill)
            {
                return skill.Get()->getParam();
            }
        }
    }

    ensure(false);
    static FSkill fail {};
    return fail;
}

void USkillComponent::RPCClientFailed_Implementation(ESkill _skill, ESkillReturn _returnValue)
{
    if(this->SkillDataAsset == nullptr) return;

    FString info = "";

    if (_returnValue == ESkillReturn::InternError)
    {
        info = this->SkillDataAsset->InternError;
    }
    else if (_returnValue == ESkillReturn::NoMater)
    {
        info = this->SkillDataAsset->NotEnoughMater;
        FSkill const& skill = getSkill(_skill);

        info = info.Replace(TEXT("%value%"), *FString::FromInt(skill.Value));
    }
    else if (_returnValue == ESkillReturn::CountDown)
    {
        info = this->SkillDataAsset->CountDown;
    }
    else if (_returnValue == ESkillReturn::Unavailable)
    {
        FSkill const& skill = getSkill(_skill);
        info = skill.TextWhenFail;
    }

    // send text failed to player
    if (get() != nullptr && info != "")
    {
        get()->OnSendInfoPlayerDelegate.Broadcast(info);
    }
}

void USkillComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
    Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    if (get() != nullptr)
    {
        for (auto& skill : m_skills)
        {
            if (skill.IsValid())
            {
                // client side
                if (APlayerController* controller = UGameplayStatics::GetPlayerController(get()->GetWorld(), 0))
                {
                    bool keyDown = controller->IsInputKeyDown(skill.Get()->getParam().Key);
                    bool& inputState = skill.Get()->inputeState();
                    if (keyDown && !inputState)
                    {
                        inputState = true;
                        RPCServerUseSkill(skill.Get()->getParam().Skill);
                    }
                    else if (!keyDown && inputState)
                    {
                        inputState = false;
                    }
                }

                skill.Get()->tick(_deltaTime);
            }
        }
    }
}

