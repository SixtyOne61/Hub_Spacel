// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class HUB_SPACEL_API SpacelNoise
{
private:
	struct Grad
	{
		Grad(double _x, double _y, double _z)
			: m_x(_x)
			, m_y(_y)
			, m_z(_z)
			, m_w(0)
		{}

		double m_x;
		double m_y;
		double m_z;
		double m_w;

	};

	static SpacelNoise* m_instance;

	SpacelNoise();
public:
	~SpacelNoise();

	static SpacelNoise* getInstance()
	{
		if (!m_instance)
		{
			m_instance = new SpacelNoise();
		}
		return m_instance;
	}

	float getNoise(double _xin, double _yin, double _zin) const;
	int fastFloor(double _x) const;
	double dot(Grad const& _g, double _x, double _y, double _z) const;
	float getOctaveNoise(double _x, double _y, double _z, int _octaves) const;

public:
	// Skewing and unskewing factors for 2, 3, and 4 dimensions
	double m_f3;
	double m_g3;

	TArray<short> m_p;
	short m_perm[512];
	short m_permMod12[512];

	TArray<Grad> m_grad3;
};
