#include "GpuMemoryHeap.h"

GpuMemoryHeap::GpuMemoryHeap(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkMemoryPropertyFlags properties)
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

GpuMemoryHeap::~GpuMemoryHeap()
{
	vkFreeMemory(mDevice, mMemory, nullptr);
}

const GpuAllocation * GpuMemoryHeap::alloc(const VkMemoryRequirements &requirements)
{
	assert(requirements.memoryTypeBits & (1 << mAllocationInfo.memoryTypeIndex));
	GpuAllocation *allocation = new GpuAllocation(this);

	if (mCurrentOffset % requirements.alignment != 0)
	{
		mCurrentOffset += requirements.alignment - (mCurrentOffset % requirements.alignment);
	}

	if (mCurrentOffset + requirements.size > mAllocationInfo.allocationSize)
	{
		assert(false);
		return allocation;
	}

#if 1
	allocation->heapMemory = mMemory;
	allocation->heapOffset = mCurrentOffset;
	allocation->properties = mProperties;
	allocation->size = requirements.size;
#else
	//Faking the sub-allocations for now
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = requirements.size;
	allocInfo.memoryTypeIndex = mAllocationInfo.memoryTypeIndex;
	VkResult result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &allocation->heapMemory);
	assert(result == VK_SUCCESS);
	allocation->heapOffset = 0;
	allocation->properties = mProperties;
	allocation->size = requirements.size;
#endif

	mAllocations.push_back(allocation);

	mCurrentOffset += requirements.size;

	return allocation;
}

void GpuMemoryHeap::free(const GpuAllocation *allocation)
{
	if (allocation->heapMemory != VK_NULL_HANDLE)
	{
	}
}
