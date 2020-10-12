// Fill out your copyright notice in the Description page of Project Settings.


#include "ChainedLocation.h"

ChainedLocation::ChainedLocation(FVector const& _center, FVector const& _size, int const& _id)
	: m_center(_center)
	, m_size(_size)
	, m_mask(EFace::None)
    , m_id(_id)
{
	FVector half = m_size / 2.0f;

	TArray<FVector> const points = {
		{_center.X - half.X, _center.Y - half.Y, _center.Z - half.Z},
		{_center.X + half.X, _center.Y - half.Y, _center.Z - half.Z},
		{_center.X + half.X, _center.Y + half.Y, _center.Z - half.Z},
		{_center.X - half.X, _center.Y + half.Y, _center.Z - half.Z},
		{_center.X - half.X, _center.Y - half.Y, _center.Z + half.Z},
		{_center.X + half.X, _center.Y - half.Y, _center.Z + half.Z},
		{_center.X + half.X, _center.Y + half.Y, _center.Z + half.Z},
		{_center.X - half.X, _center.Y + half.Y, _center.Z + half.Z} };
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
	m_mask |= _face;		 
}

void ChainedLocation::removeMeToOtherFace()
{
	for (auto const& pair : m_neighbor)
	{
		pair.Value->removeFace(m_center);
	}
	m_neighbor.Empty();
}

void ChainedLocation::removeFace(FVector const& _location)
{
	for(int i = 0; i < m_neighbor.Num(); ++i)
	{
        if (!m_neighbor[i].Value.IsValid())
        {
            continue;
        }

		if (m_neighbor[i].Value->getCenter() == _location)
		{
			m_mask ^= m_neighbor[i].Key;
			m_neighbor.RemoveAt(i);
			break;
		}
	}
}

void ChainedLocation::createBox(FVector const& _location)
{
	FVector half = m_size / 2.0f;
	FVector worldCenter = m_center + _location;

	TArray<FVector> const points = {
        {worldCenter.X - half.X, worldCenter.Y - half.Y, worldCenter.Z - half.Z},
        {worldCenter.X + half.X, worldCenter.Y - half.Y, worldCenter.Z - half.Z},
        {worldCenter.X + half.X, worldCenter.Y + half.Y, worldCenter.Z - half.Z},
        {worldCenter.X - half.X, worldCenter.Y + half.Y, worldCenter.Z - half.Z},
        {worldCenter.X - half.X, worldCenter.Y - half.Y, worldCenter.Z + half.Z},
        {worldCenter.X + half.X, worldCenter.Y - half.Y, worldCenter.Z + half.Z},
        {worldCenter.X + half.X, worldCenter.Y + half.Y, worldCenter.Z + half.Z},
        {worldCenter.X - half.X, worldCenter.Y + half.Y, worldCenter.Z + half.Z} };
	m_box = FBox(points);
}

FString ChainedLocation::getXml() const
{
    FString xml;

    xml = "<location id=\"" + FString::FromInt(m_id) + "\" center=\"" + m_center.ToString() + "\" size=\"" + m_size.ToString() + "\">";
    for (auto const& pair : m_neighbor)
    {
        xml.Append("\n<face type=\"" + FString::FromInt((int)pair.Key) + "\" id=\"" + FString::FromInt(pair.Value->getId()) + "\"/>");
    }
    xml.Append("\n</location>");

    return std::move(xml);
}