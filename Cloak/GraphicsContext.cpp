#include "GraphicsContext.h"

#include "geometry.h"
#include "CloakUtils.h"

#define VK_CALL(func_call) { \
	result = func_call; \
	if(result != VK_SUCCESS) \
		std::cerr << "Error in " << __FILE__ << ":" << __LINE__ << "calling " << #func_call << "\n\treturned " << resultToString(result) << '\n'; \
}

static VkBool32 debugCallback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	U64 obj,
	size_t location,
	U32 code,
	const char *layerPrefix,
	const char *msg,
	void *userData)
{
	std::cerr << "validation layer: " << msg << std::endl;
	return VK_FALSE;
}

GraphicsContext::GraphicsContext() : mFrameCount(0)
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::init(HINSTANCE hinstance, HWND hwnd)
{
	createInstance();
	setupDebugCallback();
	selectPhysicalDevice();
	createLogicalDevice();
	createSurface(hinstance, hwnd);
	createSwapchain();
	createImageViews();
	createDepthResources();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createVertexBuffer();
	createUniformBuffer();
	createDescriptorPool();
	createDescriptorSet();
	createIndexBuffer();
	createCommandBuffers();
	createSemaphores();
}

void GraphicsContext::createInstance()
{
	VkResult result = VK_SUCCESS;

	U32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "Available extensions:" << std::endl;
	for (VkExtensionProperties &extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}
	
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = nullptr;
	
	const std::vector<const char *> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
	const std::vector<const char *> enabledExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
	if (gEnableValidationLayers)
	{
		assert(checkValidationLayerSupport(validationLayers));
		instanceInfo.enabledLayerCount = validationLayers.size();
		instanceInfo.ppEnabledLayerNames = validationLayers.data();
		instanceInfo.enabledExtensionCount = enabledExtensions.size();
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}

	result = vkCreateInstance(&instanceInfo, nullptr, &mInstance);
	assert(checkResult(result));
}

void GraphicsContext::setupDebugCallback()
{
	if (!gEnableValidationLayers)
	{
		return;
	}
	VkResult result = VK_SUCCESS;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

	auto pfnCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT");
	result = pfnCreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr, &mDebugCallback);
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

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
	result = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);
	assert(checkResult(result));
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

	VkCommandBuffer commandBuffer = beginSingleUseCommandBuffer();
	mImageViews.resize(imageCount);
	for (uint32_t imageIndex = 0; imageIndex < imageCount; imageIndex++)
	{
		setImageLayout(commandBuffer, images[imageIndex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		createImageView(images[imageIndex], mSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, &mImageViews[imageIndex]);
	}
	endSingleUseCommandBuffer(commandBuffer);

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

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = mDepthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;
	subPass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	result = vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass);
	assert(checkResult(result));
}

void GraphicsContext::createDescriptorSetLayout()
{
	VkResult result = VK_SUCCESS;

	VkDescriptorSetLayoutBinding perFrameLayoutBinding = {};
	perFrameLayoutBinding.binding = 0;
	perFrameLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	perFrameLayoutBinding.descriptorCount = 1;
	perFrameLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	perFrameLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding perObjectLayoutBinding = {};
	perObjectLayoutBinding.binding = 1;
	perObjectLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	perObjectLayoutBinding.descriptorCount = 1;
	perObjectLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	perObjectLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding animationLayoutBinding = {};
	animationLayoutBinding.binding = 2;
	animationLayoutBinding.descriptorCount = 1;
	animationLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	animationLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	animationLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 3;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 4> bindings = { perFrameLayoutBinding, perObjectLayoutBinding, animationLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();
	
	result = vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout);
	assert(checkResult(result));
}

void GraphicsContext::createGraphicsPipeline()
{
	VkResult result = VK_SUCCESS;

	std::vector<char> vertShaderBytes = CloakUtils::readFile("../data/shaders/vert.spv");
	std::vector<char> fragShaderBytes = CloakUtils::readFile("../data/shaders/frag.spv");

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

	VkVertexInputBindingDescription bindingDescription = AnimatedMeshVertex::getBindingDescription();
	auto attributeDescriptions = AnimatedMeshVertex::getAttributeDescriptions();

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
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

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

	VkDescriptorSetLayout setLayouts[] = { mDescriptorSetLayout };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = setLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout);
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
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
	assert(checkResult(result));

}

