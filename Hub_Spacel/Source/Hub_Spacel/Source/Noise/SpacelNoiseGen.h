// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class HUB_SPACEL_API SpacelNoiseGen
{
public:
	SpacelNoiseGen()
		: m_xScale(0.02f)
		, m_yScale(0.02f)
		, m_zScale(1)
		, m_octaves(1)
	{}

	SpacelNoiseGen(double _scale, int _octaves)
		: m_zScale(1)
	{
		m_xScale = _scale;
		m_yScale = _scale;
		m_octaves = _octaves;
	}

	SpacelNoiseGen(double _scaleX, double _scaleY, int _octaves)
		: m_zScale(1)
	{
		m_xScale = _scaleX;
		m_yScale = _scaleY;
		m_octaves = _octaves;
	}

	~SpacelNoiseGen();

	// get / set
	inline void setScale(double _x, double _y) { m_xScale = _x, m_yScale = _y; }

	float getNoise(double _x, double _y, double _z);

public:
	double m_xScale;
	double m_yScale;
	double m_zScale;
	int  m_octaves;
};
