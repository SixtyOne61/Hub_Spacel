// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Kismet/GameplayStatics.h"

USkillComponent::USkillComponent()
    : UPlayerActorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USkillComponent::setupSkill()
{
    if(this->SkillDataAsset == nullptr) return;

    ENetMode mode = this->GetNetMode();

    // order is important
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::UseMatiere), get(), mode));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::EscapeMode), get(), mode));

    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        auto lb = [&](ESkillType _skilltype, ESkill _skill, uint8 _level)
        {
            if (spacelPlayerState->getSkillPoint(_skilltype) >= _level)
            {
                m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(_skill), get(), mode));
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

void USkillComponent::useSkill(float _slot)
{
    if(_slot >= m_skills.Num()) return;

    if (get() != nullptr)
    {
        m_skills[(int32)_slot].Get()->use(get()->GetWorld());
    }
}