void GraphicsContext::createDepthResources()
{
	const std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	mDepthFormat = findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	createImage(mSwapchainExtent.width, mSwapchainExtent.height, mDepthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&mDepthImage, &mDepthImageMemory);
	createImageView(mDepthImage, mDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &mDepthImageView);

	VkCommandBuffer commandBuffer = beginSingleUseCommandBuffer();
	setImageLayout(commandBuffer, mDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	endSingleUseCommandBuffer(commandBuffer);
}

void GraphicsContext::createFramebuffers()
{
	VkResult result = VK_SUCCESS;

	mSwapchainFramebuffers.resize(mImageViews.size());
	for (size_t i = 0; i < mSwapchainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments = { mImageViews[i], mDepthImageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = mSwapchainExtent.width;
		framebufferInfo.height = mSwapchainExtent.height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]);
		assert(checkResult(result));
	}
}

void GraphicsContext::createImageFromSurface(SDL_Surface *pSurface, VkImage *pImageOut, VkDeviceMemory *pImageMemoryOut)
{
	VkResult result = VK_SUCCESS;

	SDL_Surface *pImageSurface = SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_ABGR8888, 0);
	U32 imageSize = pImageSurface->w * pImageSurface->h * pImageSurface->format->BytesPerPixel;

	VkImage stagingImage;
	VkDeviceMemory stagingImageMemory;
	createImage(pImageSurface->w, pImageSurface->h, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingImage, &stagingImageMemory);

	void *data;
	vkMapMemory(mDevice, stagingImageMemory, 0, imageSize, 0, &data);
	memcpy(data, pImageSurface->pixels, imageSize);
	vkUnmapMemory(mDevice, stagingImageMemory);

	createImage(pImageSurface->w, pImageSurface->h, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		pImageOut, pImageMemoryOut);

	VkCommandBuffer commandBuffer = beginSingleUseCommandBuffer();
	setImageLayout(commandBuffer, stagingImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	setImageLayout(commandBuffer, *pImageOut, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyImage(commandBuffer, stagingImage, *pImageOut, pImageSurface->w, pImageSurface->h);
	setImageLayout(commandBuffer, *pImageOut, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	endSingleUseCommandBuffer(commandBuffer);

	vkFreeMemory(mDevice, stagingImageMemory, nullptr);
	vkDestroyImage(mDevice, stagingImage, nullptr);

	SDL_FreeSurface(pImageSurface);
	pImageSurface = nullptr;
}

void GraphicsContext::createTextureImage()
{
	VkResult result = VK_SUCCESS;

	SDL_Surface *surface = IMG_Load("../data/textures/guard1_body.tga");
	
	createImageFromSurface(surface, &mTextureImage, &mTextureImageMemory);

	SDL_FreeSurface(surface);
}

void GraphicsContext::createTextureImageView()
{
	createImageView(mTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &mTextureImageView);
}

void GraphicsContext::createTextureSampler()
{
	VkResult result = VK_SUCCESS;

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	result = vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTextureSampler);
	assert(checkResult(result));
}

void GraphicsContext::createBufferFromData(void *pData, U32 bufferSize, VkBufferUsageFlags usage, VkBuffer *pBufferOut, VkDeviceMemory *pBufferMemoryOut)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	void *pStagingData;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &pStagingData);
	memcpy(pStagingData, pData, bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pBufferOut, pBufferMemoryOut);

	copyBuffer(stagingBuffer, *pBufferOut, bufferSize);

	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
}

void GraphicsContext::createVertexBuffer()
{
	U32 bufferSize = sizeof(gDemoVertices[0]) * gDemoVertices.size();

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
	U32 bufferSize = sizeof(gDemoIndices[0]) * gDemoIndices.size();

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

void GraphicsContext::createUniformBuffer()
{
	//make the staging buffer the size of the largest constant buffer for now so it can be used for any buffer
	createBuffer(sizeof(AnimationConstantBuffer), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&mUniformStagingBuffer, &mUniformStagingBufferMemory);
	createBuffer(sizeof(FrameConstantBuffer), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&mUniformBuffer, &mUniformBufferMemory);
}

void GraphicsContext::createDescriptorPool()
{
	VkResult result = VK_SUCCESS;

	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 384;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 128;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 128;

	result = vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool);
	assert(checkResult(result));
}

