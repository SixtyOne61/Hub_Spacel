// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelNoiseGen.h"
#include "SpacelNoise.h"

SpacelNoiseGen::~SpacelNoiseGen()
{
}

float SpacelNoiseGen::getNoise(double _x, double _y, double _z)
{
	if (m_octaves > 1)
		return SpacelNoise::getInstance()->getOctaveNoise(_x * m_xScale, _y * m_yScale, _z * m_zScale, m_octaves);
	else
		return SpacelNoise::getInstance()->getNoise(_x * m_xScale, _y * m_yScale, _z * m_zScale);
}
