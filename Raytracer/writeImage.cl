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

typedef struct Light
{
	float4 position;
	float4 intensity;
} Light;

__kernel void accumulateImage(__global float4* _accumulationBuffer, __global Ray* _rays, int _numRays, __constant Light* _lights, int _lightIdx)
{
	int id = get_global_id(0);
	if (id >= _numRays)
		return;
	Ray r = _rays[id];
	
	if (r.distance == INFINITY)
	{
		_accumulationBuffer[id] += r.strength * (float4)(0.f, 0.2f, 0.f, 0.f);
	}
	else
	{
		float4 surfaceReflectivity = r.diffuseReflectivity;
		float4 color = {0.f, 0.f, 0.f, 0.f};

		if(r.inShadow == false)
		{
			float4 intersectPoint = r.position;
			float4 relativePos = _lights[_lightIdx].position - intersectPoint;
			float4 lightDir = normalize(relativePos);
			float distanceSq = dot(relativePos, relativePos);

			float NdotL = dot(r.surfaceNormal, lightDir);
			float intensity = clamp(NdotL, 0.f, 1.f);

			float4 diffuseLight = intensity * _lights[_lightIdx].intensity / distanceSq;

			float4 halfway = normalize(lightDir - (r.reflectDir - 2 * dot(r.reflectDir, r.surfaceNormal) * r.surfaceNormal));

			float NdotH = dot(r.surfaceNormal, halfway);
			intensity = pow(clamp(NdotH, 0.f, 1.f), r.shininess);

			float4 specularLight = intensity * _lights[_lightIdx].intensity / distanceSq;

			color += surfaceReflectivity * (diffuseLight + specularLight);
		}
	
		_accumulationBuffer[id] += r.strength * (float4)(color.xyz, 0.f);
	}

	r.direction = r.reflectDir;
	r.distance = INFINITY;

	_rays[id] = r;
}

__kernel void dumpImage(__global float4* _accumulationBuffer, __global Ray* _rays, __write_only image2d_t _image)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	if (pos.x >= get_image_width(_image) || pos.y >= get_image_height(_image))
		return;

	write_imagef(_image, pos, clamp(_accumulationBuffer[pos.x + get_image_width(_image) * pos.y], 0.f, 1.f));
}
