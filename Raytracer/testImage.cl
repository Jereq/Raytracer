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

__constant Light l = {
	{0.f, 0.f, 50.f, 1.f},
	{0.7f, 0.7f, 0.7f, 0.f}
};

__constant float4 ambientIntensity = {0.3f, 0.3f, 0.3f, 0.f};

__kernel void testImage(__write_only image2d_t image, __global Ray* _rays)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	Ray r = _rays[pos.x + get_image_width(image) * pos.y];
	
	if (r.distance == INFINITY)
	{
		return;
	}

	float4 color = { 0.f, 0.f, 0.f, 1.f };

	if(r.inShadow == false)
	{
		float4 intersectPoint = r.position + r.direction * r.distance;
		float4 lightDir = normalize(l.position - intersectPoint);

		float4 surfaceReflectivity = r.diffuseReflectivity;
		color = surfaceReflectivity * (l.intensity * max(dot(lightDir, r.surfaceNormal), 0.f) + ambientIntensity);	
	}
	
	write_imagef(image, pos, (float4)(color.xyz, 1.f));
}
