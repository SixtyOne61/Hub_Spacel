// Fill out your copyright notice in the Description page of Project Settings.


#include "ChainedLocation.h"

ChainedLocation::ChainedLocation(FVector && _center, float _size)
	: m_center(std::move(_center))
	, m_size(_size)
	, m_mask(EFace::None)
{
	float half = m_size / 2.0f;

	TArray<FVector> const points = {
		{_center.X - half, _center.Y - half, _center.Z - half},
		{_center.X + half, _center.Y - half, _center.Z - half},
		{_center.X + half, _center.Y + half, _center.Z - half},
		{_center.X - half, _center.Y + half, _center.Z - half},
		{_center.X - half, _center.Y - half, _center.Z + half},
		{_center.X + half, _center.Y - half, _center.Z + half},
		{_center.X + half, _center.Y + half, _center.Z + half},
		{_center.X - half, _center.Y + half, _center.Z + half} };
	m_box = FBox(points);
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

void ChainedLocation::removeMeToOtherFace()
{
	for (auto pair : m_neighbor)
	{
		pair.Value->removeFace(m_center);
	}
	m_neighbor.Empty();
}

void ChainedLocation::removeFace(FVector const& _location)
{
	for(int i = 0; i < m_neighbor.Num(); ++i)
	{
		if (m_neighbor[i].Value->getCenter() == _location)
		{
			m_mask ^= m_neighbor[i].Key;
			m_neighbor.RemoveAt(i);
			break;
		}
	}
}