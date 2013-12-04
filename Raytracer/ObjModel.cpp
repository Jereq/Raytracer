#include "ObjModel.h"
#include <fstream>
using std::ifstream;


ObjModel::ObjModel(void)
{
}

ObjModel::ObjModel(const ObjModel &objModel)
{
}

ObjModel::~ObjModel(void)
{
}

//int ObjModel::GetIndexCount(void)
//{
//	return mIndexCount;
//}

int ObjModel::GetVertexCount(void)
{
	return mVertexCount;
}

void ObjModel::GetPosition(float &x, float &y, float &z)
{
	x = mPositionX;
	y = mPositionY;
	z = mPositionZ;
}

//ID3D11ShaderResourceView **ObjModel::GetTextureArray(void)
//{
//	return mTextureArray->GetTextureArray();
//}

void ObjModel::SetPosition(float x, float y, float z)
{
	mPositionX = x;
	mPositionY = y;
	mPositionZ = z;
}

bool ObjModel::Initialize(cl::Context &context, const char *modelFilename/*, 
	WCHAR *textureFilename1, WCHAR *textureFilename2*/)
{
	bool tResult;

	tResult = LoadFile(modelFilename);
	if(!tResult)
	{
		return false;
	}

	CalculateModelVectors();

	//Calling initialization functions for vertex and index buffers
	tResult = InitializeBuffers(context);
	if(!tResult)
	{
		return false;
	}

	/*tResult = LoadTextures(d3DDevice, textureFilename1, textureFilename2);
	if(!tResult)
	{
		return false;
	}*/

	return true;
}

void ObjModel::Shutdown(void)
{
	//ReleaseTextures();
	ShutdownBuffers(); 
	ReleaseModel();
}

cl::Buffer ObjModel::getBuffer()
{
	return mVertexBuffer;
}

bool ObjModel::InitializeBuffers(cl::Context &context)
{
	vector<VertexType>	tVertices;
	tVertices.resize(mVertexCount);

	//Load the vertex array and index array with data.
	for(int i = 0; i < mVertexCount; i++)
	{
		tVertices[i].position = glm::vec4(mModel[i].x, mModel[i].y, mModel[i].z, 1.f);
		tVertices[i].texture = glm::vec4(mModel[i].tu, mModel[i].tv, 0.f, 0.f);
		tVertices[i].normal = glm::vec4(mModel[i].nx, mModel[i].ny, mModel[i].nz, 0.f);
		tVertices[i].tangent = glm::vec4(mModel[i].tx, mModel[i].ty, mModel[i].tz, 0.f);
		tVertices[i].binormal = glm::vec4(mModel[i].bx, mModel[i].by, mModel[i].bz, 0.f);
	}

	mVertexBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(VertexType) * mVertexCount, tVertices.data());
	
	return true;
}

void ObjModel::ShutdownBuffers(void) 
{
	mVertexBuffer = cl::Buffer();
}

/*bool ObjModel::LoadTextures(ID3D11Device *d3DDevice, WCHAR *filename1,
	WCHAR *filename2)
{
	bool tResult;

	mTextureArray = new TextureArray();
	if(!mTextureArray)
	{
		return false;
	}

	tResult = mTextureArray->Initialize(d3DDevice, filename1, filename2);
	if(!tResult)
	{
		return false;
	}

	return true;
}*/

//void ObjModel::ReleaseTextures(void)
//{
//	SAFE_SHUTDOWN(mTextureArray);
//}

bool ObjModel::LoadFile(const char *filename)
{	
	bool tResult;
	int tVertexCount;
	int tTextureCount;
	int tNormalCount;
	int tFaceCount;
	int tGroupCount;

	tResult = ReadFileCounts(filename, tVertexCount, tTextureCount,
		tNormalCount, tFaceCount, tGroupCount);
	if(!tResult)
	{
		return false;
	}

	mVertexCount = (tFaceCount * 3);
	//mIndexCount = (tFaceCount * 3);
	tResult = LoadDataStructures(filename, tVertexCount, tTextureCount,
		tNormalCount, tFaceCount);
	if(!tResult)
	{
		return false;
	}

	return true;
}

