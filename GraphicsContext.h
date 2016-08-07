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
	void createCommandBuffers();
	void createSemaphores();

	void createShaderModule(const std::vector<char>& code, VkShaderModule *pShaderModule);

	void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	bool checkResult(VkResult result);
	const char *resultToString(VkResult result);
};