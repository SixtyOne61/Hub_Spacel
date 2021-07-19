// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillCountDown.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Gameplay/Skill/SkillBehaviour.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "Components/ProgressBar.h"
#include "Player/Common/CommonPawn.h"

SkillCountDown::SkillCountDown(SkillCountDown const& _cpy)
    : m_netMode(_cpy.m_netMode)
    , m_param(_cpy.m_param)
    , m_pawn(_cpy.m_pawn)
{
    if (m_netMode == ENetMode::NM_DedicatedServer)
    {
        m_behaviour = SkillFactory::create(getSkillType(), m_pawn, m_netMode);
    }
    m_state = ECountDown::Available;
    m_state.init({ std::bind(&SkillCountDown::onAvailable, this),
                    std::bind(&SkillCountDown::onIng, this),
                    std::bind(&SkillCountDown::onCountDown, this) });

    m_callbackSucced = _cpy.m_callbackSucced;
    m_callbackFailed = _cpy.m_callbackFailed;
}

SkillCountDown::SkillCountDown(UUniqueSkillDataAsset const* _skill, class ACommonPawn* _pawn, ENetMode _netMode, std::function<void(ESkill)> _callbackSucced, std::function<void(ESkill, ESkillReturn)> _callbackFailed, bool _isActive)
    : m_netMode(_netMode)
    , m_pawn(_pawn)
    , m_isActive(_isActive)
    , m_callbackSucced(_callbackSucced)
    , m_callbackFailed(_callbackFailed)
{
    m_param = _skill;

    if (m_netMode == ENetMode::NM_DedicatedServer)
    {
        m_behaviour = SkillFactory::create(getSkillType(), m_pawn, m_netMode);
    }
    m_state = ECountDown::Available;
    m_state.init({ std::bind(&SkillCountDown::onAvailable, this),
                    std::bind(&SkillCountDown::onIng, this),
                    std::bind(&SkillCountDown::onCountDown, this) });
}

SkillCountDown::~SkillCountDown()
{
}

ESkill SkillCountDown::getSkillType() const
{
    if (m_param != nullptr)
    {
        return m_param->Skill;
    }

    ensure(false);
    return (ESkill)0;
}

void SkillCountDown::tick(float _delta)
{
    m_currentTime += _delta;

    if (m_param == nullptr)
    {
        ensure(false);
        return;
    }

    if (m_state == ECountDown::Ing)
    {
        if (m_currentTime >= m_param->FlatDuration)
        {
            m_state = ECountDown::CountDown;
        }
        else
        {
            updatePercent(m_progressBar, m_currentTime / m_param->FlatDuration);
        }
    }
    else if (m_state == ECountDown::CountDown)
    {
        float reducCountDown = 1.0f;
        if (m_pawn != nullptr)
        {
            reducCountDown = (100.0f - m_pawn->m_bonusCountDown) / 100.0f;
        }

        if (m_currentTime >= (m_param->CountDown * reducCountDown))
        {
            m_state = ECountDown::Available;
        }
        else
        {
            updatePercent(m_progressBar, 1.0f - m_currentTime / (m_param->CountDown * reducCountDown));
        }
    }
}

void SkillCountDown::updatePercent(UProgressBar* _progressBar, float _percent)
{
    if (_progressBar != nullptr)
    {
        _progressBar->SetPercent(_percent);
    }
}

void SkillCountDown::addProgressBar(class UProgressBar* _progressBar)
{
    m_progressBar = _progressBar;
    if (m_progressBar != nullptr)
    {
        m_progressBar->SetPercent(m_isActive ? 0.0f : 1.0f);
    }
}

void SkillCountDown::setActive(bool _val)
{
    m_isActive = _val;
    if (m_progressBar != nullptr)
    {
        m_progressBar->SetPercent(m_isActive ? 0.0f : 1.0f);
    }
}

void SkillCountDown::onAvailable()
{
    m_currentTime = 0.0f;
    updatePercent(m_progressBar, 0.0f);
    if(m_behaviour.IsValid())
    {
        m_behaviour.Get()->onEndCountDown();
    }
}

void SkillCountDown::onIng()
{
    m_currentTime = 0.0f;
    updatePercent(m_progressBar, 0.0f);
    if (m_netMode == ENetMode::NM_DedicatedServer)
    {
        ESkillReturn returnValue = m_behaviour.Get()->onStart();
        if (m_behaviour.IsValid() && returnValue == ESkillReturn::Success)
        {
            if (m_callbackSucced != nullptr)
            {
                m_callbackSucced(getSkillType());
            }
        }
        else
        {
            m_state = ECountDown::Available;
            m_callbackFailed(getSkillType(), returnValue);
        }
    }
}

void SkillCountDown::onCountDown()
{
    m_currentTime = 0.0f;
    updatePercent(m_progressBar, 1.0f);
    if (m_behaviour.IsValid()) m_behaviour.Get()->onEnd();
}

void SkillCountDown::use(class UWorld* _context)
{
    if(_context == nullptr) return;
    if(m_isActive == false) return;

    if (m_state == ECountDown::Available)
    {
        m_state = ECountDown::Ing;
    }
    else if(m_state == ECountDown::CountDown)
    {
        m_callbackFailed(getSkillType(), ESkillReturn::CountDown);
    }
}