bool ObjModel::ReadFileCounts(const char *filename, int &vertexCount,
	int &textureCount, int &normalCount, int &faceCount, int &groupCount)
{	
	ifstream tFileIn;
	char tInput;
	vertexCount = 0;
	textureCount = 0;
	normalCount = 0;
	faceCount = 0;

	tFileIn.open(filename);
	if(tFileIn.fail() == true)
	{
		return false;
	}

	tFileIn.get(tInput);
	while(!tFileIn.eof())
	{
		if(tInput == 'v')
		{
			tFileIn.get(tInput);
			if(tInput == ' ') 
			{
				vertexCount++;
			}

			if(tInput == 't')
			{
				textureCount++;
			}

			if(tInput == 'n')
			{
				normalCount++;
			}
		}

		if(tInput == 'g')
		{
			groupCount++;
		}

		if(tInput == 'f')
		{
			faceCount++;
		}

		while(tInput != '\n')
		{
			tFileIn.get(tInput);
		}

		tFileIn.get(tInput);
	}
	tFileIn.close();

	return true;
}

bool ObjModel::LoadDataStructures(const char *filename, int &vertexCount,
	int &textureCount, int &normalCount, int &faceCount)
{
	vector<ReadVertexType> tVertices;
	vector<ReadVertexType> tTexCoords;
	vector<ReadVertexType> tNormals;
	vector<FaceType> tFaces;
	ifstream tFileIn;
	int tVertexIndex;
	int tTexcoordIndex;
	int tNormalIndex;
	int tFaceIndex;
	int vIndex;
	int tIndex;
	int nIndex;
	char tInput1;
	char tInput2;
	
	tVertexIndex = 0;
	tTexcoordIndex = 0;
	tNormalIndex = 0;
	tFaceIndex = 0;

	tFileIn.open(filename);
	if(tFileIn.fail() == true)
	{
		return false;
	}

	mModel.resize(faceCount * 3);

	//Initialize the four data structures.
	tVertices.resize(vertexCount);
	tTexCoords.resize(textureCount);
	tNormals.resize(normalCount);
	tFaces.resize(faceCount);

	tFileIn.get(tInput1);
	while(!tFileIn.eof())
	{
		tFileIn.get(tInput1);
		if(tInput1 == 'v')
		{
			tFileIn.get(tInput1);

			//Read in the vertices.
			if(tInput1 == ' ') 
			{ 
				tFileIn >> tVertices[tVertexIndex].x;
				tFileIn >> tVertices[tVertexIndex].y;
				tFileIn >> tVertices[tVertexIndex].z;

				//Invert the Z vertex to change to left hand system.
				//tVertices[tVertexIndex].z = tVertices[tVertexIndex].z * -1.0f;
				tVertexIndex++; 
			}

			//Read in the texture uv coordinates.
			if(tInput1 == 't') 
			{ 
				tFileIn >> tTexCoords[tTexcoordIndex].x;
				tFileIn >> tTexCoords[tTexcoordIndex].y;

				//Invert the V texture coordinates to left hand system.
				//tTexCoords[tTexcoordIndex].y = 1.0f - tTexCoords[tTexcoordIndex].y;
				tTexcoordIndex++; 
			}

			//Read in the normals.
			if(tInput1 == 'n') 
			{ 
				tFileIn >> tNormals[tNormalIndex].x;
				tFileIn >> tNormals[tNormalIndex].y;
				tFileIn >> tNormals[tNormalIndex].z;

				//Invert the Z normal to change to left hand system.
				//tNormals[tNormalIndex].z = tNormals[tNormalIndex].z * -1.0f;
				tNormalIndex++; 
			}
		}

		//Read in the faces.
		if(tInput1 == 'f') 
		{
			tFileIn.get(tInput1);
			if(tInput1 == ' ')
			{
				//Read the face data in backwards to convert it to a left hand
				//system from right hand system.
				tFileIn >> tFaces[tFaceIndex].vIndex1 >> tInput2 >>
					tFaces[tFaceIndex].tIndex1 >> tInput2 >>
					tFaces[tFaceIndex].nIndex1 >> tFaces[tFaceIndex].vIndex2 >>
					tInput2 >> tFaces[tFaceIndex].tIndex2 >> tInput2 >>
					tFaces[tFaceIndex].nIndex2 >> tFaces[tFaceIndex].vIndex3 >>
					tInput2 >> tFaces[tFaceIndex].tIndex3 >> tInput2 >>
					tFaces[tFaceIndex].nIndex3;
				tFaceIndex++;
			}
		}
	}
	tFileIn.close();

	int i = 0;
	int j = 0;
	bool done = false;
	while(!done)
	{		
		vIndex = tFaces[i].vIndex1 - 1;
		tIndex = tFaces[i].tIndex1 - 1;
		nIndex = tFaces[i].nIndex1 - 1;

		mModel[j].x = tVertices[vIndex].x;
		mModel[j].y = tVertices[vIndex].y;
		mModel[j].z = tVertices[vIndex].z;
		mModel[j].tu = tTexCoords[tIndex].x;
		mModel[j].tv = tTexCoords[tIndex].y;
		mModel[j].nx = tNormals[nIndex].x;
		mModel[j].ny = tNormals[nIndex].y;
		mModel[j].nz = tNormals[nIndex].z;
		j++;

		vIndex = tFaces[i].vIndex2 - 1;
		tIndex = tFaces[i].tIndex2 - 1;
		nIndex = tFaces[i].nIndex2 - 1;

		mModel[j].x = tVertices[vIndex].x;
		mModel[j].y = tVertices[vIndex].y;
		mModel[j].z = tVertices[vIndex].z;
		mModel[j].tu = tTexCoords[tIndex].x;
		mModel[j].tv = tTexCoords[tIndex].y;
		mModel[j].nx = tNormals[nIndex].x;
		mModel[j].ny = tNormals[nIndex].y;
		mModel[j].nz = tNormals[nIndex].z;
		j++;

		vIndex = tFaces[i].vIndex3 - 1;
		tIndex = tFaces[i].tIndex3 - 1;
		nIndex = tFaces[i].nIndex3 - 1;

		mModel[j].x = tVertices[vIndex].x;
		mModel[j].y = tVertices[vIndex].y;
		mModel[j].z = tVertices[vIndex].z;
		mModel[j].tu = tTexCoords[tIndex].x;
		mModel[j].tv = tTexCoords[tIndex].y;
		mModel[j].nx = tNormals[nIndex].x;
		mModel[j].ny = tNormals[nIndex].y;
		mModel[j].nz = tNormals[nIndex].z;
		j++;

		i++;
		if((i == tFaceIndex) || (j == mVertexCount))
		{
			done = true;
		}
	}

	/*SAFE_DELETE_ARRAY(tVertices);
	SAFE_DELETE_ARRAY(tTexCoords);
	SAFE_DELETE_ARRAY(tNormals);
	SAFE_DELETE_ARRAY(tFaces);*/

	return true;
}

