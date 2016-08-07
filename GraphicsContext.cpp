#include "GraphicsContext.h"

#include "geometry.h"
#include "OcelotUtils.h"

#define VK_CALL(func_call) { \
	result = func_call; \
	if(result != VK_SUCCESS) \
		std::cerr << "Error in " << __FILE__ << ":" << __LINE__ << "calling " << #func_call << "\n\treturned " << resultToString(result) << '\n'; \
}

GraphicsContext::GraphicsContext() : frameCount(0)
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::init(HINSTANCE hinstance, HWND hwnd)
{
	createInstance();
	selectPhysicalDevice();
	createLogicalDevice();
	createSurface(hinstance, hwnd);
	createSwapchain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createVertexBuffer();
	createIndexBuffer();
	createCommandBuffers();
	createSemaphores();
}

void GraphicsContext::createInstance()
{
	VkResult result = VK_SUCCESS;

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = nullptr;

	result = vkCreateInstance(&instanceInfo, nullptr, &mInstance);
	assert(checkResult(result));
}

void GraphicsContext::selectPhysicalDevice()
{
	VkResult result = VK_SUCCESS;

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
}

void GraphicsContext::createLogicalDevice()
{
	VkResult result = VK_SUCCESS;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, &queueFamilyProperties[0]);
	mQueueFamilyIndex = 0;
	std::cout << "Defaulting to queue family " << mQueueFamilyIndex << std::endl
		<< "\tqueue count: " << queueFamilyProperties[mQueueFamilyIndex].queueCount << std::endl
		<< "\tsupported queue operations:" << std::endl;
	VkQueueFlags queueFlags = queueFamilyProperties[mQueueFamilyIndex].queueFlags;
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
	deviceQueueInfo.queueFamilyIndex = mQueueFamilyIndex;
	float queuePriorities[] = { 1.f };
	deviceQueueInfo.pQueuePriorities = queuePriorities;
	deviceQueueInfo.queueCount = 1;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &deviceQueueInfo;
	result = vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice);
	assert(checkResult(result));

	vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 0, &mQueue);
}

void GraphicsContext::createSurface(HINSTANCE hinstance, HWND hwnd)
{
	VkResult result = VK_SUCCESS;

	VkWin32SurfaceCreateInfoKHR surfaceInfo;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = hinstance;
	surfaceInfo.hwnd = hwnd;
	result = vkCreateWin32SurfaceKHR(mInstance, &surfaceInfo, nullptr, &mSurface);
	assert(checkResult(result));

	VkBool32 isSupported = false;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, mQueueFamilyIndex, mSurface, &isSupported);
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
}

void GraphicsContext::createSwapchain()
{
	VkResult result = VK_SUCCESS;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);
	assert(checkResult(result));
	uint32_t desiredImages = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0
		&& desiredImages > surfaceCapabilities.maxImageCount) {
		desiredImages = surfaceCapabilities.maxImageCount;
	}
	mSwapchainExtent = surfaceCapabilities.currentExtent;

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = mSurface;
	swapchainInfo.minImageCount = desiredImages;
	swapchainInfo.imageFormat = mSurfaceFormat.format;
	swapchainInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainInfo.imageExtent = mSwapchainExtent;
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
}

