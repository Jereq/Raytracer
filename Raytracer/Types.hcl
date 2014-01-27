#ifndef _TYPES_HCL_
#define _TYPES_HCL_

typedef struct mat4
{
	union
	{
		float4 rows[4];
	};
} mat4;

float4 matmul(const mat4* _mat, const float4* _vec)
{
	float4 res = {
		dot(_mat->rows[0], *_vec),
		dot(_mat->rows[1], *_vec),
		dot(_mat->rows[2], *_vec),
		dot(_mat->rows[3], *_vec)
	};

	return res;
}

typedef struct Ray
{
	float4 position;
	float4 direction;
	float4 diffuseReflectivity;
	float4 surfaceNormal;
	float4 reflectDir;
	float distance;
	float shininess;
	float strength;
	float totalStrength;
	int inShadow;
	int collideGroup;
	int collideObject;
} Ray;

typedef struct Sphere
{
	float4 position;
	float4 diffuseReflectivity;
	float radius;
	float reflectFraction;
} Sphere;

typedef struct Vertex
{
	float4 position;
	float4 textureCoord;
	float4 normal;
	float4 tangent;
	float4 bitangent;
} Vertex;

typedef struct Triangle
{
	Vertex v[3];
} Triangle;

typedef struct Light
{
	float4 position;
	float4 intensity;
} Light;

#endif