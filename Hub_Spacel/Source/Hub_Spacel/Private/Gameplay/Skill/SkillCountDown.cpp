// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillCountDown.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Gameplay/Skill/SkillBehaviour.h"
#include "Components/ProgressBar.h"

SkillCountDown::SkillCountDown(SkillCountDown const& _cpy)
    : m_netMode(_cpy.m_netMode)
    , m_param(_cpy.m_param)
    , m_pawn(_cpy.m_pawn)
{
    if (m_netMode == ENetMode::NM_DedicatedServer)
    {
        m_behaviour = SkillFactory::create(m_param.Skill, m_pawn, m_netMode);
    }
    m_state = ECountDown::Available;
    m_state.init({ std::bind(&SkillCountDown::onAvailable, this),
                    std::bind(&SkillCountDown::onIng, this),
                    std::bind(&SkillCountDown::onCountDown, this) });
}

SkillCountDown::SkillCountDown(FSkill _skill, class ACommonPawn* _pawn, ENetMode _netMode)
    : m_netMode(_netMode)
    , m_param(_skill)
    , m_pawn(_pawn)
{
    if (m_netMode == ENetMode::NM_DedicatedServer)
    {
        m_behaviour = SkillFactory::create(m_param.Skill, m_pawn, m_netMode);
    }
    m_state = ECountDown::Available;
    m_state.init({ std::bind(&SkillCountDown::onAvailable, this),
                    std::bind(&SkillCountDown::onIng, this),
                    std::bind(&SkillCountDown::onCountDown, this) });
}

SkillCountDown::~SkillCountDown()
{
}

void SkillCountDown::tick(float _delta)
{
    m_currentTime += _delta;

    if (m_state == ECountDown::Ing)
    {
        if (m_currentTime >= m_param.FlatDuration)
        {
            m_state = ECountDown::CountDown;
        }
        else
        {
            updatePercent(m_progressBar, m_currentTime / m_param.FlatDuration);
        }
    }
    else if (m_state == ECountDown::CountDown)
    {
        if (m_currentTime >= m_param.CountDown)
        {
            m_state = ECountDown::Available;
        }
        else
        {
            updatePercent(m_progressBar, 1.0f - m_currentTime / m_param.CountDown);
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
    if (m_netMode == ENetMode::NM_DedicatedServer
        && (!m_behaviour.IsValid() || !m_behaviour.Get()->onStart()))
    {
        m_state = ECountDown::Available;
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

    if (m_state == ECountDown::Available)
    {
        m_state = ECountDown::Ing;
    }
}
