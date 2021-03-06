// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelNoise.h"


SpacelNoise* SpacelNoise::m_instance = nullptr;

SpacelNoise::SpacelNoise()
	: m_f3(1.0 / 3.0f)
	, m_g3(1.0 / 6.0f)
{
	m_grad3 = {
		   Grad(1,1,0), Grad(-1,1,0), Grad(1,-1,0), Grad(-1,-1,0),
		   Grad(1,0,1), Grad(-1,0,1), Grad(1,0,-1), Grad(-1,0,-1),
		   Grad(0,1,1), Grad(0,-1,1), Grad(0,1,-1), Grad(0,-1,-1)
	};

	m_p = {
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
			247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
			74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
			65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
			52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
			119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
			218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
			184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	for (int i = 0; i < 512; i++)
	{
		m_perm[i] = m_p[i & 255];
		m_permMod12[i] = (short)(m_perm[i] % 12);
	}
}

SpacelNoise::~SpacelNoise()
{
}

float SpacelNoise::getNoise(double _xin, double _yin, double _zin) const
{
	double n0 = 0;
	double n1 = 0;
	double n2 = 0;
	double n3 = 0;
	double s = (_xin + _yin +_zin) * m_f3;

	int i = fastFloor(_xin + s);
	int j = fastFloor(_yin + s);
	int k = fastFloor(_zin + s);

	double t = (i + j + k) * m_g3;

	double X0 = i - t; // Unskew the cell origin back to (x,y,z) space
	double Y0 = j - t;
	double Z0 = k - t;
	double x0 = _xin - X0; // The x,y,z distances from the cell origin
	double y0 = _yin - Y0;
	double z0 = _zin - Z0;
	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

	if (x0 >= y0)
	{
		if (y0 >= z0)
		{
			i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
		} // X Y Z order
		else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
		else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
	}
	else
	{ // x0<y0
		if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
		else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
		else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
	}

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
			// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
			// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
			// c = 1/6.
	double x1 = x0 - i1 + m_g3; // Offsets for second corner in (x,y,z) coords
	double y1 = y0 - j1 + m_g3;
	double z1 = z0 - k1 + m_g3;
	double x2 = x0 - i2 + 2.0 * m_g3; // Offsets for third corner in (x,y,z) coords
	double y2 = y0 - j2 + 2.0 * m_g3;
	double z2 = z0 - k2 + 2.0 * m_g3;
	double x3 = x0 - 1.0 + 3.0 * m_g3; // Offsets for last corner in (x,y,z) coords
	double y3 = y0 - 1.0 + 3.0 * m_g3;
	double z3 = z0 - 1.0 + 3.0 * m_g3;
	// Work out the hashed gradient indices of the four simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;

	int idx1 = jj + m_perm[kk];
	int idx2 = ii + m_perm[idx1];
	int gi0 = m_permMod12[idx2];

	int idx3 = jj + j1 + m_perm[kk + k1];
	int idx4 = ii + i1 + m_perm[idx3];
	int gi1 = m_permMod12[idx4];

	int idx5 = jj + j2 + m_perm[kk + k2];
	int idx6 = ii + i2 + m_perm[idx5];
	int gi2 = m_permMod12[idx6];

	int idx7 = jj + 1 + m_perm[kk + 1];
	int idx8 = ii + 1 + m_perm[idx7];
	int gi3 = m_permMod12[idx8];
	// Calculate the contribution from the four corners
	double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0; // change to 0.5 if you want

	if (t0 < 0) n0 = 0.0;
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(m_grad3[gi0], x0, y0, z0);
	}
	double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1; // change to 0.5 if you want
	if (t1 < 0) n1 = 0.0;
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * dot(m_grad3[gi1], x1, y1, z1);
	}
	double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2; // change to 0.5 if you want
	if (t2 < 0) n2 = 0.0;
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(m_grad3[gi2], x2, y2, z2);
	}
	double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3; // change to 0.5 if you want
	if (t3 < 0) n3 = 0.0;
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * dot(m_grad3[gi3], x3, y3, z3);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1] (now [0, 1])
	return (float)(32.0 * (n0 + n1 + n2 + n3) + 1) * 0.5f; // change to 76.0 if you want
}

int SpacelNoise::fastFloor(double _x) const
{
	int xi = (int)_x;
	return _x < xi ? xi - 1 : xi;
}

double SpacelNoise::dot(Grad const& _g, double _x, double _y, double _z) const
{
	return _g.m_x * _x + _g.m_y * _y + _g.m_z * _z;
}

float SpacelNoise::getOctaveNoise(double _x, double _y, double _z, int _octaves) const
{
	float value = 0;
	float divisor = 0;
	float currentHalf = 0;
	float currentDouble = 0;

	for (int i = 0; i < _octaves; i++)
	{
		currentHalf = (float)FMath::Pow(0.5f, i);
		currentDouble = (float)FMath::Pow(2, i);
		value += getNoise(_x * currentDouble, _y * currentDouble, _z) * currentHalf;
		divisor += currentHalf;
	}

	return value / divisor;
}