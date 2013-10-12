typedef struct mat4
{
	union
	{
		float4 rows[4];
	};
} mat4;

typedef struct Ray
{
	float4 position;
	float4 direction;
	float3 diffuseReflectivity;
	float3 surfaceNormal;
	float distance;
} Ray;

typedef struct Sphere
{
	float4 position;
	float3 diffuseReflectivity;
	float radius;
} Sphere;


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

__kernel void primaryRays(__global Ray* _res, const mat4 _invMat, const float4 _camPos, const int _width, const int _height)
{
	int2 pos = {get_global_id(0), get_global_id(1)};

	if (pos.x > _width || pos.y > _height)
	{
		return;
	}

	float4 fpos = {((float)pos.x + 0.5f) * 2.f / (float)_width - 1.f, ((float)pos.y + 0.5f) * 2.f / (float)_height - 1.f, -1.f, 1.f};
	float4 worldPos = matmul(&_invMat, &fpos);
	worldPos *= (1.f / worldPos.w);
	_res[pos.x + _width * pos.y].position = _camPos;
	_res[pos.x + _width * pos.y].direction = normalize(worldPos - _camPos);
	_res[pos.x + _width * pos.y].distance = INFINITY;
}

// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-sphere-intersection/
bool sphereIntersect(Ray* _ray, __constant Sphere* _sphere)
{
	float3 rDistance = _sphere->position.xyz - _ray->position.xyz;
	float rayDist = dot(rDistance, _ray->direction.xyz);

	if (rayDist < 0.f)
	{
		return false;
	}

	float centerDistance2 = dot(rDistance, rDistance) - rayDist * rayDist;
	if (centerDistance2 > _sphere->radius * _sphere->radius)
	{
		return false;
	}

	float rayDistInSphere = sqrt(_sphere->radius * _sphere->radius - centerDistance2);
	float t0 = rayDist - rayDistInSphere;
	float t1 = rayDist + rayDistInSphere;

	if (t0 > _ray->distance)
	{
		return false;
	}
	
	_ray->distance = t0;
	_ray->diffuseReflectivity = _sphere->diffuseReflectivity;

	float3 intersectPoint = _ray->position.xyz + _ray->direction.xyz * t0;
	_ray->surfaceNormal = normalize(intersectPoint - _sphere->position.xyz);

	return true;
}

__kernel void intersectSpheres(__global Ray* _rays, int numRays, __constant Sphere* _spheres, int _numSpheres)
{
	int id = get_global_id(0);
	if (id > numRays)
		return;

	Ray r = _rays[id];

	for (unsigned int i = 0; i < _numSpheres; i++)
	{
		sphereIntersect(&r, &_spheres[i]);
	}

	_rays[id] = r;
}
