#include "AnimatedMesh.h"

#define DEFAULT_TEXTURE_PATH "../data/textures/"

AnimatedMesh::AnimatedMesh() : mPosition(0, 0, 0), mOrientation(0, 0, 0, 1)
{
}


AnimatedMesh::~AnimatedMesh()
{
}

bool AnimatedMesh::loadModel(const std::string &filename)
{
	mSubMeshes.clear();
	mBones.clear();

	U32 boneCount = 0;
	U32 meshCount = 0;

	//attempt to open the file
	std::ifstream file(filename);
	if (file.fail()) {
		return false;
	}

	//get file length
	file.seekg(0, std::ios::end);
	int fileLength = file.tellg();
	file.seekg(std::ios::beg);

	std::string token;
	std::string ignore;

	while (file >> token, !file.eof()) {
		if (token == "numJoints") {
			file >> boneCount;
		}
		else if (token == "numMeshes") {
			file >> meshCount;
		}
		else if (token == "joints") {
			file >> ignore; //opening brace
			for (int i = 0; i < boneCount; i++) {
				readBone(file, fileLength);
			}
			file >> ignore; // closing brace
		}
		else if (token == "mesh") {
			readSubMesh(file, fileLength);
		}
	}
	assert(boneCount == mBones.size());
	assert(meshCount == mSubMeshes.size());
	return true;
}

void AnimatedMesh::setAnimation(Animation *animation)
{
	assert(animation->getBoneCount() == mBones.size());
	mAnimation = animation;
}

void AnimatedMesh::rotate(float angle, const glm::vec3 & axis)
{
	mOrientation = glm::angleAxis(angle, axis) * mOrientation;
}

void AnimatedMesh::update(U32 elapsedMillis)
{
	if (mAnimation)
	{
		mAnimation->update(elapsedMillis);
		const Animation::FrameSkeleton &skeleton = mAnimation->getSkeleton();
		for (int i = 0; i < skeleton.bones.size(); i++)
		{
			const Animation::SkeletonBone &bone = skeleton.bones[i];
		
			glm::mat4 boneTranslation = glm::translate(glm::mat4(), bone.position);
			glm::mat4 boneRotation = glm::mat4_cast(bone.orientation);
			mBoneMatrices[i] = (boneTranslation * boneRotation) * mBones[i].inverseBindMatrix;
		}
	}
}

std::vector<AnimatedSubMesh>& AnimatedMesh::getSubMeshes()
{
	return mSubMeshes;
}

glm::mat4 AnimatedMesh::getModelMatrix()
{
	glm::mat4 modelMatrix = glm::mat4_cast(mOrientation);
	modelMatrix[3][0] = mPosition[0];
	modelMatrix[3][1] = mPosition[1];
	modelMatrix[3][2] = mPosition[2];
	return modelMatrix;
}

std::vector<glm::mat4>& AnimatedMesh::getBoneMatrices()
{
	return mBoneMatrices;
}

struct BoneWeight
{
	int boneId;
	float bias;
	glm::vec3 position;
};

struct VertexInfo
{
	glm::vec2 textureCoord;
	int startWeight;
	int weightCount;
};

struct Triangle
{
	int indices[3];
};

