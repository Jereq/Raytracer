typedef struct Ray
{
	float4 position;
	float4 direction;
} Ray;

typedef struct Sphere
{
	float4 position;
	float radius;
} Sphere;

__kernel void testImage(__write_only image2d_t image, __global Ray* _rays)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	Ray r = _rays[pos.x + get_image_width(image) * pos.y];

	Sphere sphere = {{0.f, 0.f, -5.f, 1.f}, 1.f};

	float4 rDistance = sphere.position - r.position;
	float rayDist = dot(r.direction, rDistance);
	if (rayDist < -sphere.radius)
	{
		return;
	}

	float4 sDistance = rDistance - rayDist * r.direction;
	float centerDistance2 = dot(sDistance, sDistance);

	if (centerDistance2 > sphere.radius * sphere.radius)
	{
		return;
	}

	float k2 = sqrt(sphere.radius * sphere.radius - centerDistance2);
	float4 n = normalize(-sDistance - k2 * r.direction);

	float4 color = (float4)(n.x, n.y, n.z, 1.f);
	write_imagef(image, pos, color);
}
