#include "GraphicsMemoryHeap.h"



GraphicsMemoryHeap::GraphicsMemoryHeap(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkMemoryPropertyFlags properties)
	: mDevice(device), mProperties(properties), mCurrentOffset(0)
{
	bool bFoundMemoryType = false;
	U32 memoryTypeIndex;
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	for (U32 i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			memoryTypeIndex = i;
			bFoundMemoryType = true;
		}
	}

	mAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mAllocationInfo.allocationSize = size;
	mAllocationInfo.memoryTypeIndex = memoryTypeIndex;
	mAllocationInfo.pNext = nullptr;
	VkResult result = vkAllocateMemory(mDevice, &mAllocationInfo, nullptr, &mMemory);
	assert(result == VK_SUCCESS);
}

GraphicsMemoryHeap::~GraphicsMemoryHeap()
{
	vkFreeMemory(mDevice, mMemory, nullptr);
}

const GraphicsAllocation & GraphicsMemoryHeap::alloc(const VkMemoryRequirements &requirements)
{
	GraphicsAllocation allocation(this);

	mCurrentOffset += requirements.alignment - (mCurrentOffset % requirements.alignment);
	if (mCurrentOffset + requirements.size > mAllocationInfo.allocationSize)
	{
		assert(false);
		return allocation;
	}

	allocation.heapMemory = mMemory;
	allocation.heapOffset = mCurrentOffset;
	allocation.properties = mProperties;
	allocation.size = requirements.size;
	mAllocations.push_back(allocation);

	mCurrentOffset += requirements.size;

	return allocation;
}

void GraphicsMemoryHeap::free(const GraphicsAllocation &allocation)
{
	if (allocation.heapMemory != VK_NULL_HANDLE)
	{

	}
}
