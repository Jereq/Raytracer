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

typedef struct Light
{
	float4 position;
	float4 intensity;
} Light;

__constant float4 ambientIntensity = {0.3f, 0.3f, 0.3f, 0.f};

__kernel void testImage(__write_only image2d_t image, __global Ray* _rays, __constant Light* _lights, int _lightIdx)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	Ray r = _rays[pos.x + get_image_width(image) * pos.y];
	
	if (r.distance == INFINITY)
	{
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
	
	write_imagef(image, pos, (float4)(color.xyz, 1.f));
}
