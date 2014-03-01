#include "AnimatedObjModel.h"
#include <fstream>
using std::ifstream;

AnimatedObjModel::AnimatedObjModel(cl::Context _context)
	: context(_context)
{
}

ModelData::ptr AnimatedObjModel::loadFromFile(const std::string& _path)
{
	std::ifstream dataStream(_path);
	if (dataStream.fail())
	{
		throw std::exception(("Could not open model file: " + _path).c_str());
	}

	loadFile(dataStream);
	calculateModelVectors();
	cl::Buffer vertexBuffer = initializeVertexBuffer();

	ModelData::ptr result(new ModelData(vertexBuffer, mModel.size()));
	result->setBindPose(bones);

	mModel.clear();
	bones.clear();

	return result;
}

cl::Buffer AnimatedObjModel::initializeVertexBuffer()
{
	return cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(VertexType) * mModel.size(), mModel.data());
}

void AnimatedObjModel::loadFile(std::ifstream& _dataStream)
{
	DataCounts counts = readFileCounts(_dataStream);
	_dataStream.clear(std::ios::goodbit);
	_dataStream.seekg(0, std::ios::beg);
	loadDataStructures(_dataStream, counts);
}

AnimatedObjModel::DataCounts AnimatedObjModel::readFileCounts(std::ifstream& _dataStream)
{	
	DataCounts counts;

	char tInput;
	counts.positions = 0;
	counts.texCoords = 0;
	counts.normals = 0;
	counts.faces = 0;
	counts.groups = 0;
	counts.bones = 0;

	_dataStream.get(tInput);
	while(!_dataStream.eof())
	{
		if(tInput == 'v')
		{
			_dataStream.get(tInput);
			if(tInput == ' ') 
			{
				counts.positions++;
			}

			if(tInput == 't')
			{
				counts.texCoords++;
			}

			if(tInput == 'n')
			{
				counts.normals++;
			}
		}

		if(tInput == 'g')
		{
			counts.groups++;
		}

		if(tInput == 'f')
		{
			counts.faces++;
		}

		if (tInput == 'b')
		{
			counts.bones++;
		}

		while(tInput != '\n')
		{
			_dataStream.get(tInput);
		}

		_dataStream.get(tInput);
	}

	return counts;
}

void AnimatedObjModel::loadDataStructures(std::ifstream& _dataStream, const DataCounts& counts)
{
	std::vector<glm::vec3> tVertices;
	std::vector<glm::vec2> tTexCoords;
	std::vector<glm::vec3> tNormals;
	std::vector<FaceType> tFaces;

	char tInput1;
	
	int tVertexIndex = 0;
	int tTexcoordIndex = 0;
	int tNormalIndex = 0;
	int tFaceIndex = 0;
	int tBoneIndex = 0;

	mModel.resize(counts.faces * 3);

	//Initialize the four data structures.
	tVertices.resize(counts.positions);
	tTexCoords.resize(counts.texCoords);
	tNormals.resize(counts.normals);
	tFaces.resize(counts.faces);
	bones.resize(counts.bones);

	while(_dataStream)
	{
		_dataStream.get(tInput1);

		if (tInput1 == '#')
		{
#undef max
			_dataStream.ignore(std::numeric_limits<int>::max(), '\n');
			continue;
		}

		if(tInput1 == 'v')
		{
			_dataStream.get(tInput1);

			//Read in the vertices.
			if(tInput1 == ' ') 
			{ 
				_dataStream >> tVertices[tVertexIndex].x;
				_dataStream >> tVertices[tVertexIndex].y;
				_dataStream >> tVertices[tVertexIndex].z;
				tVertexIndex++; 
			}

			//Read in the texture uv coordinates.
			if(tInput1 == 't') 
			{ 
				_dataStream >> tTexCoords[tTexcoordIndex].x;
				_dataStream >> tTexCoords[tTexcoordIndex].y;
				tTexcoordIndex++; 
			}

			//Read in the normals.
			if(tInput1 == 'n') 
			{ 
				_dataStream >> tNormals[tNormalIndex].x;
				_dataStream >> tNormals[tNormalIndex].y;
				_dataStream >> tNormals[tNormalIndex].z;
				tNormalIndex++; 
			}
		}

		//Read in the faces.
		if(tInput1 == 'f') 
		{
			_dataStream.get(tInput1);
			if(tInput1 == ' ')
			{
				FaceType& triangle = tFaces[tFaceIndex];
				_dataStream >> triangle.v[0] >>
					triangle.v[1] >>
					triangle.v[2];
				tFaceIndex++;
			}
		}

		if (tInput1 == 'b')
		{
			_dataStream >> bones[tBoneIndex];
			tBoneIndex++;
		}
	}

	Bone::setupSkeleton(bones);

	int i = 0;
	int j = 0;
	bool done = false;
	while(!done)
	{
		for (int face = 0; face < 3; ++face)
		{
			const IVertex& v = tFaces[i].v[face];

			mModel[j].position = glm::vec4(tVertices[v.posIndex - 1], 1.f);
			mModel[j].texCoord = glm::vec4(tTexCoords[v.texIndex - 1], 0.f, 0.f);
			mModel[j].normal = glm::vec4(tNormals[v.normalIndex - 1], 0.f);
			mModel[j].bone = v.bone - 1;
			j++;
		}

		i++;
		if((i == tFaceIndex) || (j == mModel.size()))
		{
			done = true;
		}
	}
}

