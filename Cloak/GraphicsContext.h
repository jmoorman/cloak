#pragma once

//This define needs to be included before we include vulkan.h
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan.h>

#include "stdafx.h"

#include "AnimatedMesh.h"
#include "GpuMemoryHeap.h"
#include "graphics_resources.h"

#ifdef NDEBUG
const bool gEnableValidationLayers = false;
#else
const bool gEnableValidationLayers = true;
#endif

class GraphicsContext
{
public:
	GraphicsContext();
	~GraphicsContext();

	void init(HINSTANCE hinstance, HWND hwnd);

	void createCommandBuffer(AnimatedMesh *animatedMesh);
	void updateConstantBuffer(const void *pData, U32 bufferSize, VkBuffer buffer);

	void updateSceneConstantBuffer(const SceneConstantBuffer &sceneConstantBuffer);
	void drawFrame();

	void destroy();

private:

	VkPhysicalDeviceProperties mPhysicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties mPhysicalMemoryProperties;

	VkInstance mInstance;
	VkDebugReportCallbackEXT mDebugCallback;
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
	
	GpuImage m_depthImage;
	VkImageView m_depthImageView;
	VkFormat m_depthFormat;

	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;
	VkDescriptorPool mDescriptorPool;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;
	std::vector<VkCommandBuffer> mSecondaryCommandBuffers;
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

	VkSampler mTextureSampler;

	GpuMemoryHeap *m_graphicsMemory;
	GpuMemoryHeap *m_stagingGraphicsMemory;

	U32 mFrameCount;

	//Initialization
	void createInstance();
	void setupDebugCallback();
	void createSurface(HINSTANCE hinstance, HWND hwnd);
	void selectPhysicalDevice();
	void createLogicalDevice();
	void createMemoryHeaps();
	void createSwapchain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createTextureSampler();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();
	void createDescriptorPool();
	void createCommandBuffers();
	void createSemaphores();

	void createImageFromSurface(SDL_Surface *pSurface, GpuImage *pImageOut);
	void createBufferFromData(void *pData, U32 bufferSize, VkBufferUsageFlags usage, VkBuffer *pBufferOut, VkDeviceMemory *pBufferMemoryOut);

	//Utility functions
	bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);
	void createShaderModule(const std::vector<char>& code, VkShaderModule *pShaderModule);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBufferOut, VkDeviceMemory *pBufferMemoryOut);
	void createImage(U32 width, U32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, GpuImage *pImageOut);
	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView * pImageViewOut);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, U32 width, U32 height);
	void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkCommandBuffer beginSingleUseCommandBuffer();
	void endSingleUseCommandBuffer(VkCommandBuffer commandBuffer);
	U32 findMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	bool checkResult(VkResult result);
	const char *resultToString(VkResult result);
};

