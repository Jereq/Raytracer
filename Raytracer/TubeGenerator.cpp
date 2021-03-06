#include "TubeGenerator.h"

#include <ostream>

template <typename vecType>
static int insertUnique(vecType _val, std::vector<vecType>& _outCol)
{
	auto it = std::find(_outCol.begin(), _outCol.end(), _val);
	if (it == _outCol.end())
	{
		_outCol.push_back(_val);
		return _outCol.size();
	}
	else
	{
		return it - _outCol.begin() + 1;
	}
}

static std::ostream& operator<<(std::ostream& _stream, const glm::vec2& _vec)
{
	return _stream << _vec.x << ' ' << _vec.y;
}

static std::ostream& operator<<(std::ostream& _stream, const glm::vec3& _vec)
{
	return _stream << _vec.x << ' ' << _vec.y << ' ' << _vec.z;
}

static std::ostream& operator<<(std::ostream& _stream, const glm::quat& _quat)
{
	return _stream << _quat.x << ' ' << _quat.y << ' ' << _quat.z << ' ' << _quat.w;
}

void TubeGenerator::generate()
{
	static const glm::vec3 basePos(0.f, 0.f, 0.f);
	static const float halfWidth(0.5f);
	static const float length(10.f);
	static const unsigned int segments(10);
	static const float segmentLength(length / (float)segments);
	static const glm::vec3 lengthDir(1.f, 0.f, 0.f);
	static const glm::vec3 heightDir(0.f, 1.f, 0.f);
	static const glm::vec3 widthDir(glm::cross(lengthDir, heightDir));

	static const glm::vec3 minPos(basePos - halfWidth * widthDir - halfWidth * heightDir);

	static const glm::vec2 baseTexPos(3.f / 8.f, 0.f);
	static const glm::vec2 rightTex(1.f / 4.f, 0.f);
	static const glm::vec2 upTex(0.f, 1.f / 4.f);

	static const glm::vec3 widthOffset(2.f * halfWidth * widthDir);
	static const glm::vec3 heightOffset(2.f * halfWidth * heightDir);
	static const glm::vec3 lengthOffset(segmentLength * lengthDir);

	bones.reserve(segments + 1);

	createQuad(minPos, widthOffset, heightOffset, baseTexPos, rightTex, upTex);
	for (unsigned int i = 0; i < segments; ++i)
	{
		createSegment(basePos + (float)i * lengthOffset, lengthOffset, widthOffset, baseTexPos, rightTex, upTex);
		createBone(basePos + (float)i * lengthOffset);
	}
	createQuad(minPos + widthOffset + length * lengthDir, -widthOffset, heightOffset, baseTexPos, rightTex, upTex);

	createBone(basePos + length * lengthDir);

	bindVerticesToClosestBone();
}

void TubeGenerator::outputMesh(std::ostream& _stream)
{
	_stream << "# Generated test file, don't expect fancy things" << std::endl << std::endl;
	for (const auto& pos : positions)
	{
		_stream << "v " << pos << std::endl;
	}
	_stream << std::endl;
	for (const auto& texCoord : textureCoords)
	{
		_stream << "vt " << texCoord << std::endl;
	}
	_stream << std::endl;
	for (const auto& normal : normals)
	{
		_stream << "vn " << normal << std::endl;
	}
	_stream << std::endl;
	for (const auto& bone : bones)
	{
		_stream << "b " << bone << std::endl;
	}
	_stream << std::endl;
	for (const auto& face : faces)
	{
		_stream << "f " << face << std::endl;
	}
}

void TubeGenerator::createQuad(glm::vec3 _pos, glm::vec3 _right, glm::vec3 _up,
				glm::vec2 _texPos, glm::vec2 _texRight, glm::vec2 _texUp)
{
	const glm::vec3 normal = glm::normalize(glm::cross(_right, _up));
	const int normalIndex = insertUnique(normal, normals);

	const Vertex corners[4] =
	{
		{
			insertUnique(_pos, positions),
			insertUnique(_texPos, textureCoords),
			normalIndex
		},
		{
			insertUnique(_pos + _right, positions),
			insertUnique(_texPos + _texRight, textureCoords),
			normalIndex
		},
		{
			insertUnique(_pos + _right + _up, positions),
			insertUnique(_texPos + _texRight + _texUp, textureCoords),
			normalIndex
		},
		{
			insertUnique(_pos + _up, positions),
			insertUnique(_texPos + _texUp, textureCoords),
			normalIndex
		}
	};

	const Triangle triangles[2] =
	{
		{
			{
				corners[0],
				corners[1],
				corners[2]
			}
		},
		{
			{
				corners[0],
				corners[2],
				corners[3]
			}
		}
	};

	faces.push_back(triangles[0]);
	faces.push_back(triangles[1]);
}

void TubeGenerator::createSegment(glm::vec3 _pos, glm::vec3 _forward, glm::vec3 _width,
				glm::vec2 _texPos, glm::vec2 _texRight, glm::vec2 _texUp)
{
	const glm::vec3 perpDir(glm::cross(glm::normalize(_forward), _width));
	const glm::vec3 baseCorner(_pos - 0.5f * _width - 0.5f * perpDir);

	createQuad(baseCorner                   , _forward, perpDir , _texPos, _texRight, _texUp);
	createQuad(baseCorner + perpDir         , _forward, _width  , _texPos, _texRight, _texUp);
	createQuad(baseCorner + perpDir + _width, _forward, -perpDir, _texPos, _texRight, _texUp);
	createQuad(baseCorner           + _width, _forward, -_width , _texPos, _texRight, _texUp);
}

void TubeGenerator::createBone(glm::vec3 _pos)
{
	glm::vec3 prevPos(0.f);
	if (!bonePositions.empty())
	{
		prevPos = bonePositions.back();
	}

	const std::string boneName = "bone" + std::to_string(bones.size());
	bones.emplace_back(_pos - prevPos, glm::vec3(1.f, 1.f, 1.f), glm::quat(), boneName, bones.size());
	if (bones.size() > 1)
	{
		bones[bones.size() - 2].addChild(&bones.back());
	}
	bonePositions.push_back(_pos);
}

void TubeGenerator::bindVertexToClosestBone(Vertex& _vert)
{
	const glm::vec3 vertPos = positions[_vert.pos - 1];

	float closestSq = std::numeric_limits<float>::max();
	int closestBoneIdx = 0;

	for (unsigned int i = 0; i < bonePositions.size(); ++i)
	{
		const glm::vec3 relPos = vertPos - bonePositions[i];
		const float distSq = glm::dot(relPos, relPos);

		if (distSq < closestSq)
		{
			closestSq = distSq;
			closestBoneIdx = i + 1;
		}
	}

	_vert.bone = closestBoneIdx;
}

void TubeGenerator::bindVerticesToClosestBone()
{
	for (Triangle& tri : faces)
	{
		for (Vertex& vert : tri.v)
		{
			bindVertexToClosestBone(vert);
		}
	}
}

std::ostream& operator<<(std::ostream& _stream, const TubeGenerator::Vertex& _vert)
{
	return _stream <<
		_vert.pos << '/' <<
		_vert.texCoord << '/' <<
		_vert.normal << '/' <<
		_vert.bone;
}

std::ostream& operator<<(std::ostream& _stream, const TubeGenerator::Triangle& _face)
{
	return _stream << _face.v[0] << ' ' << _face.v[1] << ' ' << _face.v[2];
}
