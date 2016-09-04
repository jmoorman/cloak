#pragma once

#include "stdafx.h"

class GpuAllocation;

class GpuMemoryHeap
{
public:
	GpuMemoryHeap(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkMemoryPropertyFlags properties);
	~GpuMemoryHeap();

	const GpuAllocation* alloc(const VkMemoryRequirements &requirements);
	void free(const GpuAllocation *allocation);
private:
	VkMemoryAllocateInfo mAllocationInfo;
	VkDevice mDevice;
	VkMemoryPropertyFlags mProperties;
	VkDeviceMemory mMemory;
	std::vector<GpuAllocation *> mAllocations;

	VkDeviceSize mCurrentOffset;
};

class GpuAllocation
{
	friend class GpuMemoryHeap;
public:
	GpuMemoryHeap *owner;
	VkDeviceMemory heapMemory;
	VkDeviceSize heapOffset;
	VkDeviceSize size;
	VkMemoryPropertyFlags properties;

	void free() { owner->free(this); }

protected:
	GpuAllocation(GpuMemoryHeap *owner) : owner(owner)
	{
		heapMemory = VK_NULL_HANDLE;
		heapOffset = 0;
		size = 0;
		properties = NULL;
	}
};