void GraphicsContext::createImageViews()
{
	VkResult result = VK_SUCCESS;

	uint32_t imageCount;
	result = vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	assert(checkResult(result));
	std::vector<VkImage> images(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, &images[0]);

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
	commandPoolCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
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
		setImageLayout(commandBuffer, images[imageIndex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		colorAttachmentView.image = images[imageIndex];
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

void GraphicsContext::createRenderPass()
{
	VkResult result = VK_SUCCESS;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = mSurfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	result = vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass);
	assert(checkResult(result));
}

void GraphicsContext::createGraphicsPipeline()
{
	VkResult result = VK_SUCCESS;

	std::vector<char> vertShaderBytes = OcelotUtils::readFile("../data/shaders/vert.spv");
	std::vector<char> fragShaderBytes = OcelotUtils::readFile("../data/shaders/frag.spv");

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	createShaderModule(vertShaderBytes, &vertShaderModule);
	createShaderModule(fragShaderBytes, &fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkVertexInputBindingDescription bindingDescription = ScreenVertex::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = ScreenVertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapchainExtent.width;
	viewport.height = (float)mSwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
	assert(checkResult(result));

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
	assert(checkResult(result));

}

void GraphicsContext::createFramebuffers()
{
	VkResult result = VK_SUCCESS;

	mSwapchainFramebuffers.resize(mImageViews.size());
	for (size_t i = 0; i < mSwapchainFramebuffers.size(); i++)
	{
		VkImageView attachments[] = { mImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mSwapchainExtent.width;
		framebufferInfo.height = mSwapchainExtent.height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]);
		assert(checkResult(result));
	}
}

void GraphicsContext::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(gDemoVertices[0]) * gDemoVertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	void *data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, gDemoVertices.data(), bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mVertexBuffer, &mVertexBufferMemory);

	copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
}

void GraphicsContext::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(gDemoIndices[0]) * gDemoIndices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	void *data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, gDemoIndices.data(), bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mIndexBuffer, &mIndexBufferMemory);

	copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
}

void GraphicsContext::createCommandBuffers()
{
	VkResult result = VK_SUCCESS;

	mCommandBuffers.resize(mSwapchainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (U32)mCommandBuffers.size();

	result = vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data());
	assert(checkResult(result));

	for (size_t i = 0; i < mCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapchainExtent;

		VkClearValue clearColor = { 0.f, 0.f, 0.f, 1.f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
			
			VkBuffer vertexBuffers[] = { mVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(mCommandBuffers[i], mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(mCommandBuffers[i], gDemoIndices.size(), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(mCommandBuffers[i]);

		result = vkEndCommandBuffer(mCommandBuffers[i]);
		assert(checkResult(result));
	}

}

void GraphicsContext::createSemaphores()
{
	VkResult result = VK_SUCCESS;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	result = vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &imageAcquiredSemaphore);
	assert(checkResult(result));
	result = vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
	assert(checkResult(result));
}

void GraphicsContext::drawFrame()
{
	VkResult result = VK_SUCCESS;
	U32 imageIndex;
	vkAcquireNextImageKHR(mDevice, mSwapchain, std::numeric_limits<U64>::max(), imageAcquiredSemaphore, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAcquiredSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	result = vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(checkResult(result));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapchain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(mQueue, &presentInfo);
	assert(checkResult(result));
}

void GraphicsContext::createShaderModule(const std::vector<char>& code, VkShaderModule *pShaderModule)
{
	VkResult result = VK_SUCCESS;

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (U32*)code.data();

	
	result = vkCreateShaderModule(mDevice, &createInfo, nullptr, pShaderModule);
	assert(checkResult(result));
}

void GraphicsContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBufferOut, VkDeviceMemory *pBufferMemoryOut)
{
	VkResult result = VK_SUCCESS;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateBuffer(mDevice, &bufferInfo, nullptr, pBufferOut);
	assert(checkResult(result));

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(mDevice, *pBufferOut, &memReq);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

	//This is bad practice. We should be allocating a large block up-front and managing our own suballocations
	result = vkAllocateMemory(mDevice, &allocInfo, nullptr, pBufferMemoryOut);
	assert(checkResult(result));

	vkBindBufferMemory(mDevice, *pBufferOut, *pBufferMemoryOut, 0);
}

void GraphicsContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mQueue);

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void GraphicsContext::destroy()
{
	vkDeviceWaitIdle(mDevice);

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

U32 GraphicsContext::findMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProps);
	for (U32 i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties))
		{
			return i;
		}
	}
	assert(false);
	return 0;
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
