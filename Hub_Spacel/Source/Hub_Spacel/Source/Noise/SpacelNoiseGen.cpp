// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelNoiseGen.h"

SpacelNoiseGen::~SpacelNoiseGen()
{
}

float SpacelNoiseGen::getNoise(double _x, double _y, double _z)
{
	if (m_octaves > 1)
		return Noise.GetOctaveNoise(_x * m_xScale, _y * m_yScale, _z * m_zScale, m_octaves);
	else
		return Noise.GetNoise(_x * m_xScale, _y * m_yScale, _z * m_zScale);
}
