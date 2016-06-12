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
	void destroy();

private:

	VkInstance mInstance;
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkSurfaceKHR mSurface;
	VkSurfaceFormatKHR mSurfaceFormat;
	VkSwapchainKHR mSwapchain;
	std::vector<VkImage> mImages;
	std::vector<VkImageView> mImageViews;
	VkCommandPool mCommandPool;
	VkQueue mQueue;

	void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	bool checkResult(VkResult result);
	const char *resultToString(VkResult result);
};

