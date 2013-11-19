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
	int inShadow;
	int collideGroup;
	int collideObject;
} Ray;

typedef struct Sphere
{
	float4 position;
	float4 diffuseReflectivity;
	float radius;
} Sphere;

typedef struct Vertex
{
	float4 position;
	float4 textureCoord;
	float4 normal;
	float4 tangent;
	float4 biTangent;
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

__constant Light l = {
	{0.f, 0.f, 50.f, 1.f},
	{0.7f, 0.7f, 0.7f, 0.f}
};

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
	_res[pos.x + _width * pos.y].inShadow = false;
	_res[pos.x + _width * pos.y].collideGroup = -1;
	_res[pos.x + _width * pos.y].collideGroup = -1;
}

// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-sphere-intersection/
// Real-Time Rendering, pg. 741
bool findSphereIntersectDistance(Ray* _ray, __constant Sphere* _sphere, float* t)
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

	*t = rayDist + ((rDist2 > radius2) ? -rayDistInSphere : rayDistInSphere);

	if (*t > _ray->distance)
	{
		return false;
	}

	return true;
}

bool sphereIntersect(Ray* _ray, __constant Sphere* _sphere)
{
	float t = 0.f;
	if (!findSphereIntersectDistance(_ray, _sphere, &t))
	{
		return false;
	}
	
	_ray->distance = t;
	_ray->diffuseReflectivity = _sphere->diffuseReflectivity;

	float4 intersectPoint = _ray->position + _ray->direction * t;
	_ray->surfaceNormal = normalize(intersectPoint - _sphere->position);

	return true;
}

__kernel void findClosestSpheres(__global Ray* _rays, int numRays, __constant Sphere* _spheres, int _numSpheres)
{
	int id = get_global_id(0);
	if (id > numRays)
		return;

	Ray r = _rays[id];

	for (unsigned int i = 0; i < _numSpheres; i++)
	{
		if (sphereIntersect(&r, &_spheres[i]))
		{
			r.collideGroup = 0;
			r.collideObject = i;
		}

	}

	_rays[id] = r;
}

__kernel void detectShadowWithSpheres(__global Ray* _rays, int numRays, __constant Sphere* _spheres, int _numSpheres, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id > numRays)
		return;

	Ray r = _rays[id];

	r.position += r.direction * r.distance;
	float4 relativePos = _lights[_lightIdx].position - r.position;
	r.distance = length(relativePos);
	r.direction = relativePos / r.distance;

	float dummy;
	for (unsigned int i = 0; r.inShadow == falsedww && i < _numSpheres; i++)
	{
		if (r.collideGroup == 0 && r.collideObject == i)
			continue;

		r.inShadow = findSphereIntersectDistance(&r, &_spheres[i], &dummy);
	}

	_rays[id] = r;
}

// Real-Time Rendering, pg. 750
bool triangleIntersect(Ray* _ray, __global Triangle* _triangle)
{
	float4 e1 = _triangle->v[1].position - _triangle->v[0].position;
	float4 e2 = _triangle->v[2].position - _triangle->v[0].position;
	
	float4 q = cross(_ray->direction, e2);
	float a = dot(e1, q);
	if(fabs(a) < 0.00001f)
		return false;

	float f = 1.f / a;

	float4 s = _ray->position - _triangle->v[0].position;
	float u = f * dot(s, q);
	if(u < 0.f)
		return false;

	float4 r = cross(s, e1);
	float v = f * dot(_ray->direction, r);
	if(v < 0.f || u + v > 1.f)
		return false;

	float t = f * dot(e2, r);

	if (t > _ray->distance || t <= 0.f)
	{
		return false;
	}

	_ray->distance = t;
	_ray->diffuseReflectivity = (1.f - u - v) * _triangle->v[0].textureCoord + u * _triangle->v[1].textureCoord + v * _triangle->v[2].textureCoord;
	_ray->diffuseReflectivity.w = 1.f;

	float4 intersectPoint = _ray->position + _ray->direction * t;
	_ray->surfaceNormal = (1.f - u - v) * _triangle->v[0].normal + u * _triangle->v[1].normal + v * _triangle->v[2].normal;

	return true;
}

__kernel void findClosestTriangles(__global Ray* _rays, int numRays, __global Triangle* _triangles, int _numTriangles)
{
	int id = get_global_id(0);
	if (id > numRays)
		return;

	Ray r = _rays[id];

	for (unsigned int i = 0; i < _numTriangles; i++)
	{
		triangleIntersect(&r, &_triangles[i]);
	}

	_rays[id] = r;
}
