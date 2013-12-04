#include "Types.hcl"

__kernel void transformVertices(__global Vertex* _vertIn, __global Vertex* _vertOut, const mat4 _transform, const mat4 _invTransform, int _numVert)
{
	int id = get_global_id(0);
	if (id >= _numVert)
		return;

	Vertex v = _vertIn[id];

	v.position = matmul(&_transform, &v.position);
	v.normal = matmul(&_invTransform, &v.normal);

	_vertOut[id] = v;
}