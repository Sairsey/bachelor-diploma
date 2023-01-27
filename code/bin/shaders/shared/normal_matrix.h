#pragma once
#ifndef __cplusplus

float4x4 GetNormalMatrix(float4x4 m)
{
	// remove translate part
	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;

	m[3][3] = 1;

	// get scale part squared
	float3 sxvec = float3(m[0][0], m[1][0], m[2][0]);
	float3 syvec = float3(m[0][1], m[1][1], m[2][1]);
	float3 szvec = float3(m[0][2], m[1][2], m[2][2]);

	float sx2 = dot(sxvec, sxvec);
	float sy2 = dot(syvec, syvec);
	float sz2 = dot(szvec, szvec);

	m[0][0] /= sx2;
	m[1][0] /= sx2;
	m[2][0] /= sx2;

	m[0][1] /= sy2;
	m[1][1] /= sy2;
	m[2][1] /= sy2;

	m[0][2] /= sz2;
	m[1][2] /= sz2;
	m[2][2] /= sz2;

	return m;
}

#endif // __cplusplus