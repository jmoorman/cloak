#pragma once

#include "stdafx.h"

typedef class GraphicsAllocation;

class GraphicsMemoryHeap
{
public:
	GraphicsMemoryHeap(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkMemoryPropertyFlags properties);
	~GraphicsMemoryHeap();

	const GraphicsAllocation& alloc(const VkMemoryRequirements &requirements);
	void free(const GraphicsAllocation &allocation);
private:
	VkMemoryAllocateInfo mAllocationInfo;
	VkDevice mDevice;
	VkMemoryPropertyFlags mProperties;
	VkDeviceMemory mMemory;
	std::vector<GraphicsAllocation> mAllocations;

	VkDeviceSize mCurrentOffset;
};

class GraphicsAllocation
{
	friend class GraphicsMemoryHeap;
public:
	GraphicsMemoryHeap *owner;
	VkDeviceMemory heapMemory;
	VkDeviceSize heapOffset;
	VkDeviceSize size;
	VkMemoryPropertyFlags properties;

protected:
	GraphicsAllocation(GraphicsMemoryHeap *owner) : owner(owner)
	{
		heapMemory = VK_NULL_HANDLE;
		heapOffset = 0;
		size = 0;
		properties = NULL;
	}
};
