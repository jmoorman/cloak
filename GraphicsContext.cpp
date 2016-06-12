#include "GraphicsContext.h"

#define VK_CALL(func_call) { \
	result = func_call; \
	if(result != VK_SUCCESS) \
		std::cerr << "Error in " << __FILE__ << ":" << __LINE__ << "calling " << #func_call << "\n\treturned " << resultToString(result) << '\n'; \
}

GraphicsContext::GraphicsContext()
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::init(HINSTANCE hinstance, HWND hwnd)
{
	VkResult result = VK_SUCCESS;

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = nullptr;
	
	result = vkCreateInstance(&instanceInfo, nullptr, &mInstance);
	assert(checkResult(result));

	uint32_t physicalDeviceCount = 0;
	result = vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
	assert(checkResult(result));
	assert(physicalDeviceCount > 0);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, &physicalDevices[0]);
	
	mPhysicalDevice = physicalDevices[0];
	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProps);
	std::cout << "Defaulting to device 0" << std::endl
		<< "\tdriver version: " << deviceProps.driverVersion << std::endl
		<< "\tdevice name: " << deviceProps.deviceName << std::endl
		<< "\tdevice type: " << deviceProps.deviceType << std::endl
		<< "\tVulkan API version: " 
		<< VK_VERSION_MAJOR(deviceProps.apiVersion) << "."
		<< VK_VERSION_MINOR(deviceProps.apiVersion) << "."
		<< VK_VERSION_PATCH(deviceProps.apiVersion) << std::endl;

	uint32_t queueFamilyCount = 0;
	uint32_t queueFamilyIndex = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, &queueFamilyProperties[0]);
	std::cout << "Defaulting to queue family 0" << std::endl
		<< "\tqueue count: " << queueFamilyProperties[queueFamilyIndex].queueCount << std::endl
		<< "\tsupported queue operations:" << std::endl;
	VkQueueFlags queueFlags = queueFamilyProperties[queueFamilyIndex].queueFlags;
	if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
		std::cout << "\t- Graphics" << std::endl;
	}
	if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
		std::cout << "\t- Compute" << std::endl;
	}
	if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
		std::cout << "\t- Transfer" << std::endl;
	}
	if (queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
		std::cout << "\t- Sparse Binding" << std::endl;
	}

	VkDeviceQueueCreateInfo deviceQueueInfo = {};
	deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueInfo.queueFamilyIndex = queueFamilyIndex;
	float queuePriorities[] = { 1.f };
	deviceQueueInfo.pQueuePriorities = queuePriorities;
	deviceQueueInfo.queueCount = 1;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &deviceQueueInfo;
	result = vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice);
	assert(checkResult(result));

	vkGetDeviceQueue(mDevice, queueFamilyIndex, 0, &mQueue);

	//Surface creation

	VkWin32SurfaceCreateInfoKHR surfaceInfo;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = hinstance;
	surfaceInfo.hwnd = hwnd;
	result = vkCreateWin32SurfaceKHR(mInstance, &surfaceInfo, nullptr, &mSurface);
	assert(checkResult(result));

	VkBool32 isSupported = false;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, queueFamilyIndex, mSurface, &isSupported);
	assert(checkResult(result));
	assert(isSupported);

	uint32_t formatCount = 0;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
	assert(checkResult(result));
	assert(formatCount > 0);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, &surfaceFormats[0]);
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		mSurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		mSurfaceFormat.format = surfaceFormats[0].format;
	}
	mSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);
	assert(checkResult(result));
	uint32_t desiredImages = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0
			&& desiredImages > surfaceCapabilities.maxImageCount) {
		desiredImages = surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = mSurface;
	swapchainInfo.minImageCount = desiredImages;
	swapchainInfo.imageFormat = mSurfaceFormat.format;
	swapchainInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 0;
	swapchainInfo.pQueueFamilyIndices = nullptr;
	swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainInfo.clipped = true;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	result = vkCreateSwapchainKHR(mDevice, &swapchainInfo, nullptr, &mSwapchain);
	assert(checkResult(result));

	uint32_t imageCount;
	result = vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	assert(checkResult(result));
	mImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, &mImages[0]);

	mImageViews.resize(imageCount);
	VkImageViewCreateInfo colorAttachmentView = {};
	colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorAttachmentView.pNext = nullptr;
	colorAttachmentView.format = mSurfaceFormat.format;
	colorAttachmentView.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	VkImageSubresourceRange &subresourceRange = colorAttachmentView.subresourceRange;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;
	colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorAttachmentView.flags = 0;

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
	result = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);
	assert(checkResult(result));

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = mCommandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(mDevice, &commandBufferInfo, &commandBuffer);
	assert(checkResult(result));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	assert(checkResult(result));

	for (uint32_t imageIndex = 0; imageIndex < imageCount; imageIndex++)
	{
		setImageLayout(commandBuffer, mImages[imageIndex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		colorAttachmentView.image = mImages[imageIndex];
		result = vkCreateImageView(mDevice, &colorAttachmentView, nullptr, &mImageViews[imageIndex]);
		assert(checkResult(result));
	}

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	result = vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(checkResult(result));
	
	result = vkQueueWaitIdle(mQueue);
	assert(checkResult(result));
	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void GraphicsContext::destroy()
{
	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyDevice(mDevice, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

void GraphicsContext::setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.srcAccessMask |= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

bool GraphicsContext::checkResult(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		std::cerr << "Vulkan error : " << resultToString(result);
		return false;
	}
	return true;
}

const char * GraphicsContext::resultToString(VkResult result)
{
#define CASE_RETURN(value) case value: return #value
	switch (result) {
	CASE_RETURN(VK_SUCCESS);
	CASE_RETURN(VK_NOT_READY);
	CASE_RETURN(VK_TIMEOUT);
	CASE_RETURN(VK_EVENT_SET);
	CASE_RETURN(VK_EVENT_RESET);
	CASE_RETURN(VK_INCOMPLETE);
	CASE_RETURN(VK_ERROR_OUT_OF_HOST_MEMORY);
	CASE_RETURN(VK_ERROR_OUT_OF_DEVICE_MEMORY);
	CASE_RETURN(VK_ERROR_INITIALIZATION_FAILED);
	CASE_RETURN(VK_ERROR_DEVICE_LOST);
	CASE_RETURN(VK_ERROR_MEMORY_MAP_FAILED);
	CASE_RETURN(VK_ERROR_LAYER_NOT_PRESENT);
	CASE_RETURN(VK_ERROR_EXTENSION_NOT_PRESENT);
	CASE_RETURN(VK_ERROR_FEATURE_NOT_PRESENT);
	CASE_RETURN(VK_ERROR_INCOMPATIBLE_DRIVER);
	CASE_RETURN(VK_ERROR_TOO_MANY_OBJECTS);
	CASE_RETURN(VK_ERROR_FORMAT_NOT_SUPPORTED);
	CASE_RETURN(VK_ERROR_SURFACE_LOST_KHR);
	CASE_RETURN(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
	CASE_RETURN(VK_SUBOPTIMAL_KHR);
	CASE_RETURN(VK_ERROR_OUT_OF_DATE_KHR);
	CASE_RETURN(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
	CASE_RETURN(VK_ERROR_VALIDATION_FAILED_EXT);
	CASE_RETURN(VK_ERROR_INVALID_SHADER_NV);
	}
	return "Unknown";
#undef CASE_RETURN
}
