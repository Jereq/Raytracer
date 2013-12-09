#include "Types.hcl"

__constant Light l = {
	{0.f, 0.f, 50.f, 1.f},
	{0.7f, 0.7f, 0.7f, 0.f}
};

__kernel void primaryRays(__global Ray* _res, const mat4 _invMat, const float4 _camPos, const int _width, const int _height, __global float4* _accumulationBuffer)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	int id = pos.x + _width * pos.y;

	if (pos.x >= _width || pos.y >= _height)
	{
		return;
	}

	float4 fpos = {((float)pos.x + 0.5f) * 2.f / (float)_width - 1.f, ((float)pos.y + 0.5f) * 2.f / (float)_height - 1.f, -1.f, 1.f};
	float4 worldPos = matmul(&_invMat, &fpos);
	worldPos *= (1.f / worldPos.w);
	float4 direction = normalize(worldPos - _camPos);
	_res[id].position = _camPos;
	_res[id].direction = direction;
	_res[id].diffuseReflectivity = (float4)(0.f, 0.f, 0.f, 1.f);
	_res[id].surfaceNormal = (float4)(0.f, 0.f, 0.f, 0.f);
	_res[id].reflectDir = direction;
	_res[id].distance = INFINITY;
	_res[id].shininess = 0.f;
	_res[id].strength = 0.f;
	_res[id].totalStrength = 1.f;
	_res[id].inShadow = false;
	_res[id].collideGroup = -1;
	_res[id].collideGroup = -1;

	_accumulationBuffer[id] = (float4)(0.f, 0.f, 0.f, 0.f);
}

// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-sphere-intersection/
// Real-Time Rendering, pg. 741
bool findSphereIntersectDistance(float4 _position, float4 _direction, float _distance, __constant Sphere* _sphere, float* t)
{
	float4 rDistance = _sphere->position - _position;
	float rayDist = dot(rDistance, _direction);

	float rDist2 = dot(rDistance, rDistance);
	float radius2 = _sphere->radius * _sphere->radius;

	if (rayDist < 0.f && rDist2 > radius2)
	{
		return false;
	}

	float centerDistance2 = rDist2 - rayDist * rayDist;
	if (centerDistance2 > radius2)
	{
		return false;
	}

	float rayDistInSphere = sqrt(radius2 - centerDistance2);

	*t = rayDist + ((rDist2 > radius2) ? -rayDistInSphere : rayDistInSphere);

	if (*t > _distance)
	{
		return false;
	}

	return true;
}

bool sphereIntersect(Ray* _ray, __constant Sphere* _sphere)
{
	float t = 0.f;
	if (!findSphereIntersectDistance(_ray->position, _ray->direction, _ray->distance, _sphere, &t))
	{
		return false;
	}
	
	_ray->distance = t;
	_ray->diffuseReflectivity = _sphere->diffuseReflectivity * (1.f - _sphere->reflectFraction);
	_ray->strength = _sphere->reflectFraction;
	_ray->shininess = _sphere->reflectFraction * 400.f;

	float4 intersectPoint = _ray->position + _ray->direction * t;
	_ray->surfaceNormal = normalize(intersectPoint - _sphere->position);

	return true;
}

__kernel void findClosestSpheres(__global Ray* _rays, int numRays, __constant Sphere* _spheres, int _numSpheres)
{
	int id = get_global_id(0);
	if (id >= numRays)
		return;

	Ray r = _rays[id];

	for (unsigned int i = 0; i < _numSpheres; i++)
	{
		if (r.collideGroup == 0 && r.collideObject == i)
			continue;

		if (sphereIntersect(&r, &_spheres[i]))
		{
			r.collideGroup = 0;
			r.collideObject = i;
		}

	}

	_rays[id] = r;
}

__kernel void detectShadowWithSpheres(__global Ray* _rays, int _numRays, __constant Sphere* _spheres, int _numSpheres)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;

	float4 position = _rays[id].position;
	float4 direction = _rays[id].direction;
	float distance = _rays[id].distance;
	int inShadow = _rays[id].inShadow;
	int collideGroup = _rays[id].collideGroup;
	int collideObject = _rays[id].collideObject;

	float dummy;
	for (unsigned int i = 0; inShadow == false && i < _numSpheres; i++)
	{
		if (collideGroup == 0 && collideObject == i)
			continue;

		inShadow = findSphereIntersectDistance(position, direction, distance, &_spheres[i], &dummy);
		if (distance - dummy < 0.11f)
			inShadow = false;
	}

	_rays[id].inShadow = inShadow;
}

