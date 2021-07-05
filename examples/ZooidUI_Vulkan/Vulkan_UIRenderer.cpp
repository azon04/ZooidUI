#include "Vulkan_UIRenderer.h"

#include "UI/ZooidUI.h"
#include "Utils/Timer.h"
#include "Shader.h"

#include <set>
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <assert.h>
#include <algorithm>

#if WIN32 || WIN64
extern "C"
{
#include <windows.h>
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

#define MAX_FRAMES_IN_FLIGHT 2

namespace ZE
{
	namespace VulkanFunctions
	{
#define VULKAN_DECLARE_FUNCTION(func) PFN_##func func;
		VULKAN_DECLARE_FUNCTION(vkCmdSetDepthTestEnableEXT)
		VULKAN_DECLARE_FUNCTION(vkCmdSetStencilOpEXT)
		VULKAN_DECLARE_FUNCTION(vkCmdSetStencilTestEnableEXT)
#undef VULKAN_DECLARE_FUNCTION

		void InitInstanceFunctions(VkInstance instance)
		{
#define VULKAN_INIT_INSTANCE_FUNCTION(func) func = (PFN_##func)vkGetInstanceProcAddr(instance, #func); \
			if(!func) { std::cout << "Failed to init function: " << #func << std::endl; __debugbreak(); }

			VULKAN_INIT_INSTANCE_FUNCTION(vkCmdSetDepthTestEnableEXT)
			VULKAN_INIT_INSTANCE_FUNCTION(vkCmdSetStencilOpEXT)
			VULKAN_INIT_INSTANCE_FUNCTION(vkCmdSetStencilTestEnableEXT)
#undef  VULKAN_INIT_INSTANCE_FUNCTION
		}
	}

	Vulkan_UIRenderer* gVulkanRenderer = nullptr;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	Vulkan_UIRenderer::~Vulkan_UIRenderer() {}

#if VULKAN_VALIDATION_LAYER
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		bool bFound = false;
		for (const char* layerName : validationLayers)
		{
			for (const VkLayerProperties& layerProperty : availableLayers)
			{
				if (strcmp(layerProperty.layerName, layerName) == 0)
				{
					bFound = true;
					break;
				}
			}
			if (bFound) break;
		}
		return bFound;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::cout << "Validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
#endif

	std::vector<const char*> GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if VULKAN_VALIDATION_LAYER
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		return extensions;
	}

	void Vulkan_UIRenderer::createVulkanInstance()
	{
#if VULKAN_VALIDATION_LAYER
		ASSERT(checkValidationLayerSupport(), "Validation Layers requested, but not available");
#endif
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "ZooidUI Vulkan Sample";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

#if VULKAN_VALIDATION_LAYER
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
#endif
		VK_CHECK_SUCCESS(vkCreateInstance(&createInfo, nullptr, &m_instance), "Failed to create VKInstance!");
	}

	struct QueueFamilyIndices {
		uint32_t graphicsFamily = 0xFFFFFFFF;
		uint32_t presentFamily = 0xFFFFFFFF;

		bool isComplete() {
			return graphicsFamily != 0xFFFFFFFF && presentFamily != 0xFFFFFFFF;
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (unsigned i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if(indices.isComplete())
				break;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const VkExtensionProperties& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount > 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount > 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		VkPhysicalDeviceProperties2 deviceProperties{};
		vkGetPhysicalDeviceProperties2(device, &deviceProperties);

		QueueFamilyIndices indices = findQueueFamilies(device, surface);
		bool extensionSupported = checkDeviceExtensionSupport(device);
		
		bool swapChainAdequate = false;
		if (extensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionSupported && swapChainAdequate;
	}

	void Vulkan_UIRenderer::pickVulkanPhysicalDevice()
	{
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		FATALASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		bool bFound = false;
		for (const VkPhysicalDevice& device : devices)
		{
			if (isDeviceSuitable(device, m_surface))
			{
				m_physicalDevice = device;
				bFound = true;
				break;
			}
		}

		FATALASSERT(bFound, "Couldn't find a suitable device!");
	}

	void Vulkan_UIRenderer::createVulkanLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicFeature{};
		extendedDynamicFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
		extendedDynamicFeature.pNext = nullptr;
		extendedDynamicFeature.extendedDynamicState = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = nullptr;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
#if VULKAN_VALIDATION_LAYER
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif
		createInfo.pNext = &extendedDynamicFeature;

		VK_CHECK_SUCCESS(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device), "Failed to create Vulkan Device!");

		vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void Vulkan_UIRenderer::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		
		VkExtent2D extent;
		if (swapChainSupport.capabilities.currentExtent.width != UINT32_MAX)
		{
			extent = swapChainSupport.capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, std::min(swapChainSupport.capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, std::min(swapChainSupport.capabilities.maxImageExtent.height, actualExtent.height));

			extent = actualExtent;
		}

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);
		uint32_t queueFamilieIndices[] = { indices.graphicsFamily, indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilieIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK_SUCCESS(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain), "Failed to create swapchain!");

		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;
	}

	uint32_t Vulkan_UIRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (i << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		FATALASSERT(true, "Couldn't find the requested memory type!");
		return 0;
	}

	void Vulkan_UIRenderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryAllocateFlags properties, 
		VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			std::cout << "Failed tot cerate image!" << std::endl;
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		VK_CHECK_SUCCESS(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory), "Failed to allocate image memory!");

		vkBindImageMemory(m_device, image, imageMemory, 0);
	}

	VkImageView Vulkan_UIRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) 
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VK_CHECK_SUCCESS(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView), "Failed to create ImageView!");

		return imageView;
	}

	void Vulkan_UIRenderer::createSwapChainImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			m_swapChainImageViews[i] = createImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	VkFormat Vulkan_UIRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props{};
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		FATALASSERT(true, "failed to find supported format!");
	}

	VkFormat Vulkan_UIRenderer::findDepthFormat() { return findSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); }

	void Vulkan_UIRenderer::createRenderPass() 
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = nullptr;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

