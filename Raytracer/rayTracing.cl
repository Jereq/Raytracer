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
	float4 diffuseReflectivity;
	float4 surfaceNormal;
	float distance;
} Ray;

typedef struct Sphere
{
	float4 position;
	float4 diffuseReflectivity;
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
// Real-Time Rendering, pg. 741
bool sphereIntersect(Ray* _ray, __constant Sphere* _sphere)
{
	float4 rDistance = _sphere->position - _ray->position;
	float rayDist = dot(rDistance, _ray->direction);
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

	float t = rayDist + ((rDist2 > radius2) ? -rayDistInSphere : rayDistInSphere);

	if (t > _ray->distance)
	{
		return false;
	}
	
	_ray->distance = t;
	_ray->diffuseReflectivity = _sphere->diffuseReflectivity;

	float4 intersectPoint = _ray->position + _ray->direction * t;
	_ray->surfaceNormal = normalize(intersectPoint - _sphere->position);

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