__kernel void moveRaysToIntersection(__global Ray* _rays, int _numRays)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;

	Ray r = _rays[id];

	r.position += r.direction * r.distance + r.surfaceNormal * 0.001f;
	r.reflectDir = r.direction - 2 * dot(r.direction, r.surfaceNormal) * r.surfaceNormal;

	float currentStrength = r.totalStrength;
	r.totalStrength *= r.strength;
	r.strength = currentStrength;

	_rays[id] = r;
}

__kernel void updateRaysToLight(__global Ray* _rays, int _numRays, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;

	float4 position = _rays[id].position;
	float distance = _rays[id].distance;

	float4 relativeLightPos = _lights[_lightIdx].position - position;
	float newDistance = length(relativeLightPos);
	float4 direction = relativeLightPos / newDistance;

	_rays[id].distance = newDistance;
	_rays[id].direction = direction;
	_rays[id].inShadow = false;
}

// Real-Time Rendering, pg. 750
bool findTriangleIntersectDistance(float4 _position, float4 _direction, float _distance, __global Triangle* _triangle, float* t, float* u, float* v)
{
	float4 e1 = _triangle->v[1].position - _triangle->v[0].position;
	float4 e2 = _triangle->v[2].position - _triangle->v[0].position;
	
	float4 q = cross(_direction, e2);
	float a = dot(e1, q);
	if(fabs(a) < 0.00001f)
		return false;

	float f = 1.f / a;

	float4 s = _position - _triangle->v[0].position;
	*u = f * dot(s, q);
	if(*u < 0.f)
		return false;

	float4 r = cross(s, e1);
	*v = f * dot(_direction, r);
	if(*v < 0.f || *u + *v > 1.f)
		return false;

	*t = f * dot(e2, r);

	if (*t > _distance || *t <= 0.f)
	{
		return false;
	}
	
	return true;
}

bool triangleIntersect(Ray* _ray, __global Triangle* _triangle, float _reflectFraction, image2d_t _diffuseTex)
{
	float t = 0.f;
	float u = 0.f;
	float v = 0.f;
	if (!findTriangleIntersectDistance(_ray->position, _ray->direction, _ray->distance, _triangle, &t, &u, &v))
	{
		return false;
	}
	
	float2 texCoord = ((1.f - u - v) * _triangle->v[0].textureCoord.xy + u * _triangle->v[1].textureCoord.xy + v * _triangle->v[2].textureCoord.xy);

	const sampler_t diffSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

	_ray->distance = t;
	_ray->diffuseReflectivity = (1.f - _reflectFraction) * read_imagef(_diffuseTex, diffSampler, texCoord);
	_ray->diffuseReflectivity.w = 1.f;
	_ray->strength = _reflectFraction;
	_ray->shininess = _reflectFraction * 400.f;

	float4 intersectPoint = _ray->position + _ray->direction * t;
	_ray->surfaceNormal = (1.f - u - v) * _triangle->v[0].normal + u * _triangle->v[1].normal + v * _triangle->v[2].normal;

	return true;
}

__kernel void findClosestTriangles(__global Ray* _rays, int numRays, __global Triangle* _triangles, int _numTriangles, float _reflectFraction, image2d_t _diffuseTex)
{
	int id = get_global_id(0);
	if (id >= numRays)
		return;

	Ray r = _rays[id];

	for (unsigned int i = 0; i < _numTriangles; i++)
	{
		if (r.collideGroup == 1 && r.collideObject == i)
			continue;

		if(triangleIntersect(&r, &_triangles[i], _reflectFraction, _diffuseTex))
		{
			r.collideGroup = 1;
			r.collideObject = i;
		}
		
	}

	_rays[id] = r;
}

__kernel void detectShadowWithTriangles(__global Ray* _rays, int numRays, __global Triangle* _triangles, int _numTriangles)
{
	int id = get_global_id(0);
	if (id >= numRays)
		return;

	int inShadow = _rays[id].inShadow;

	if (inShadow)
	{
		return;
	}

	float4 position = _rays[id].position;
	float4 direction = _rays[id].direction;
	float distance = _rays[id].distance;
	int collideGroup = _rays[id].collideGroup;
	int collideObject = _rays[id].collideObject;

	float dummy;
	float dummier;
	float dummiest;
	for (unsigned int i = 0; inShadow == false && i < _numTriangles; i++)
	{
		if (collideGroup == 1 && collideObject == i)
			continue;

		inShadow = findTriangleIntersectDistance(position, direction, distance, &_triangles[i], &dummy, &dummier, &dummiest);
	}

	_rays[id].inShadow = inShadow;
}