void AnimatedMesh::readSubMesh(std::ifstream & file, U32 fileLength)
{
	AnimatedSubMesh mesh;

	//Working lists
	std::vector<BoneWeight> weightList;
	std::vector<VertexInfo> vertexList;
	std::vector<Triangle> triangleList;

	std::string ignore;
	std::string token;
	int numVerts, numTris, numWeights;

	file >> ignore; // opening brace
	while (file >> token, token != "}") { // read until the closing brace
		if (token == "shader") {
			std::string textureName;
			file >> textureName;
			size_t n;
			while ((n = textureName.find('\"')) != std::string::npos) textureName.erase(n, 1);
			mesh.textureName = DEFAULT_TEXTURE_PATH + textureName;
		}
		else if (token == "numverts") {
			file >> numVerts;               // Read in the vertices
			file.ignore(fileLength, '\n');
			for (int i = 0; i < numVerts; i++) {
				VertexInfo vert;
				std::string ignore;
				file >> ignore >> ignore >> ignore;                    // vert vertIndex (
				file >> vert.textureCoord.x >> vert.textureCoord.y >> ignore;  //  s t )
				file >> vert.startWeight >> vert.weightCount;
				file.ignore(fileLength, '\n');
				vertexList.push_back(vert);
			}
		}
		else if (token == "numtris") {
			file >> numTris;
			file.ignore(fileLength, '\n');
			for (int i = 0; i < numTris; i++) {
				Triangle tri;
				file >> ignore >> ignore;
				file >> tri.indices[0] >> tri.indices[1] >> tri.indices[2];
				file.ignore(fileLength, '\n');
				triangleList.push_back(tri);
			}
		}
		else if (token == "numweights") {
			file >> numWeights;
			file.ignore(fileLength, '\n');
			for (int i = 0; i < numWeights; i++) {
				BoneWeight weight;
				file >> ignore >> ignore;
				file >> weight.boneId >> weight.bias >> ignore;
				file >> weight.position.x >> weight.position.y >> weight.position.z >> ignore;
				file.ignore(fileLength, '\n');
				weightList.push_back(weight);
			}
		}
		else {
			file.ignore(fileLength, '\n');
		}
	}

	//make sure the file wasn't lying to us (...or we misread something...)
	assert(numVerts == vertexList.size());
	assert(numTris == triangleList.size());
	assert(numWeights == weightList.size());

	//compute the vertices in the bind pose
	for (unsigned int i = 0; i < vertexList.size(); i++) {
		VertexInfo& vertInfo = vertexList[i];
		assert(vertInfo.weightCount <= 4);
		AnimatedMeshVertex vertex;
		vertex.position = glm::vec3(0);
		vertex.normal = glm::vec3(0);
		vertex.texcoord = vertInfo.textureCoord;
		vertex.bone_weights = glm::vec4(0.f);
		vertex.bone_indices = glm::uvec4(0);

		for (int j = vertInfo.startWeight; j < vertInfo.startWeight + vertInfo.weightCount; j++) {
			BoneWeight& weight = weightList[j];
			Bone& bone = mBones[weight.boneId];
			//convert the weight position from bone local to object local
			glm::vec3 rotatedPos = bone.orientation * weight.position;
			vertex.position += (bone.position + rotatedPos) * weight.bias;
			vertex.bone_indices[j - vertInfo.startWeight] = weight.boneId;
			vertex.bone_weights[j - vertInfo.startWeight] = weight.bias;
			assert(weight.boneId >= 0 && weight.boneId < numWeights);
		}
		mesh.vertices.push_back(vertex);

	}

	//compute the normals in the bind pose
	for (unsigned int i = 0; i < triangleList.size(); i++) {
		Triangle& tri = triangleList[i];
		glm::vec3 v0 = mesh.vertices[tri.indices[0]].position;
		glm::vec3 v1 = mesh.vertices[tri.indices[1]].position;
		glm::vec3 v2 = mesh.vertices[tri.indices[2]].position;
		glm::vec3 normal = glm::cross(v2 - v0, v1 - v0);
		mesh.vertices[tri.indices[0]].normal += normal;
		mesh.vertices[tri.indices[1]].normal += normal;
		mesh.vertices[tri.indices[2]].normal += normal;
		mesh.indices.push_back(tri.indices[0]);
		mesh.indices.push_back(tri.indices[1]);
		mesh.indices.push_back(tri.indices[2]);
	}
	//normalize the normals and convert to joint-local space
	for (unsigned int i = 0; i < mesh.vertices.size(); i++)
	{
		AnimatedMeshVertex& vert = mesh.vertices[i];
		VertexInfo &vertInfo = vertexList[i];
		glm::vec3 normal = glm::normalize(vert.normal);
		
		vert.normal = glm::vec3(0);
		for (int j = vertInfo.startWeight; j < vertInfo.startWeight + vertInfo.weightCount; j++) {
			const BoneWeight& weight = weightList[j];
			const Bone& bone = mBones[weight.boneId];
			vert.normal += (normal * bone.orientation) * weight.bias;
		}
	}
	mSubMeshes.push_back(mesh);
}

void AnimatedMesh::readBone(std::ifstream & file, U32 fileLength)
{
	Bone bone;
	std::string ignore;
	file >> bone.name >> bone.parentId >> ignore;
	file >> bone.position.x >> bone.position.y >> bone.position.z >> ignore >> ignore;
	file >> bone.orientation.x >> bone.orientation.y >> bone.orientation.z >> ignore;
	file.ignore(fileLength, '\n');

	size_t n;
	while ((n = bone.name.find('\"')) != std::string::npos) bone.name.erase(n, 1);
	float t = 1.0f - (bone.orientation.x * bone.orientation.x) -
		(bone.orientation.y * bone.orientation.y) -
		(bone.orientation.z * bone.orientation.z);
	if (t < 0.0f) {
		bone.orientation.w = 0.0f;
	}
	else {
		bone.orientation.w = -sqrtf(t);
	}
	glm::mat4 boneTranslation = glm::translate(glm::mat4(), bone.position);
	glm::mat4 boneRotation = glm::mat4_cast(bone.orientation);
	glm::mat4 bindMatrix = boneTranslation * boneRotation;
	bone.inverseBindMatrix = glm::inverse(bindMatrix);
	mBones.push_back(bone);
	mBoneMatrices.push_back(glm::mat4());
}