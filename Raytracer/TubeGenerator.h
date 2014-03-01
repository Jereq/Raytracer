#pragma once

#include "Bone.h"

#include <glm/glm.hpp>

#include <iosfwd>
#include <string>
#include <vector>

class TubeGenerator
{
private:
	struct Vertex
	{
		int pos;
		int texCoord;
		int normal;
		int bone;
	};

	struct Triangle
	{
		Vertex v[3];
	};

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::vec3> normals;
	std::vector<Bone> bones;
	std::vector<glm::vec3> bonePositions;
	std::vector<Triangle> faces;



public:
	void generate();
	void outputMesh(std::ostream& _stream);
	void outputSkeleton(std::ostream& _stream);
	void outputBendAnimation(std::ostream& _stream);

private:
	void createQuad(glm::vec3 _pos, glm::vec3 _right, glm::vec3 _up,
				glm::vec2 _texPos, glm::vec2 _texRight, glm::vec2 _texUp);
	void createSegment(glm::vec3 _pos, glm::vec3 _forward, glm::vec3 _width,
				glm::vec2 _texPos, glm::vec2 _texRight, glm::vec2 _texUp);
	void createBone(glm::vec3 _pos);

	void bindVertexToClosestBone(Vertex& _vert);
	void bindVerticesToClosestBone();

	friend std::ostream& operator<<(std::ostream& _stream, const Vertex& _vert);
	friend std::ostream& operator<<(std::ostream& _stream, const Triangle& _face);
};
