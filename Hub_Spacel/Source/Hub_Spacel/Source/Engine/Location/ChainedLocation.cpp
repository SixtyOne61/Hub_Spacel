// Fill out your copyright notice in the Description page of Project Settings.


#include "ChainedLocation.h"

ChainedLocation::ChainedLocation(FVector const& _center, float _size)
	: m_center(_center)
	, m_size(_size)
	, m_mask(EFace::None)
{
}

ChainedLocation::~ChainedLocation()
{
}

void ChainedLocation::addNeighbor(EFace _face, TSharedPtr<ChainedLocation> _neighbor)
{
	if (EnumHasAllFlags(m_mask, _face))
	{
		return;
	}

	m_neighbor.Add(TPair<EFace, TSharedPtr<ChainedLocation>>(_face, _neighbor));
	m_mask &= _face;		 
}