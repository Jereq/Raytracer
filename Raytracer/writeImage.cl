#include "Types.hcl"

__kernel void accumulateImage(__global float4* _accumulationBuffer, __global Ray* _rays, int _numRays, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;

	if (_rays[id].distance == INFINITY)
	{
		_rays[id].direction = _rays[id].reflectDir;
		return;
	}

	if (_rays[id].inShadow)
	{
		_rays[id].direction = _rays[id].reflectDir;
		_rays[id].distance = INFINITY;
		return;
	}

	float4 position = _rays[id].position;
	float4 surfaceReflectivity = _rays[id].diffuseReflectivity;
	float4 normal = _rays[id].surfaceNormal;
	float4 reflectDir = _rays[id].reflectDir;
	float shininess = _rays[id].shininess;
	float strength = _rays[id].strength;

	float4 intersectPoint = position;
	float4 relativePos = _lights[_lightIdx].position - intersectPoint;
	float4 lightDir = normalize(relativePos);
	float distanceSq = dot(relativePos, relativePos);

	float NdotL = dot(normal, lightDir);
	float intensity = clamp(NdotL, 0.f, 1.f);

	float4 diffuseLight = intensity * _lights[_lightIdx].intensity / distanceSq;

	float4 halfway = normalize(lightDir - (reflectDir - 2 * dot(reflectDir, normal) * normal));

	float NdotH = dot(normal, halfway);
	intensity = pow(clamp(NdotH, 0.f, 1.f), shininess);

	float4 specularLight = intensity * _lights[_lightIdx].intensity / distanceSq;

	float4 color = surfaceReflectivity * (diffuseLight + specularLight);
	
	_accumulationBuffer[id] += strength * (float4)(color.xyz, 0.f);

	_rays[id].direction = reflectDir;
	_rays[id].distance = INFINITY;
}

__kernel void dumpImage(__global float4* _accumulationBuffer, __global Ray* _rays, __write_only image2d_t _image)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	if (pos.x >= get_image_width(_image) || pos.y >= get_image_height(_image))
		return;

	write_imagef(_image, pos, clamp(_accumulationBuffer[pos.x + get_image_width(_image) * pos.y], 0.f, 1.f));
}
