// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemCarrouselWidget.h"

void UItemCarrouselWidget::setupItems(FData const& _data)
{
    m_data.m_id = _data.m_id;
    m_data.m_backgroundColor = _data.m_backgroundColor;
    m_data.m_text = _data.m_text;
    BP_Setup(m_data.m_backgroundColor);
}