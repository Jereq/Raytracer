#version 400

in vec2 vertexPosition;
out vec2 texCoord;

void main()
{
	texCoord = vertexPosition * 0.5f + 0.5f;
	gl_Position = vec4(vertexPosition, 0.f, 1.f);
}
