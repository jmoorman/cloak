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

struct AnimationConstantBuffer {
	glm::mat4 boneMatrices[256];
};

class AnimatedMesh
{
public:
	AnimatedMesh();
	~AnimatedMesh();

	bool loadModel(const std::string & filename);
	void setAnimation(Animation *animation);

	void update(U32 elapsedMillis);
	std::vector<AnimatedSubMesh>& getSubMeshes();
	std::vector<glm::mat4>& getBoneMatrices();
	void recordCommandBuffer(VkCommandBuffer commandBuffer);

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

	std::vector<AnimatedSubMesh> mSubMeshes;
	std::vector<Bone> mBones;
	std::vector<glm::mat4> mBoneMatrices;

	Animation *mAnimation;

	VkCommandBuffer commandBuffer;
};

