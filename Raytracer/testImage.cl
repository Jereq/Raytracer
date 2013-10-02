__kernel void testImage(__write_only image2d_t image)
{
	int2 pos = {get_global_id(0), get_global_id(1)};
	float4 color = {pos.x / (float)get_image_width(image), pos.y / (float)get_image_height(image), 0.5f, 1.f};
	
	write_imagef(image, pos, color);
}
