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

typedef struct Light
{
	float4 position;
	float4 intensity;
} Light;

__constant float4 ambientIntensity = {0.3f, 0.3f, 0.3f, 0.f};

__kernel void writeImage(__global float4* _accumulationBuffer, __global Ray* _rays, int _numRays, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id > _numRays)
		return;
	Ray r = _rays[id];
	
	if (r.distance == INFINITY)
	{
		_accumulationBuffer[id] = (float4)(1.f, 0.f, 0.f, 1.f);
		return;
	}

	float4 surfaceReflectivity = r.diffuseReflectivity;
	float4 color = surfaceReflectivity * ambientIntensity;

	if(r.inShadow == false)
	{
		float4 intersectPoint = r.position;
		float4 lightDir = normalize(_lights[_lightIdx].position - intersectPoint);

		color += surfaceReflectivity * (_lights[_lightIdx].intensity * max(dot(lightDir, r.surfaceNormal), 0.f));	
	}
	
	_accumulationBuffer[id] = (float4)(color.xyz, 1.f);

	r.direction = r.reflectDir;
	r.distance = INFINITY;
}

__kernel void accumulateImage(__global float4* _accumulationBuffer, __global Ray* _rays, int _numRays, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id > _numRays)
		return;
	Ray r = _rays[id];
	
	if (r.distance == INFINITY)
	{
		_accumulationBuffer[id] += (float4)(0.f, 1.f, 0.f, 0.f);
		return;
	}

	float4 surfaceReflectivity = r.diffuseReflectivity;
	float4 color = {0.f, 0.f, 0.f, 0.f};

	if(r.inShadow == false)
	{
		float4 intersectPoint = r.position;
		float4 lightDir = normalize(_lights[_lightIdx].position - intersectPoint);

		color += surfaceReflectivity * (_lights[_lightIdx].intensity * max(dot(lightDir, r.surfaceNormal), 0.f));	
	}
	
	_accumulationBuffer[id] += (float4)(color.xyz, 0.f);

	r.direction = r.reflectDir;
	r.distance = INFINITY;
}

__kernel void dumpImage(__global float4* _accumulationBuffer, __global Ray* _rays, __write_only image2d_t _image)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	if (pos.x > get_image_width(_image) || pos.y > get_image_height(_image))
		return;

	write_imagef(_image, pos, _accumulationBuffer[pos.x + get_image_width(_image) * pos.y]);
}
