#pragma once

#include "stdafx.h"

struct GpuBuffer
{
	GpuBuffer() : buffer(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE) {}

	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;
};

struct GpuImage
{
	GpuImage() : image(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE) {}

	VkImage image;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;
};