void AnimatedObjModel::calculateModelVectors()
{
	int tFaceCount;
	int tIndex;
	TempVertexType tVertex[3];
	glm::vec3 tTangent;
	glm::vec3 tBinormal;

	tFaceCount = mModel.size() / 3;
	tIndex = 0;
	for(int i = 0; i < tFaceCount; i++)
	{
		for (int i = 0; i < 3; ++i)
		{
			tVertex[i].pos	= mModel[tIndex].position.swizzle(glm::X, glm::Y, glm::Z);
			tVertex[i].texCoord	= mModel[tIndex].texCoord.swizzle(glm::X, glm::Y);
			tVertex[i].normal	= mModel[tIndex].normal.swizzle(glm::X, glm::Y, glm::Z);
			tIndex++;
		}

		calculateTangentBinormal(tVertex[0], tVertex[1], tVertex[2], tTangent,
			tBinormal);

		for (int i = 0; i < 3; ++i)
		{
			mModel[tIndex - 3 + i].tangent = glm::vec4(tTangent, 0.f);
			mModel[tIndex - 3 + i].bitangent = glm::vec4(tBinormal, 0.f);
		}
	}
}

void AnimatedObjModel::calculateTangentBinormal(TempVertexType vertex1,
	TempVertexType vertex2, TempVertexType vertex3, glm::vec3& tangent,
	glm::vec3& binormal)
{
	glm::vec3 tVector1;
	glm::vec3 tVector2;
	glm::vec2 tuVector;
	glm::vec2 tvVector;
	float den;

	tVector1 = vertex2.pos - vertex1.pos;
	tVector2 = vertex3.pos - vertex1.pos;

	tuVector = vertex2.texCoord - vertex1.texCoord;
	tvVector = vertex3.texCoord - vertex1.texCoord;

	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	tangent = (tvVector[1] * tVector1 - tuVector[1] * tVector2) * den;
	binormal = (tuVector[0] * tVector2 - tvVector[0] * tVector1) * den;

	tangent = glm::normalize(tangent);
	binormal = glm::normalize(binormal);
}

std::istream& operator>>(std::istream& _stream, AnimatedObjModel::IVertex& _vert)
{
	char sep;

	return _stream >> _vert.posIndex >> sep >>
		_vert.texIndex >> sep >>
		_vert.normalIndex >> sep >>
		_vert.bone;
}