// 		VkSubpassDependency dependency{};
// 		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
// 		dependency.dstSubpass = 0;
// 		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
// 		dependency.srcAccessMask = 0;
// 		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
// 		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = nullptr;

		VK_CHECK_SUCCESS(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass), "Failed to create render pass!");
	}

	void Vulkan_UIRenderer::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_SUCCESS(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool), "Failed to create command pool!");
	}

	void Vulkan_UIRenderer::createDepthResource()
	{
		VkFormat depthFormat = findDepthFormat();
		createImage(m_swapChainExtent.width, m_swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
		m_depthImageView = createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		transitionImageLayout(m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	void Vulkan_UIRenderer::createFrameBuffers()
	{
		m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());
		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			std::array<VkImageView, 2> attachments = 
			{
				m_swapChainImageViews[i],
				m_depthImageView
			};

			VkFramebufferCreateInfo frameBufferInfo{};
			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.renderPass = m_renderPass;
			frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferInfo.pAttachments = attachments.data();
			frameBufferInfo.width = m_swapChainExtent.width;
			frameBufferInfo.height = m_swapChainExtent.height;
			frameBufferInfo.layers = 1;

			VK_CHECK_SUCCESS(vkCreateFramebuffer(m_device, &frameBufferInfo, nullptr, &m_swapChainFrameBuffers[i]), "Failed to create a framebuffer!");
		}
	}

	void Vulkan_UIRenderer::createDescriptorPools()
	{
		m_descriptorPools.resize(m_swapChainImageViews.size());
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1024;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1024;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 512;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		for (size_t i = 0; i < m_descriptorPools.size(); i++)
		{
			VK_CHECK_SUCCESS(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, m_descriptorPools.data() + i), "Failed to create a descriptor pool!");
		}
	}

	void Vulkan_UIRenderer::createSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 0;
		samplerInfo.mipLodBias = 0.0f;

		VK_CHECK_SUCCESS(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler), "Failed to create a sampler!");
	}

	void Vulkan_UIRenderer::createCommandBuffers()
	{
		m_commandBuffers.resize(m_swapChainImageViews.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		VK_CHECK_SUCCESS(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()), "Failed to create command buffers!");
	}

	void Vulkan_UIRenderer::createSyncObjects()
	{
		m_imageAvailSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(m_swapChainImageViews.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
			{
				FATALASSERT(false,"Failed to create sync objects!");
			}
		}
	}

	void Vulkan_UIRenderer::recreateSwapChain()
	{
		vkDeviceWaitIdle(m_device);

		cleanupSwapChain();

		{
			SharedUniformBuffer::SharedUniformBufferData& bufferData = m_globalUniformBuffer.getBufferData();
			bufferData.screenDimension = UIVector2((Float32)m_width, (Float32)m_height);
			m_globalUniformBuffer.flush();
		}

		createSwapChain();
		createSwapChainImageViews();
		createDepthResource();
		createFrameBuffers();
	}

	void Vulkan_UIRenderer::cleanupSwapChain()
	{
		for (size_t i = 0; i < m_swapChainFrameBuffers.size(); ++i)
		{
			vkDestroyFramebuffer(m_device, m_swapChainFrameBuffers[i], nullptr);
		}

		vkDestroyImageView(m_device, m_depthImageView, nullptr);
		vkDestroyImage(m_device, m_depthImage, nullptr);
		vkFreeMemory(m_device, m_depthImageMemory, nullptr);

		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	}


	void Vulkan_UIRenderer::beginRender()
	{
		vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentSwapChainIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			FATALASSERT(false, "Failed to acquire swap chain images!");
		}

		// Check if the previous frame is using this image (there is its fence to wait on)
		if (m_imagesInFlight[m_currentSwapChainIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_device, 1, &m_imagesInFlight[m_currentSwapChainIndex], VK_TRUE, UINT64_MAX);
		}

		// Make the image as now being used in this frame
		m_imagesInFlight[m_currentSwapChainIndex] = m_inFlightFences[m_currentFrame];

		VK_CHECK_SUCCESS(vkResetDescriptorPool(m_device, m_descriptorPools[m_currentSwapChainIndex], 0), "Failed to reset descriptor pool!");

		VK_CHECK_SUCCESS(vkResetCommandBuffer(m_commandBuffers[m_currentSwapChainIndex], 0), "Failed to reset command buffer");

		// Begin command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;
		VK_CHECK_SUCCESS(vkBeginCommandBuffer(m_commandBuffers[m_currentSwapChainIndex], &beginInfo), "Failed to begin command buffer!");

		// Begin Render Pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_swapChainFrameBuffers[m_currentSwapChainIndex];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = m_swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffers[m_currentSwapChainIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_currentBufferIndex = 0;
		m_currentVertexUBOIndex = 0;
		m_currentFragmentUBOIndex = 0;
		maskCount = 0;
		m_currentShader = nullptr;
	}

	void Vulkan_UIRenderer::endRender()
	{
		vkCmdEndRenderPass(m_commandBuffers[m_currentSwapChainIndex]);

		VK_CHECK_SUCCESS(vkEndCommandBuffer(m_commandBuffers[m_currentSwapChainIndex]), "Failed to end command buffer!");

		// Submit the command buffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_imageAvailSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[m_currentSwapChainIndex];

		VkSemaphore signalSemaphore[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphore;

		vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

		VK_CHECK_SUCCESS(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]), "Failed to submit the command buffer!");
	}

	void Vulkan_UIRenderer::present()
	{
		VkSemaphore signalSemaphore[] = { m_renderFinishedSemaphores[m_currentFrame] };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphore;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapChain;
		presentInfo.pImageIndices = &m_currentSwapChainIndex;

		VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			// TODO
			__debugbreak();
		}
		else if (result != VK_SUCCESS)
		{
			FATALASSERT(false,"Failed to present the swapchain!");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	VkCommandBuffer Vulkan_UIRenderer::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Vulkan_UIRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
	}

	void Vulkan_UIRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			std::cout << "Unsupported Layout!";
			endSingleTimeCommands(commandBuffer);
			return;
		}

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}

	void Vulkan_UIRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
		{
			std::cout << "Failed to create buffer!" << std::endl;
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			std::cout << "Failed to allocate buffer memory!" << std::endl;
			return;
		}

		vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
	}

	void Vulkan_UIRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	void Vulkan_UIRenderer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, UInt32 width, UInt32 height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}

	ZE::VertexBuffer* Vulkan_UIRenderer::getOrCreateNextVertexBuffer(VkDeviceSize size)
	{
		std::vector<VertexBuffer>& currentBuffers = m_buffers[m_currentSwapChainIndex];
		if (m_currentBufferIndex >= currentBuffers.size())
		{
			VertexBuffer newBuffer;
			currentBuffers.push_back(newBuffer);
		}

		if (currentBuffers[m_currentBufferIndex].getSize() < size)
		{
			currentBuffers[m_currentBufferIndex].Init(size);
		}

		return &currentBuffers[m_currentBufferIndex++];
	}

	void Vulkan_UIRenderer::destroyAllInstanceBuffers()
	{
		for (std::vector<VertexBuffer>& currentBuffers : m_buffers)
		{
			for (VertexBuffer& buffer : currentBuffers)
			{
				buffer.Destroy();
			}
		}
		m_buffers.clear();
	}

	VkDescriptorSet Vulkan_UIRenderer::createDescriptorSet(VkDescriptorSetLayout layout)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPools[m_currentSwapChainIndex];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet newSet;
		VK_CHECK_SUCCESS(vkAllocateDescriptorSets(m_device, &allocInfo, &newSet), "Failed to create a descriptor set");

		return newSet;
	}

	ZE::VertexUniformBuffer* Vulkan_UIRenderer::getOrCreateNextVertexUniformBuffer()
	{
		std::vector<VertexUniformBuffer>& currentBuffers = m_vertexUniformBuffersMap[m_currentSwapChainIndex];
		if (m_currentVertexUBOIndex < currentBuffers.size())
		{
			return &currentBuffers[m_currentVertexUBOIndex++];
		}

		VertexUniformBuffer newBuffer;
		newBuffer.Init();
		currentBuffers.push_back(newBuffer);
		return &currentBuffers[m_currentVertexUBOIndex++];
	}

	ZE::FragmentUniformBuffer* Vulkan_UIRenderer::getOrCreateNextFragmentUniformBuffer()
	{
		std::vector<FragmentUniformBuffer>& currentBuffers = m_fragmentUniformBuffersMap[m_currentSwapChainIndex];
		if (m_currentFragmentUBOIndex < currentBuffers.size())
		{
			return &currentBuffers[m_currentFragmentUBOIndex++];
		}

		FragmentUniformBuffer newBuffer;
		newBuffer.Init();
		currentBuffers.push_back(newBuffer);
		return &currentBuffers[m_currentFragmentUBOIndex++];
	}

	void Vulkan_UIRenderer::BindShader(VkCommandBuffer commandBuffer, Shader* shader)
	{
		if (m_currentShader != shader)
		{
			m_currentShader = shader;
			m_currentShader->Use(commandBuffer);

			VkViewport viewPort{};
			viewPort.x = 0.0f;
			viewPort.y = (float)m_swapChainExtent.height;
			viewPort.width = (float)m_swapChainExtent.width;
			viewPort.height = -1.0f * (float)m_swapChainExtent.height;
			viewPort.minDepth = 0.0f;
			viewPort.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_swapChainExtent;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		}
	}

	void Vulkan_UIRenderer::Init(int width, int height)
	{
		gVulkanRenderer = this;

		// Init Library
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Create Window
		m_window = glfwCreateWindow(width, height, "Zooid UI Sample", nullptr, nullptr);
		if (m_window == nullptr)
		{
			glfwTerminate();
			std::cout << "Failed to create GLFW Window" << std::endl;
			return;
		}
		glfwSetWindowUserPointer(m_window, this);

		// Viewport setup
		glfwGetFramebufferSize(m_window, &width, &height);

		createVulkanInstance();

#if VULKAN_VALIDATION_LAYER
		// Create Debug Messenger
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		VK_CHECK_SUCCESS(CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger), "Failed to create Debug Messenger!");
#endif

		// Create surface
		VK_CHECK_SUCCESS(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface), "Failed to create Vulkan Surface!");

		VulkanFunctions::InitInstanceFunctions(m_instance);

		pickVulkanPhysicalDevice();
		createVulkanLogicalDevice();
		createSwapChain();
		createSwapChainImageViews();
		createRenderPass();
		createCommandPool();
		createDepthResource();
		createFrameBuffers();

		// Setup Shaders
		Shader::Init(m_device);
 		m_drawShader = UINEW(Shader(m_device, m_renderPass, "Shaders/BaseShapeShader.spv", "Shaders/BaseShapeShader_Color.spv", nullptr));
 		m_drawInstanceShader = UINEW(InstanceShader(m_device, m_renderPass, "Shaders/BaseShapeShader_Instance.spv", "Shaders/BaseShapeShader_Color_Instance.spv", nullptr));
 		m_drawTexShader = UINEW(Shader(m_device, m_renderPass, "Shaders/BaseShapeShader.spv", "Shaders/BaseShapeShader_Texture.spv", nullptr));
 		m_drawInstanceTexShader = UINEW(InstanceShader(m_device, m_renderPass, "Shaders/BaseShapeShader_Texture_Instance.spv", "Shaders/BaseShapeShader_Texture_Instance_Frag.spv", nullptr));
 		m_textShader = UINEW(Shader(m_device, m_renderPass, "Shaders/BaseTextShader.spv", "Shaders/BaseTextShader_Frag.spv", nullptr));
 		m_textInstanceShader = UINEW(InstanceShader(m_device, m_renderPass, "Shaders/BaseTextShader_Instance.spv", "Shaders/BaseTextShader_Instance_Frag.spv", nullptr));
 
 		// Initialize Buffer
 		
 		// Rect Buffer
		{
			UIVertex rectArray[6] = { 
								{ UIVector2{ 1.0f, 0.0f }, 0.0f, UIVector2{ 1.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } }, 
								{ UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 0.0f, 1.0f }, 0.0f, UIVector2{ 0.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } } 
								};

			VkDeviceSize bufferSize = sizeof(rectArray);
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void *data;
			vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, rectArray, bufferSize);
			vkUnmapMemory(m_device, stagingBufferMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_rectBuffer, m_rectBufferMemory);

			copyBuffer(stagingBuffer, m_rectBuffer, bufferSize);

			vkDestroyBuffer(m_device, stagingBuffer, nullptr);
			vkFreeMemory(m_device, stagingBufferMemory, nullptr);
		}
		
		// Buffers Pools
		m_buffers.resize(m_swapChainImageViews.size());

		{
			UIVertex rectTextArray[6] = { { UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
							{ UIVector2{ 1.0f, 0.0f }, 0.0f, UIVector2{ 1.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
							{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
							{ UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
							{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
							{ UIVector2{ 0.0f, 1.0f }, 0.0f, UIVector2{ 0.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } } };

			VkDeviceSize bufferSize = sizeof(rectTextArray);
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

			void *data;
			vkMapMemory(m_device, stagingMemory, 0, bufferSize, 0, &data);
			memcpy(data, rectTextArray, bufferSize);
			vkUnmapMemory(m_device, stagingMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_rectTextBuffer, m_rectTextBufferMemory);

			copyBuffer(stagingBuffer, m_rectTextBuffer, bufferSize);

			vkDestroyBuffer(m_device, stagingBuffer, nullptr);
			vkFreeMemory(m_device, stagingMemory, nullptr);
		}

 		glfwGetFramebufferSize(m_window, &width, &height);

		m_width = width;
		m_height = height;

		{
			m_globalUniformBuffer.Init();
			SharedUniformBuffer::SharedUniformBufferData& bufferData = m_globalUniformBuffer.getBufferData();
			bufferData.screenDimension = UIVector2((Float32)width, (Float32)height);
			m_globalUniformBuffer.flush();
		}

		{
			m_nullVertexUBO.Init();
			m_nullVertexUBO.flush();
		}

		{
			m_nullFragmentUBO.Init();
			m_nullFragmentUBO.flush();
		}

		m_vertexUniformBuffersMap.resize(m_swapChainImageViews.size());
		m_fragmentUniformBuffersMap.resize(m_swapChainImageViews.size());

		createDescriptorPools();
		createSampler();
		createCommandBuffers();
		createSyncObjects();
	}

	void Vulkan_UIRenderer::ProcessCurrentDrawList()
	{
 		glfwPollEvents();
		beginRender();

		UIArray<UIDrawItem*> secondPass;
		for (int i = 0; i < m_drawList->itemCount(); i++)
		{
			UIDrawItem* drawItem = m_drawList->getDrawItem(i);
			if (drawItem->getLayer() > 0)
			{
				secondPass.push_back(drawItem);
				continue;
			}
			processDrawItem(drawItem);
		}

		for (unsigned int i = 0; i < secondPass.size(); i++)
		{
			UIDrawItem* drawItem = secondPass[i];
			processDrawItem(drawItem);
		}

		endRender();
		present();
	}

	void Vulkan_UIRenderer::processDrawItem(UIDrawItem* drawItem)
	{
		VkCommandBuffer commandBuffer = m_commandBuffers[m_currentSwapChainIndex];
		bool isFont = drawItem->getTextureHandle() && drawItem->isFont();
		bool isUsingRect = drawItem->isUsingRectInstance();
		bool isInstance = isUsingRect;

		Shader* shader = m_drawShader;
		if (drawItem->getTextureHandle() > 0)
		{
			if (drawItem->isFont())
			{
				shader = isUsingRect ? m_textInstanceShader : m_textShader;
			}
			else
			{
				shader = isUsingRect ? m_drawInstanceTexShader : m_drawTexShader;
			}
		}
		else if (isUsingRect)
		{
			shader = m_drawInstanceShader;
		}

		VkDescriptorSet descriptorSet = shader->CreateDescriptorSet();
		shader->Use(commandBuffer);
		BindShader(commandBuffer, shader);

		if (drawItem->isDrawMask())
		{
			VulkanFunctions::vkCmdSetStencilTestEnableEXT(commandBuffer, VK_TRUE);
			if (drawItem->getDrawMask() == DRAW_MASK_PUSH)
			{
				pushMask();
				vkCmdSetStencilWriteMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0XFF);
				vkCmdSetStencilCompareMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFF);
				vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, maskCount);
				VulkanFunctions::vkCmdSetStencilOpEXT(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_INCREMENT_AND_CLAMP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS);
				VulkanFunctions::vkCmdSetDepthTestEnableEXT(commandBuffer, VK_FALSE);
			}
			else
			{
				vkCmdSetStencilWriteMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0XFF);
				vkCmdSetStencilCompareMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFF);
				vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, maskCount);
				VulkanFunctions::vkCmdSetStencilOpEXT(commandBuffer, VK_STENCIL_FACE_FRONT_BIT, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_DECREMENT_AND_CLAMP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS);
				VulkanFunctions::vkCmdSetDepthTestEnableEXT(commandBuffer, VK_FALSE);
			}
		}
		else if (maskCount)
		{
			VulkanFunctions::vkCmdSetStencilTestEnableEXT(commandBuffer, VK_TRUE);
			vkCmdSetStencilWriteMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0x00);
			vkCmdSetStencilCompareMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFF);
			vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, maskCount);
			VulkanFunctions::vkCmdSetStencilOpEXT(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_DECREMENT_AND_CLAMP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL);
			VulkanFunctions::vkCmdSetDepthTestEnableEXT(commandBuffer, VK_TRUE);
		}

		// Bind global UBO
		m_globalUniformBuffer.bind(descriptorSet, 0);

		// Bind vertex UBO
		if (drawItem->isCrop())
		{
			VertexUniformBuffer* vertexUbo = getOrCreateNextVertexUniformBuffer();
			vertexUbo->getBufferData().CropBox = drawItem->getCropDimension();
			vertexUbo->flush();
			vertexUbo->bind(descriptorSet, 1);
		}
		else
		{
			m_nullVertexUBO.bind(descriptorSet, 1);
		}

		// Bind fragment UBO
		if ((drawItem->getTextureHandle() == 0 && !isUsingRect) || (isFont && isUsingRect))
		{
			FragmentUniformBuffer* fragmentUbo = getOrCreateNextFragmentUniformBuffer();
			FragmentUniformBuffer::FragmentUniformBufferData& data = fragmentUbo->getBufferData();
			data.bCrop = drawItem->isCrop();
			data.roundness = drawItem->getRoundness();
			data.shapeDimension = drawItem->getDimension();
			fragmentUbo->flush();
			fragmentUbo->bind(descriptorSet, 2);
		}
		else
		{
			m_nullFragmentUBO.bind(descriptorSet, 2);
		}

		// Bind texture
		if (drawItem->getTextureHandle() > 0)
		{
			m_textures[drawItem->getTextureHandle() - 1]->bind(descriptorSet, 3);
		}

		shader->BindDescriptorSet(commandBuffer, descriptorSet);
		
		VkDeviceSize offsets[] = { 0 };
		if (isInstance)
		{
			VertexBuffer* instanceBuffer = setInstanceDrawData(drawItem->getInstances());
			if (isFont)
			{
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_rectTextBuffer, offsets);
			}
			else
			{
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_rectBuffer, offsets);
			}
			instanceBuffer->bindForRendering(commandBuffer, 1);

			vkCmdDraw(commandBuffer, 6, static_cast<uint32_t>(drawItem->getInstances().size()), 0, 0);
		}
		else
		{
			VertexBuffer* vertexBuffer = setDrawData(drawItem->getVertices());
			vertexBuffer->bindForRendering(commandBuffer, 0);
			vkCmdDraw(commandBuffer, static_cast<uint32_t>(drawItem->getVertices().size()), 1, 0, 0);
		}

		if (drawItem->isDrawMask())
		{
			if (drawItem->getDrawMask() == DRAW_MASK_POP)
			{
				popMask();
			}
		}
	}

	void Vulkan_UIRenderer::pushMask()
	{
 		maskCount++;
	}

	void Vulkan_UIRenderer::popMask()
	{
		assert(maskCount > 0);
		maskCount--;
	}

	void Vulkan_UIRenderer::destroyVertexUniformBufferMap()
	{
		for (size_t i = 0; i < m_vertexUniformBuffersMap.size(); i++)
		{
			std::vector<VertexUniformBuffer>& currentBuffers = m_vertexUniformBuffersMap[i];
			for (VertexUniformBuffer& buffer : currentBuffers)
			{
				buffer.Destroy();
			}
			currentBuffers.clear();
		}
	}

	void Vulkan_UIRenderer::destroyFragmentUniformBufferMap()
	{
		for (size_t i = 0; i < m_fragmentUniformBuffersMap.size(); i++)
		{
			std::vector<FragmentUniformBuffer>& currentBuffers = m_fragmentUniformBuffersMap[i];
			for (FragmentUniformBuffer& buffer : currentBuffers)
			{
				buffer.Destroy();
			}
			currentBuffers.clear();
		}
	}

	void Vulkan_UIRenderer::Destroy()
	{
		vkDeviceWaitIdle(m_device);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device, m_imageAvailSemaphores[i], nullptr);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		}

		vkDestroySampler(m_device, m_sampler, nullptr);

		destroyTextures();

		// Clear Descriptor Pool
		for (size_t i = 0; i < m_descriptorPools.size(); i++)
		{
			vkDestroyDescriptorPool(m_device, m_descriptorPools[i], nullptr);
		}

		destroyFragmentUniformBufferMap();
		destroyVertexUniformBufferMap();
		m_globalUniformBuffer.Destroy();
		m_nullFragmentUBO.Destroy();
		m_nullVertexUBO.Destroy();

		// Deleting Shaders and Buffer
 		UIFREE(m_drawShader);
 		UIFREE(m_drawInstanceShader);
 		UIFREE(m_drawTexShader);
 		UIFREE(m_drawInstanceTexShader);
 		UIFREE(m_textShader);
 		UIFREE(m_textInstanceShader);
		Shader::Destroy(m_device);
		
		vkDestroyBuffer(m_device, m_rectTextBuffer, nullptr);
		vkFreeMemory(m_device, m_rectTextBufferMemory, nullptr);

		vkDestroyBuffer(m_device, m_rectBuffer, nullptr);
		vkFreeMemory(m_device, m_rectBufferMemory, nullptr);

		destroyAllInstanceBuffers();

		cleanupSwapChain();

		vkDestroyCommandPool(m_device, m_commandPool, nullptr);

		vkDestroyRenderPass(m_device, m_renderPass, nullptr);

		vkDestroyDevice(m_device, nullptr);

