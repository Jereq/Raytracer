#pragma once

#include "Bone.h"
#include "ModelData.h"

#include <glm/glm.hpp>
#include "CL/cl.hpp"

#include <cstdint>
#include <vector>

class AnimatedObjModel
{
public:
	//this typedef must match the layout in the Shader Class
	struct VertexType
	{
		glm::vec4 position;
		glm::vec4 texCoord;
		glm::vec4 normal;
		glm::vec4 tangent;
		glm::vec4 bitangent;
		uint32_t bone;
		uint32_t padding[3];
	};
private:
	struct IVertex
	{
		int posIndex;
		int texIndex;
		int normalIndex;
		int bone;
	};
	struct FaceType
	{
		IVertex v[3];
	};
	struct TempVertexType
	{
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};
	struct DataCounts
	{
		unsigned int positions;
		unsigned int texCoords;
		unsigned int normals;
		unsigned int bones;
		unsigned int faces;
		unsigned int groups;
	};

	cl::Context context;
	std::vector<VertexType> mModel;
	std::vector<Bone> bones;

public:
	AnimatedObjModel(cl::Context _context);

	ModelData::ptr loadFromFile(const std::string& _path);

private:
	cl::Buffer initializeVertexBuffer();

	void loadFile(std::ifstream& _dataStream);
	DataCounts readFileCounts(std::ifstream& _dataStream);
	void loadDataStructures(std::ifstream& _dataStream, const DataCounts& counts);

	void calculateModelVectors();
	void calculateTangentBinormal(TempVertexType vertex1,
		TempVertexType vertex2, TempVertexType vertex3, glm::vec3& tangent,
		glm::vec3& binormal);

	friend std::istream& operator>>(std::istream& _stream, IVertex& _vert);
};