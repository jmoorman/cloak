#pragma once

#include "stdafx.h"

#include "GpuMemoryHeap.h"

struct GpuBuffer
{
	VkBuffer buffer;
	GpuAllocation *memory;
};

struct GpuImage
{
	VkImage image;
	const GpuAllocation *allocation;
};