void ObjModel::ReleaseModel(void)
{
	//SAFE_DELETE_ARRAY(mModel);
}

void ObjModel::CalculateModelVectors(void)
{
	int tFaceCount;
	int tIndex;
	TempVertexType tVertex1;
	TempVertexType tVertex2;
	TempVertexType tVertex3;
	VectorType tTangent;
	VectorType tBinormal;

	tFaceCount = mVertexCount / 3;
	tIndex = 0;
	for(int i = 0; i < tFaceCount; i++)
	{
		tVertex1.x	= mModel[tIndex].x;
		tVertex1.y	= mModel[tIndex].y;
		tVertex1.z	= mModel[tIndex].z;
		tVertex1.tu	= mModel[tIndex].tu;
		tVertex1.tv	= mModel[tIndex].tv;
		tVertex1.nx	= mModel[tIndex].nx;
		tVertex1.ny	= mModel[tIndex].ny;
		tVertex1.nz	= mModel[tIndex].nz;
		tIndex++;

		tVertex2.x	= mModel[tIndex].x;
		tVertex2.y	= mModel[tIndex].y;
		tVertex2.z	= mModel[tIndex].z;
		tVertex2.tu	= mModel[tIndex].tu;
		tVertex2.tv	= mModel[tIndex].tv;
		tVertex2.nx	= mModel[tIndex].nx;
		tVertex2.ny	= mModel[tIndex].ny;
		tVertex2.nz	= mModel[tIndex].nz;
		tIndex++;	

		tVertex3.x	= mModel[tIndex].x;
		tVertex3.y	= mModel[tIndex].y;
		tVertex3.z	= mModel[tIndex].z;
		tVertex3.tu	= mModel[tIndex].tu;
		tVertex3.tv	= mModel[tIndex].tv;
		tVertex3.nx	= mModel[tIndex].nx;
		tVertex3.ny	= mModel[tIndex].ny;
		tVertex3.nz	= mModel[tIndex].nz;
		tIndex++;

		CalculateTangentBinormal(tVertex1, tVertex2, tVertex3, tTangent,
			tBinormal);

		mModel[tIndex-1].tx = tTangent.x;
		mModel[tIndex-1].ty = tTangent.y;
		mModel[tIndex-1].tz = tTangent.z;
		mModel[tIndex-1].bx = tBinormal.x;
		mModel[tIndex-1].by = tBinormal.y;
		mModel[tIndex-1].bz = tBinormal.z;

		mModel[tIndex-2].tx = tTangent.x;
		mModel[tIndex-2].ty = tTangent.y;
		mModel[tIndex-2].tz = tTangent.z;
		mModel[tIndex-2].bx = tBinormal.x;
		mModel[tIndex-2].by = tBinormal.y;
		mModel[tIndex-2].bz = tBinormal.z;

		mModel[tIndex-3].tx = tTangent.x;
		mModel[tIndex-3].ty = tTangent.y;
		mModel[tIndex-3].tz = tTangent.z;
		mModel[tIndex-3].bx = tBinormal.x;
		mModel[tIndex-3].by = tBinormal.y;
		mModel[tIndex-3].bz = tBinormal.z;
	}
}

