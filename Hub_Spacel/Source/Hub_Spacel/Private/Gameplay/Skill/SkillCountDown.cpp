// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillCountDown.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Gameplay/Skill/SkillBehaviour.h"

SkillCountDown::SkillCountDown(SkillCountDown const& _cpy)
    : m_param(_cpy.m_param)
    , m_pawn(_cpy.m_pawn)
{
    m_behaviour = SkillFactory::create(m_param.Skill, m_pawn);
    m_state = ECountDown::Available;
    m_state.init({ std::bind(&SkillCountDown::onAvailable, this),
                        std::bind(&SkillCountDown::onIng, this),
                        std::bind(&SkillCountDown::onCountDown, this) });
}

SkillCountDown::SkillCountDown(FSkill _skill, class AShipPawn* _pawn)
    : m_param(_skill)
    , m_pawn(_pawn)
{
    m_behaviour = SkillFactory::create(m_param.Skill, m_pawn);
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

    if (m_state == ECountDown::Ing && m_currentTime >= m_param.Duration)
    {
        m_state = ECountDown::CountDown;
    }
    else if (m_state == ECountDown::CountDown && m_currentTime >= m_param.CoundDown)
    {
        m_state = ECountDown::Available;
    }
}

void SkillCountDown::onAvailable()
{
    m_currentTime = 0.0f;
    if(m_behaviour.IsValid())
    {
        m_behaviour.Get()->onEndCountDown();
    }
}

void SkillCountDown::onIng()
{
    m_currentTime = 0.0f;
    if (!m_behaviour.IsValid() || !m_behaviour.Get()->onStart())
    {
        m_state = ECountDown::Available;
    }
}

void SkillCountDown::onCountDown()
{
    m_currentTime = 0.0f;
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
