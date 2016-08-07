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
	VkPipeline mPipeline;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;
	VkSemaphore imageAcquiredSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	uint32_t frameCount;

	//Initialization helpers
	void createInstance();
	void createSurface(HINSTANCE hinstance, HWND hwnd);
	void selectPhysicalDevice();
	void createLogicalDevice();
	void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createVertexBuffer();
	void createIndexBuffer();
	void createCommandBuffers();
	void createSemaphores();

	void createShaderModule(const std::vector<char>& code, VkShaderModule *pShaderModule);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBufferOut, VkDeviceMemory *pBufferMemoryOut);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	U32 findMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties);

	bool checkResult(VkResult result);
	const char *resultToString(VkResult result);
};

