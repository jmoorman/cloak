#pragma once

#include "stdafx.h"

#include "Animation.h"
#include "geometry.h"

struct AnimatedSubMesh {
	std::vector<AnimatedMeshVertex> vertices;
	std::vector<U16> indices;
	std::string textureName;

	//Vulkan handles
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer constantBuffer;
	VkDeviceMemory constantBufferMemory;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkDescriptorSet descriptorSet;
};

class AnimatedMesh
{
public:
	AnimatedMesh();
	~AnimatedMesh();

	bool loadModel(const std::string & filename);
	void setAnimation(Animation *animation);

	void setPosition(const glm::vec3 &position) { mPosition = position; }
	const glm::vec3& getPosition() { return mPosition; }
	void setOrientation(const glm::quat &orientation) { mOrientation = orientation; }
	
	void rotate(float angle, const glm::vec3 &axis);

	void update(U32 elapsedMillis);
	std::vector<AnimatedSubMesh>& getSubMeshes();
	glm::mat4 getModelMatrix();
	std::vector<glm::mat4>& getBoneMatrices();

	VkBuffer mObjectConstantBuffer;
	VkDeviceMemory mObjectConstantBufferMemory;

	VkBuffer mAnimationConstantBuffer;
	VkDeviceMemory mAnimationConstantBufferMemory;
private:

	struct Bone {
		glm::quat orientation;
		glm::vec3 position;
		int parentId;
		std::string name;
		glm::mat4 inverseBindMatrix;
	};

	void readSubMesh(std::ifstream &file, U32 fileLength);
	void readBone(std::ifstream &file, U32 fileLength);

	glm::vec3 mPosition;
	glm::quat mOrientation;
	std::vector<AnimatedSubMesh> mSubMeshes;
	std::vector<Bone> mBones;
	std::vector<glm::mat4> mBoneMatrices;

	Animation *mAnimation;

	VkCommandBuffer commandBuffer;
};

