// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Hub_SpacelGameInstance.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include <functional>
#include "Util/DebugScreenMessage.h"

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

    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::EscapeMode), get(), mode, callbackSucced, callbackFailed, true));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::Repair), get(), mode, callbackSucced, callbackFailed, true));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::Katyusha), get(), mode, callbackSucced, callbackFailed, true));
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::HealPack), get(), mode, callbackSucced, callbackFailed, true));

    SetDelegateForPlayerState();

    UWorld const* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
    {
        spacelGameState->OnEndMissionDelegate.AddDynamic(this, &USkillComponent::OnMissionEnd);
    }
}

void USkillComponent::SetupSpecialSkill()
{
    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        if (this->SkillDataAsset == nullptr) return;

        ENetMode mode = this->GetNetMode();

        auto callbackSucced = std::bind(&USkillComponent::RPCClientSucced, this, std::placeholders::_1);
        auto callbackFailed = std::bind(&USkillComponent::RPCClientFailed, this, std::placeholders::_1, std::placeholders::_2);

        auto lb = [&](ESkillType _skilltype)
        {
            ESkill skill = (ESkill)spacelPlayerState->getSkillId(_skilltype);
            UUniqueSkillDataAsset* skillParam = this->SkillDataAsset->getSKill(skill);
            m_skills.Add(MakeUnique<SkillCountDown>(skillParam, get(), mode, callbackSucced, callbackFailed, false));

            if (spacelPlayerState->OnAddSkillUniqueDelegate != nullptr)
            {
                spacelPlayerState->OnAddSkillUniqueDelegate(m_skills.Last().Get());
            }
        };

        lb(ESkillType::Medium);
        lb(ESkillType::Hight);
    }
    else
    {
        FTimerHandle handle;
        this->GetWorld()->GetTimerManager().SetTimer(handle, this, &USkillComponent::SetupSpecialSkill, 1.0f, false, 0.0f);
    }
}

void USkillComponent::SetDelegateForPlayerState()
{
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
    else
    {
        FTimerHandle handle;
        this->GetWorld()->GetTimerManager().SetTimer(handle, this, &USkillComponent::SetDelegateForPlayerState, 1.0f, false, 0.0f);
    }
}

void USkillComponent::OnMissionEnd(EMission _type)
{
    if (_type != EMission::FirstBlood && _type != EMission::ScoreRace) return;

    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        auto lb = [&](ESkillType _skilltype)
        {
            ESkill skillId = (ESkill)spacelPlayerState->getSkillId(_skilltype);

            for (auto& skill : m_skills)
            {
                if (skill.IsValid() && skill->getSkillType() == skillId)
                {
                    skill->setActive(true);
                }
            }
        };

        if (_type == EMission::FirstBlood)
        {
            lb(ESkillType::Medium);
        }
        else if (_type == EMission::ScoreRace)
        {
            lb(ESkillType::Hight);
        }
    }
}

void USkillComponent::RPCServerUseSkill_Implementation(ESkill _skill)
{
    for (auto& skill : m_skills)
    {
        if (skill.IsValid())
        {
            if (skill.Get()->getSkillType() == _skill)
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
            if (skill.Get()->getSkillType() == _skill)
            {
                // no behaviour will be triggered, only UI
                skill.Get()->use(get()->GetWorld());
            }
        }
    }
}

UUniqueSkillDataAsset const* USkillComponent::getSkill(ESkill _skill) const
{
    for (auto& skill : m_skills)
    {
        if (skill.IsValid())
        {
            if (skill.Get()->getSkillType() == _skill)
            {
                return skill.Get()->getParam();
            }
        }
    }

    ensure(false);
    return nullptr;
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
        if (UUniqueSkillDataAsset const* skill = getSkill(_skill))
        {
            info = info.Replace(TEXT("%value%"), *FString::FromInt(skill->Value));
        }
    }
    else if (_returnValue == ESkillReturn::CountDown)
    {
        info = this->SkillDataAsset->CountDown;
    }
    else if (_returnValue == ESkillReturn::Unavailable)
    {
        if (UUniqueSkillDataAsset const* skill = getSkill(_skill))
        {
            info = skill->TextWhenFail;
        }
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
                    UUniqueSkillDataAsset const* param = skill.Get()->getParam();
                    if (param == nullptr)
                    {
                        ensure(false);
                        return;
                    }

                    bool keyDown = controller->IsInputKeyDown(param->Key);
                    bool& isDown = skill.Get()->isDown();
                    if (keyDown && !isDown)
                    {
                        isDown = true;
                        RPCServerUseSkill(param->Skill);
                    }
                    else if (!keyDown && isDown)
                    {
                        isDown = false;
                    }
                }

                skill.Get()->tick(_deltaTime);
            }
        }
    }

    removeSkill();
}

void USkillComponent::emergencyRedCube()
{
    // check if we already are in emergency
    for (auto & skill : m_skills)
    {
        if (skill.IsValid() && skill->getSkillType() == ESkill::Emergency)
        {
            return;
        }
    }

    auto callbackSucced = std::bind(&USkillComponent::RPCClientSucced, this, std::placeholders::_1);
    auto callbackFailed = std::bind(&USkillComponent::RPCClientFailed, this, std::placeholders::_1, std::placeholders::_2);
    ENetMode mode = this->GetNetMode();
    m_skills.Add(MakeUnique<SkillCountDown>(this->SkillDataAsset->getSKill(ESkill::Emergency), get(), mode, callbackSucced, callbackFailed, true));

    if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
    {
        if (spacelPlayerState->OnAddSkillUniqueDelegate != nullptr)
        {
            spacelPlayerState->OnAddSkillUniqueDelegate(m_skills.Last().Get());
        }
    }
}


void USkillComponent::RPCClientEmergencyRedCube_Implementation()
{
    emergencyRedCube();
}

void USkillComponent::emergencyRedCubeRemove()
{
    m_skillToRemove.Add(ESkill::Emergency);
}

void USkillComponent::RPCClientEmergencyRedCubeRemove_Implementation()
{
    emergencyRedCubeRemove();
}

void USkillComponent::removeSkill()
{
    for (auto type : m_skillToRemove)
    {
        if (m_skills.RemoveAll([&](auto const& _skill) {
            return _skill.IsValid() && _skill->getSkillType() == type;
            }))
        {
            if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(get()->GetPlayerState()))
            {
                if (spacelPlayerState->OnRemoveSkillUniqueDelegate != nullptr)
                {
                    spacelPlayerState->OnRemoveSkillUniqueDelegate(type);
                }
            }
        }
    }

    m_skillToRemove.Empty();
}