void ObjModel::CalculateTangentBinormal(TempVertexType vertex1,
	TempVertexType vertex2, TempVertexType vertex3, VectorType &tangent,
	VectorType &binormal)
{
	float tVector1[3];
	float tVector2[3];
	float tuVector[2];
	float tvVector[2];
	float den;
	float tLength;

	tVector1[0] = vertex2.x - vertex1.x;
	tVector1[1] = vertex2.y - vertex1.y;
	tVector1[2] = vertex2.z - vertex1.z;

	tVector2[0] = vertex3.x - vertex1.x;
	tVector2[1] = vertex3.y - vertex1.y;
	tVector2[2] = vertex3.z - vertex1.z;

	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	tangent.x = (tvVector[1] * tVector1[0] - tvVector[0] * tVector2[0]) * den;
	tangent.y = (tvVector[1] * tVector1[1] - tvVector[0] * tVector2[1]) * den;
	tangent.z = (tvVector[1] * tVector1[2] - tvVector[0] * tVector2[2]) * den;

	binormal.x = (tuVector[0] * tVector2[0] - tuVector[1] * tVector1[0]) * den;
	binormal.y = (tuVector[0] * tVector2[1] - tuVector[1] * tVector1[1]) * den;
	binormal.z = (tuVector[0] * tVector2[2] - tuVector[1] * tVector1[2]) * den;

	tLength = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) +
		(tangent.z * tangent.z));

	binormal.x = binormal.x / tLength;
	binormal.y = binormal.y / tLength;
	binormal.z = binormal.z / tLength;
}