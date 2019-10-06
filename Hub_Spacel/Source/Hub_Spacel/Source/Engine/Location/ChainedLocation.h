// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class EFace : uint32
{
	None = 0,
	Top = 1 << 0, 
	Bot = 1 << 1,
	Front = 1 << 2,
	Back = 1 << 3,
	Right = 1 << 4,
	Left = 1 << 5,
};
ENUM_CLASS_FLAGS(EFace);


/**
 * 
 */
class HUB_SPACEL_API ChainedLocation
{
public:
	ChainedLocation(FVector && _center, float _size);
	~ChainedLocation();

	// -- get / set
	inline EFace getMask() const { return m_mask; }
	inline FVector const& getCenter() const { return m_center; }

	// -- add a neighbor
	void addNeighbor(EFace _face, TSharedPtr<ChainedLocation> _neighbor);
	// -- remove this objet to other face
	void removeMeToOtherFace();
	// -- remove a face
	void removeFace(FVector const& _location);

private:
	// center of edgde
	FVector m_center;
	// cube size
	float m_size;
	// neighbor mask
	EFace m_mask;
	// neighbor list
	TArray<TPair<EFace, TSharedPtr<ChainedLocation>>> m_neighbor;
};
