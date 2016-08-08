#pragma once

//This define needs to be included before we include vulkan.h
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan.h>

#include "stdafx.h"

class GraphicsContext
{
public:
	GraphicsContext();
	~GraphicsContext();

	void init(HINSTANCE hinstance, HWND hwnd);

	void updateUniformBuffer(U32 elapsedMillis);
	void drawFrame();

	void destroy();

private:

	VkInstance mInstance;
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkQueue mQueue;
	U32 mQueueFamilyIndex;

	VkSurfaceKHR mSurface;
	VkSurfaceFormatKHR mSurfaceFormat;

	VkSwapchainKHR mSwapchain;
	VkExtent2D mSwapchainExtent;
	std::vector<VkImageView> mImageViews;
	std::vector<VkFramebuffer> mSwapchainFramebuffers;

	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;
	VkDescriptorPool mDescriptorPool;
	VkDescriptorSet mDescriptorSet;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;
	VkSemaphore imageAcquiredSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	VkBuffer mUniformStagingBuffer;
	VkDeviceMemory mUniformStagingBufferMemory;
	VkBuffer mUniformBuffer;
	VkDeviceMemory mUniformBufferMemory;
	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
	VkImageView mTextureImageView;
	VkSampler mTextureSampler;

	uint32_t frameCount;

	//Initialization
	void createInstance();
	void createSurface(HINSTANCE hinstance, HWND hwnd);
	void selectPhysicalDevice();
	void createLogicalDevice();
	void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSet();
	void createCommandBuffers();
	void createSemaphores();

	//Utility functions
	void createShaderModule(const std::vector<char>& code, VkShaderModule *pShaderModule);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBufferOut, VkDeviceMemory *pBufferMemoryOut);
	void createImage(U32 width, U32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
	void createImageView(VkImage image, VkFormat format, VkImageView * pImageViewOut);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, U32 width, U32 height);
	void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkCommandBuffer beginSingleUseCommandBuffer();
	void endSingleUseCommandBuffer(VkCommandBuffer commandBuffer);
	U32 findMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties);

	bool checkResult(VkResult result);
	const char *resultToString(VkResult result);
};