#if VULKAN_VALIDATION_LAYER
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);

		glfwDestroyWindow(m_window);

		glfwTerminate();
	}

	bool Vulkan_UIRenderer::requestToClose()
	{
		return glfwWindowShouldClose(m_window) != 0;
	}

	ZE::UInt32 Vulkan_UIRenderer::createRendererTexture(void* pAddress, UInt32 width, UInt32 height , UInt32 channelCount)
	{
		if (m_freeTextureIndices.size() == 0)
		{
			VulkanTexture* newTexture = UINEW(VulkanTexture);
			newTexture->Init(pAddress, width, height, channelCount);
			m_textures.push_back(newTexture);
			return static_cast<UInt32>(m_textures.size());
		}

		UInt32 idx = m_freeTextureIndices.back();
		m_freeTextureIndices.pop_back();

		VulkanTexture* freeTexture = m_textures[idx];
		freeTexture->Init(pAddress, width, height, channelCount);

		return idx+1;
	}

	void Vulkan_UIRenderer::destroyTexture(UInt32 textureHandle)
	{
		m_textures[textureHandle-1]->Destroy();
		m_freeTextureIndices.push_back(textureHandle-1);
	}

	void Vulkan_UIRenderer::destroyTextures()
	{
		for (UInt32 i = 0; i < m_textures.size(); i++)
		{
			m_textures[i]->Destroy();
			UIFREE(m_textures[i]);
		}
		m_textures.clear();
		m_freeTextureIndices.clear();
	}

	void Vulkan_UIRenderer::resizeWindow(int width, int height)
	{
		glfwGetFramebufferSize(m_window, &m_width, &m_height);
		recreateSwapChain();
	}

	VertexBuffer* Vulkan_UIRenderer::setDrawData(const UIArray<UIVertex>& vertices)
	{
		VkDeviceSize bufferSize = sizeof(UIVertex) * vertices.size();
		VertexBuffer* vertexBuffer = getOrCreateNextVertexBuffer(bufferSize);
		vertexBuffer->copyData((void*)vertices.data(), bufferSize);
		return vertexBuffer;
	}

	VertexBuffer* Vulkan_UIRenderer::setTextData(const UIArray<UIVertex>& vertices)
	{
		VkDeviceSize bufferSize = sizeof(UIVertex) * vertices.size();
		VertexBuffer* vertexBuffer = getOrCreateNextVertexBuffer(bufferSize);
		vertexBuffer->copyData((void*)vertices.data(), bufferSize);
		return vertexBuffer;
	}

	VertexBuffer* Vulkan_UIRenderer::setInstanceDrawData(const UIArray<UIDrawInstance>& instances)
	{
		VkDeviceSize bufferSize = sizeof(UIDrawInstance) * instances.size();
		VertexBuffer* vertexBuffer = getOrCreateNextVertexBuffer(bufferSize);
		vertexBuffer->copyData((void*)instances.data(), bufferSize);
		return vertexBuffer;
	}

	// Specific Platform Implementation
	ZE::UIRenderer* UI::Platform::CreateRenderer()
	{
		return UINEW(Vulkan_UIRenderer);
	}

	VkVertexInputBindingDescription Vertex::GetBindingDescriptions()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(UIVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
		// Pos
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(UIVertex, pos);
		// TexCoord
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(UIVertex, texCoord);
		// Color
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(UIVertex, color);
		return attributeDescriptions;
	}

	std::array<VkVertexInputBindingDescription, 2> DrawInstanceHelper::GetBindingDescriptions()
	{
		std::array<VkVertexInputBindingDescription, 2> bindingDescriptons;
		// Vertex binding
		bindingDescriptons[0].binding = 0;
		bindingDescriptons[0].stride = sizeof(UIVertex);
		bindingDescriptons[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		// Instance binding
		bindingDescriptons[1].binding = 1;
		bindingDescriptons[1].stride = sizeof(UIDrawInstance);
		bindingDescriptons[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		return bindingDescriptons;
	}

	std::array<VkVertexInputAttributeDescription, 7> DrawInstanceHelper::GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions;
		// Pos
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(UIVertex, pos);
		// TexCoord
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(UIVertex, texCoord);
		// Color
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(UIVertex, color);
		// instancePos
		attributeDescriptions[3].binding = 1;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(UIDrawInstance, pos);
		// dimension
		attributeDescriptions[4].binding = 1;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(UIDrawInstance, dimension);
		// instanceColor
		attributeDescriptions[5].binding = 1;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(UIDrawInstance, color);
		// uvDim
		attributeDescriptions[6].binding = 1;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[6].offset = offsetof(UIDrawInstance, uvDim);
		return attributeDescriptions;
	}

	void VertexBuffer::Init(VkDeviceSize size)
	{
		Destroy();
		gVulkanRenderer->createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);
		m_size = size;
	}

	void VertexBuffer::Destroy()
	{
		if (m_buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(gVulkanRenderer->m_device, m_buffer, nullptr);
			vkFreeMemory(gVulkanRenderer->m_device, m_bufferMemory, nullptr);
			m_buffer = VK_NULL_HANDLE;
			m_size = 0;
		}
	}

	void VertexBuffer::copyData(void* data, size_t size)
	{
		void *pData;
		vkMapMemory(gVulkanRenderer->m_device, m_bufferMemory, 0, size, 0, &pData);
		memcpy(pData, data, size);
		vkUnmapMemory(gVulkanRenderer->m_device, m_bufferMemory);
	}

	void VertexBuffer::bindForRendering(VkCommandBuffer commandBuffer, uint32_t binding)
	{
		VkDeviceSize offset[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, binding, 1, &m_buffer, offset);
	}

	void UniformBuffer::Init()
	{
		VkDeviceSize bufferSize = getSize();
		gVulkanRenderer->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			m_buffer, m_bufferMemory);
	}

	void UniformBuffer::Destroy()
	{
		VkDevice device = gVulkanRenderer->m_device;
		vkDestroyBuffer(device, m_buffer, nullptr);
		vkFreeMemory(device, m_bufferMemory, nullptr);
	}

	void UniformBuffer::flush()
	{
		VkDevice device = gVulkanRenderer->m_device;
		void* data;
		vkMapMemory(device, m_bufferMemory, 0, getSize(), 0, &data);
		memcpy(data, getData(), getSize());
		vkUnmapMemory(device, m_bufferMemory);
	}

	void UniformBuffer::bind(VkDescriptorSet descriptorSet, uint32_t binding)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = getSize();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(gVulkanRenderer->m_device, 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanTexture::Init(void* pData, UInt32 width, UInt32 height, UInt32 channelCount)
	{
		VkDeviceSize imageSize = width * height * channelCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		gVulkanRenderer->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

		void* data;
		vkMapMemory(gVulkanRenderer->m_device, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, pData, imageSize);
		vkUnmapMemory(gVulkanRenderer->m_device, stagingMemory);

		VkFormat imageFormat = VK_FORMAT_R8G8B8_UNORM;
		switch (channelCount)
		{
		case 1:
			imageFormat = VK_FORMAT_R8_UNORM;
			break;
		case 2:
			imageFormat = VK_FORMAT_R8G8_UNORM;
			break;
		case 3:
			imageFormat = VK_FORMAT_R8G8B8_UNORM;
			break;
		case 4:
			imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}
		gVulkanRenderer->createImage(width, height, 1, VK_SAMPLE_COUNT_1_BIT, imageFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);

		gVulkanRenderer->transitionImageLayout(m_image, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
		gVulkanRenderer->copyBufferToImage(stagingBuffer, m_image, width, height);
		gVulkanRenderer->transitionImageLayout(m_image, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

		vkDestroyBuffer(gVulkanRenderer->m_device, stagingBuffer, nullptr);
		vkFreeMemory(gVulkanRenderer->m_device, stagingMemory, nullptr);

		m_imageView = gVulkanRenderer->createImageView(m_image, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void VulkanTexture::Destroy()
	{
		vkDestroyImageView(gVulkanRenderer->m_device, m_imageView, nullptr);
		vkDestroyImage(gVulkanRenderer->m_device, m_image, nullptr);
		vkFreeMemory(gVulkanRenderer->m_device, m_imageMemory, nullptr);
	}

	void VulkanTexture::bind(VkDescriptorSet descriptorSet, uint32_t binding)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_imageView;
		imageInfo.sampler = gVulkanRenderer->m_sampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(gVulkanRenderer->m_device, 1, &descriptorWrite, 0, nullptr);
	}
}