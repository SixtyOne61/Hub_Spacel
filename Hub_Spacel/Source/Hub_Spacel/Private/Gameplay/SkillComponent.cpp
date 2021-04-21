// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Kismet/GameplayStatics.h"
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
    auto callbackFailed = std::bind(&USkillComponent::RPCClientFailed, this, std::placeholders::_1);

    // order is important
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::EscapeMode), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::RepairProtection), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::RepairSupport), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::GiveAlly1), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::GiveAlly2), get(), mode, callbackSucced, callbackFailed));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::NinePack), get(), mode, callbackSucced, callbackFailed));

    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        // last in first out so 3 - 4
        TArray<FKey> defaultKeyboard{ FKey("Apostrophe"), FKey("Quote") };

        auto lb = [&](ESkillType _skilltype, ESkill _skill, uint8 _level)
        {
            if (spacelPlayerState->getSkillPoint(_skilltype) >= _level)
            {
                FSkill skill = this->SkillDataAsset->getSKill(_skill);
                skill.Key = defaultKeyboard.Pop();
                m_skills.Add(MakeUnique<SkillCountDown>(skill, get(), mode, callbackSucced, callbackFailed));
            }
        };

        uint8 levelSpecial = this->SkillDataAsset->LevelSpecial;
        lb(ESkillType::Attack, ESkill::SpecialAttack, levelSpecial);
        lb(ESkillType::Protection, ESkill::SpecialProtection, levelSpecial);
        lb(ESkillType::Support, ESkill::SpecialSupport, levelSpecial);

        uint8 levelMetaForm = this->SkillDataAsset->LevelMetaForm;
        lb(ESkillType::Attack, ESkill::MetaFormAttack, levelMetaForm);
        lb(ESkillType::Protection, ESkill::MetaFormProtection, levelMetaForm);
        lb(ESkillType::Support, ESkill::MetaFormSupport, levelMetaForm);
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

void USkillComponent::RPCClientFailed_Implementation(ESkill _skill)
{
    for (auto& skill : m_skills)
    {
        if (skill.IsValid())
        {
            if (skill.Get()->getParam().Skill == _skill)
            {
                // send text failed to player
                if (get() != nullptr)
                {
                    get()->OnSendInfoPlayerDelegate.Broadcast(skill.Get()->getParam().TextWhenFail);
                }
            }
        }
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

