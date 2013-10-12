typedef struct Ray
{
	float4 position;
	float4 direction;
	float3 diffuseReflectivity;
	float3 surfaceNormal;
	float distance;
} Ray;

typedef struct Light
{
	float4 position;
	float3 intensity;
} Light;

__constant Light l = {
	{0.f, 0.f, 50.f, 1.f},
	{0.7f, 0.7f, 0.7f}
};

__constant float3 ambientIntensity = {0.3f, 0.3f, 0.3f};

__kernel void testImage(__write_only image2d_t image, __global Ray* _rays)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	Ray r = _rays[pos.x + get_image_width(image) * pos.y];
	
	if (r.distance == INFINITY)
	{
		return;
	}

	float3 intersectPoint = r.position.xyz + r.direction.xyz * r.distance;
	float3 lightDir = normalize(l.position.xyz - intersectPoint);

	float3 surfaceReflectivity = r.diffuseReflectivity;
	float3 color = surfaceReflectivity * (l.intensity * max(dot(lightDir, r.surfaceNormal), 0.f) + ambientIntensity);
	write_imagef(image, pos, (float4)(color, 1.f));
}
