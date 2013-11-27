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
	float4 reflectDir;
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
	_res[id].inShadow = false;
	_res[id].collideGroup = -1;
	_res[id].collideGroup = -1;

	_accumulationBuffer[id] = (float4)(0.f, 0.f, 0.f, 0.f);
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

	Ray r = _rays[id];

	float dummy;
	for (unsigned int i = 0; r.inShadow == false && i < _numSpheres; i++)
	{
		if (r.collideGroup == 0 && r.collideObject == i)
			continue;

		r.inShadow = findSphereIntersectDistance(&r, &_spheres[i], &dummy);
		if (r.distance - dummy < 0.11f)
			r.inShadow = false;
	}

	_rays[id] = r;
}

__kernel void moveRaysToIntersection(__global Ray* _rays, int _numRays)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;

	Ray r = _rays[id];

	r.position += r.direction * r.distance;
	r.reflectDir = r.direction - 2 * dot(r.direction, r.surfaceNormal) * r.surfaceNormal;

	_rays[id] = r;
}

__kernel void updateRaysToLight(__global Ray* _rays, int _numRays, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;

	Ray r = _rays[id];

	float4 relativeLightPos = _lights[_lightIdx].position - r.position;
	r.distance = length(relativeLightPos);
	r.direction = relativeLightPos / r.distance;
	r.inShadow = false;

	_rays[id] = r;
}

// Real-Time Rendering, pg. 750
bool findTriangleIntersectDistance(Ray* _ray, __global Triangle* _triangle, float* t, float* u, float* v)
{
	float4 e1 = _triangle->v[1].position - _triangle->v[0].position;
	float4 e2 = _triangle->v[2].position - _triangle->v[0].position;
	
	float4 q = cross(_ray->direction, e2);
	float a = dot(e1, q);
	if(fabs(a) < 0.00001f)
		return false;

	float f = 1.f / a;

	float4 s = _ray->position - _triangle->v[0].position;
	*u = f * dot(s, q);
	if(*u < 0.f)
		return false;

	float4 r = cross(s, e1);
	*v = f * dot(_ray->direction, r);
	if(*v < 0.f || *u + *v > 1.f)
		return false;

	*t = f * dot(e2, r);

	if (*t > _ray->distance || *t <= 0.f)
	{
		return false;
	}
	
	return true;
}

bool triangleIntersect(Ray* _ray, __global Triangle* _triangle)
{
	float t = 0.f;
	float u = 0.f;
	float v = 0.f;
	if (!findTriangleIntersectDistance(_ray, _triangle, &t, &u, &v))
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
	if (id >= numRays)
		return;

	Ray r = _rays[id];

	for (unsigned int i = 0; i < _numTriangles; i++)
	{
		if (r.collideGroup == 1 && r.collideObject == i)
			continue;

		if(triangleIntersect(&r, &_triangles[i]))
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

	Ray r = _rays[id];

	float dummy;
	float dummier;
	float dummiest;
	for (unsigned int i = 0; r.inShadow == false && i < _numTriangles; i++)
	{
		if (r.collideGroup == 1 && r.collideObject == i)
			continue;

		r.inShadow = findTriangleIntersectDistance(&r, &_triangles[i], &dummy, &dummier, &dummiest);
	}

	_rays[id] = r;
}
