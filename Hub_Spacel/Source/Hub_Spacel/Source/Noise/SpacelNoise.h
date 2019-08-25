// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class NoiseGen
{
public:
	NoiseGen()
		: m_xScale(0.02f)
		, m_yScale(0.02f)
		, m_zScale(1)
		, m_octaves(1)
	{}

	NoiseGen(double _scale, int _octaves)
		: m_zScale(1)
	{
		m_xScale = _scale;
		m_yScale = _scale;
		m_octaves = _octaves;
	}

	NoiseGen(double _scaleX, double _scaleY, int _octaves)
		: m_zScale(1)
	{
		m_xScale = _scaleX;
		m_yScale = _scaleY;
		m_octaves = _octaves;
	}

	float getNoise(double _x, double _y, double _z)
	{
		if (m_octaves > 1)
			return Noise.GetOctaveNoise(x * XScale, y * YScale, z * ZScale, Octaves);
		else
			return Noise.GetNoise(x * XScale, y * YScale, z * ZScale);
	}

	inline void setScale(double _x, double _y) { m_xScale = _x, m_yScale = _y; }

public:
	double m_xScale;
	double m_yScale;
	double m_zScale;
	int  m_octaves;

	

		public float GetNoise(double x, double y, double z)
		{
			if (Octaves > 1)
				return Noise.GetOctaveNoise(x * XScale, y * YScale, z * ZScale, Octaves);
			else
				return Noise.GetNoise(x * XScale, y * YScale, z * ZScale);
		}
}

/**
 * 
 */
class HUB_SPACEL_API SpacelNoise
{
public:
	SpacelNoise();
	~SpacelNoise();

	float getNoise(double _xin, double _yin, double _zin);
	int fastFloor(double _x);

public:
	// Skewing and unskewing factors for 2, 3, and 4 dimensions
	double F3;
	double G3;

	TArray<short> p;
	short perm[512];
	short permMod12[512];
};
