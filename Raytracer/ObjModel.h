#pragma once
//#include "TextureArray.h"
#include <glm/glm.hpp>
#include <CL/cl.hpp>
#include <vector>
using std::vector;

class ObjModel
{
private:
	//this typedef must match the layout in the Shader Class
	struct VertexType
	{
		glm::vec4 position;
		glm::vec4 texture;
		glm::vec4 normal;
		glm::vec4 tangent;
		glm::vec4 binormal;
	};	
	struct FaceType
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	};
	struct ReadVertexType
	{
		float x, y, z;
	};
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
	};
	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
	struct VectorType
	{
		float x, y, z;
	};

	cl::Buffer mVertexBuffer;
	//cl::Buffer mIndexBuffer;
	int	mVertexCount;
	//int mIndexCount;
	//TextureArray *mTextureArray;
	vector<ModelType> mModel;
	float mPositionX;
	float mPositionY;
	float mPositionZ;

public:
	ObjModel(void);
	ObjModel(const ObjModel &objModel);
	~ObjModel(void);

	//int GetIndexCount(void);
	int GetVertexCount(void);
	void GetPosition(float &posX, float &posY, float &posZ);
	//ID3D11ShaderResourceView **GetTextureArray(void);

	void SetPosition(float posX, float posY, float posZ);

	bool Initialize(cl::Context &context, char *modelFilename/*, WCHAR *textureFilename1, WCHAR *textureFilename2*/);
	void Shutdown(void);

	cl::Buffer getBuffer();

private:
	bool InitializeBuffers(cl::Context &context);
	void ShutdownBuffers(void);
	

	/*bool LoadTextures(ID3D11Device *d3DDevice, WCHAR *textureFilename1,
		WCHAR *textureFilename2);
	void ReleaseTextures(void);*/

	bool LoadFile(char *filename);
	bool ReadFileCounts(char *filename,	int &vertexCount, int &textureCount, 
		int &normalCount, int &faceCout, int &groupCount);
	bool LoadDataStructures(char *filename, int &vertexCount, int &textureCount, 
		int &normalCount, int &faceCount);
	void ReleaseModel(void);

	void CalculateModelVectors(void);
	void CalculateTangentBinormal(TempVertexType vertex1,
		TempVertexType vertex2, TempVertexType vertex3, VectorType &tangent,
		VectorType &binormal);
};