void GraphicsContext::createDescriptorSet()
{
	VkResult result = VK_SUCCESS;

	VkDescriptorSetLayout layouts[] = { mDescriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	result = vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet);
	assert(checkResult(result));

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = mUniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(FrameConstantBuffer);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = mTextureImageView;
	imageInfo.sampler = mTextureSampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = mDescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;
	descriptorWrites[0].pImageInfo = nullptr;
	descriptorWrites[0].pTexelBufferView = nullptr;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = mDescriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = nullptr;
	descriptorWrites[1].pImageInfo = &imageInfo;
	descriptorWrites[1].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(mDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
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

void GraphicsContext::createCommandBuffer(AnimatedMesh *animatedMesh)
{
	VkResult result = VK_SUCCESS;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);
	assert(checkResult(result));
	
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = mRenderPass;
	inheritanceInfo.subpass = 0;
	inheritanceInfo.framebuffer = VK_NULL_HANDLE;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	ObjectConstantBuffer objectBuffer = {};
	objectBuffer.modelMatrix = animatedMesh->getModelMatrix();
	createBufferFromData(&objectBuffer, sizeof(objectBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		&animatedMesh->mObjectConstantBuffer, &animatedMesh->mObjectConstantBufferMemory);

	auto &boneMatrices = animatedMesh->getBoneMatrices();
	createBufferFromData(boneMatrices.data(), sizeof(boneMatrices[0]) * boneMatrices.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		&animatedMesh->mAnimationConstantBuffer, &animatedMesh->mAnimationConstantBufferMemory);
	
	for (AnimatedSubMesh &subMesh : animatedMesh->getSubMeshes())
	{
		//Create resources in GPU memory
		createBufferFromData(subMesh.vertices.data(), sizeof(subMesh.vertices[0]) * subMesh.vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			&subMesh.vertexBuffer, &subMesh.vertexBufferMemory);
		
		createBufferFromData(subMesh.indices.data(), sizeof(subMesh.indices[0]) * subMesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			&subMesh.indexBuffer, &subMesh.indexBufferMemory);
		
		SDL_Surface *pImageSurface = IMG_Load(subMesh.textureName.c_str());
		createImageFromSurface(pImageSurface, &subMesh.textureImage, &subMesh.textureImageMemory);
		SDL_FreeSurface(pImageSurface);
		createImageView(subMesh.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &subMesh.textureImageView);
		
		//Create descriptor set
		VkDescriptorSetLayout layouts[] = { mDescriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		result = vkAllocateDescriptorSets(mDevice, &allocInfo, &subMesh.descriptorSet);
		assert(checkResult(result));

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = mUniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(FrameConstantBuffer);

		VkDescriptorBufferInfo objectBufferInfo = {};
		objectBufferInfo.buffer = animatedMesh->mObjectConstantBuffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(ObjectConstantBuffer);

		VkDescriptorBufferInfo animationBufferInfo = {};
		animationBufferInfo.buffer = animatedMesh->mAnimationConstantBuffer;
		animationBufferInfo.offset = 0;
		animationBufferInfo.range = sizeof(AnimationConstantBuffer);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = subMesh.textureImageView;
		imageInfo.sampler = mTextureSampler;

		std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = subMesh.descriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = subMesh.descriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &objectBufferInfo;
		descriptorWrites[1].pImageInfo = nullptr;
		descriptorWrites[1].pTexelBufferView = nullptr;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = subMesh.descriptorSet;
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &animationBufferInfo;
		descriptorWrites[2].pImageInfo = nullptr;
		descriptorWrites[2].pTexelBufferView = nullptr;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = subMesh.descriptorSet;
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pBufferInfo = nullptr;
		descriptorWrites[3].pImageInfo = &imageInfo;
		descriptorWrites[3].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(mDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		//Record commands
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
		VkBuffer vertexBuffers[] = { subMesh.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, subMesh.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &subMesh.descriptorSet, 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, subMesh.indices.size(), 1, 0, 0, 0);
	}

	vkEndCommandBuffer(commandBuffer);

	mSecondaryCommandBuffers.push_back(commandBuffer);
}

void GraphicsContext::updateConstantBuffer(const void * pData, U32 bufferSize, VkBuffer buffer)
{
	void *data;
	vkMapMemory(mDevice, mUniformStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, pData, bufferSize);
	vkUnmapMemory(mDevice, mUniformStagingBufferMemory);
	copyBuffer(mUniformStagingBuffer, buffer, bufferSize);
}

void GraphicsContext::updatePerFrameConstantBuffer(const FrameConstantBuffer &perFrameCB)
{
	updateConstantBuffer(&perFrameCB, sizeof(perFrameCB), mUniformBuffer);
}

void GraphicsContext::drawFrame()
{
	VkResult result = VK_SUCCESS;
	
	U32 imageIndex;
	vkAcquireNextImageKHR(mDevice, mSwapchain, std::numeric_limits<U64>::max(), imageAcquiredSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	const VkCommandBuffer commandBuffer = mCommandBuffers[imageIndex];
	result = vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	assert(checkResult(result));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	assert(checkResult(result));

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mRenderPass;
	renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = mSwapchainExtent;

	std::array<VkClearValue, 2> clearColors = {};
	clearColors[0].color = { 0.f, 0.f, 0.f, 1.f };
	clearColors[1].depthStencil = { 1.f, 0 };
	renderPassInfo.clearValueCount = clearColors.size();
	renderPassInfo.pClearValues = clearColors.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
		/*
		VkBuffer vertexBuffers[] = { mVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, gDemoIndices.size(), 1, 0, 0, 0);
		*/
		vkCmdExecuteCommands(commandBuffer, mSecondaryCommandBuffers.size(), mSecondaryCommandBuffers.data());
	}
	vkCmdEndRenderPass(commandBuffer);

	result = vkEndCommandBuffer(commandBuffer);
	assert(checkResult(result));

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

bool GraphicsContext::checkValidationLayerSupport(const std::vector<const char *> &validationLayers)
{
	U32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const VkLayerProperties& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}
	return true;
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

void GraphicsContext::createImage(U32 width, U32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	VkImage *image, VkDeviceMemory *imageMemory)
{
	VkResult result = VK_SUCCESS;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;
	result = vkCreateImage(mDevice, &imageInfo, nullptr, image);
	assert(checkResult(result));

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(mDevice, *image, &memReq);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

	result = vkAllocateMemory(mDevice, &allocInfo, nullptr, imageMemory);
	assert(checkResult(result));

	vkBindImageMemory(mDevice, *image, *imageMemory, 0);
}

void GraphicsContext::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *pImageViewOut)
{
	VkResult result = VK_SUCCESS;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
	};

	result = vkCreateImageView(mDevice, &viewInfo, nullptr, pImageViewOut);
	assert(checkResult(result));
}

void GraphicsContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleUseCommandBuffer();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleUseCommandBuffer(commandBuffer);
}

void GraphicsContext::copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, U32 width, U32 height)
{
	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;

	VkImageCopy copyRegion = {};
	copyRegion.srcSubresource = subResource;
	copyRegion.dstSubresource = subResource;
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent.width = width;
	copyRegion.extent.height = height;
	copyRegion.extent.depth = 1;

	vkCmdCopyImage(commandBuffer,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &copyRegion);	
}

void GraphicsContext::destroy()
{
	vkDeviceWaitIdle(mDevice);

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyDevice(mDevice, nullptr);

	auto pfnDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance,
		"vkDestroyDebugReportCallbackEXT");
	pfnDestroyDebugReportCallbackEXT(mInstance, mDebugCallback, nullptr);

	vkDestroyInstance(mInstance, nullptr);
}

void GraphicsContext::setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

VkCommandBuffer GraphicsContext::beginSingleUseCommandBuffer()
{
	VkResult result = VK_SUCCESS;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);
	assert(checkResult(result));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	assert(checkResult(result));

	return commandBuffer;
}

void GraphicsContext::endSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkResult result = VK_SUCCESS;

	result = vkEndCommandBuffer(commandBuffer);
	assert(checkResult(result));

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

VkFormat GraphicsContext::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);
		if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			|| (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features))
		{
			return format;
		}
	}
	assert(true);
	return VK_FORMAT_UNDEFINED;
}

bool GraphicsContext::checkResult(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		std::cerr << "Vulkan error : " << resultToString(result) << std::endl;
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
