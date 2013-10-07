typedef struct mat4
{
	union
	{
		float4 rows[4];
	};
} mat4;

typedef struct ray
{
	float4 position;
	float4 direction;
} ray;

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

__kernel void primaryRays(__global ray* _res, const mat4 _invMat, const float4 _camPos, const int _width, const int _height)
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
}
