#ifndef __ZE_GL_UI_RENDERER__
#define __ZE_GL_UI_RENDERER__

#include "UI/UIRenderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>


#if NDEBUG
#define VULKAN_VALIDATION_LAYER 0
#define ASSERT(expression, message) (void(0))
#define FATALASSERT(expression, message) if(!(expression)) abort()
#else
#define VULKAN_VALIDATION_LAYER 1
#define ASSERT(expression, message) (void) ((!!(expression)) || (_wassert(_CRT_WIDE(#expression message), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0))
#define FATALASSERT(expression, message) (void) ((!!(expression)) || (_wassert(_CRT_WIDE(#expression message), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0))
#endif

#define VK_CHECK_SUCCESS(function_call, message) { bool expression = function_call == VK_SUCCESS; FATALASSERT(expression, message); }

namespace ZE
{
	class Shader;

	namespace Vertex
	{
		VkVertexInputBindingDescription GetBindingDescriptions();
		std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
	};

	namespace DrawInstanceHelper
	{
		std::array<VkVertexInputBindingDescription, 2> GetBindingDescriptions();
		std::array<VkVertexInputAttributeDescription, 7> GetAttributeDescriptions();
	}

	class VertexBuffer
	{
	public:
		VertexBuffer() : m_size(0), m_buffer(VK_NULL_HANDLE) {}
		
		void Init(VkDeviceSize size);
		void Destroy();
		void copyData(void* data, size_t size);
		void bindForRendering(VkCommandBuffer commandBuffer, uint32_t binding);
		
		VkDeviceSize getSize() const { return m_size; }
	protected:
		VkDeviceSize m_size;

		VkBuffer m_buffer;
		VkDeviceMemory m_bufferMemory;
	};

	class UniformBuffer
	{
	public:

		void Init();
		void Destroy();
		void flush();
		void bind(VkDescriptorSet descriptorSet, uint32_t binding);

		virtual void* getData() const = 0;
		virtual VkDeviceSize getSize() const = 0;
	protected:
		VkBuffer m_buffer;
		VkDeviceMemory m_bufferMemory;
	};

	class SharedUniformBuffer : public UniformBuffer
	{
	public:
		struct SharedUniformBufferData
		{
			UIVector2 screenDimension;
			UIVector2 pad;
		};

		virtual void* getData() const { return (void*)(&m_data); }
		virtual VkDeviceSize getSize() const { return sizeof(SharedUniformBufferData); }
		SharedUniformBufferData& getBufferData() { return m_data; }

	protected:
		SharedUniformBufferData m_data;
	};

	class VertexUniformBuffer : public UniformBuffer
	{
	public:
		struct VertexUniformBufferData
		{
			UIVector4 CropBox;
		};

		virtual void* getData() const { return (void*)(&m_data); }
		virtual VkDeviceSize getSize() const { return sizeof(VertexUniformBufferData); }
		VertexUniformBufferData& getBufferData() { return m_data; }

	protected:
		VertexUniformBufferData m_data;
	};

	class FragmentUniformBuffer : public UniformBuffer
	{
	public:
		struct FragmentUniformBufferData
		{
			UIVector2 shapeDimension;
			float roundness;
			bool bCrop;
		};

		virtual void* getData() const { return (void*)(&m_data); }
		virtual VkDeviceSize getSize() const { return sizeof(FragmentUniformBufferData); }
		FragmentUniformBufferData& getBufferData() { return m_data; }

	protected:
		FragmentUniformBufferData m_data;
	};

	class VulkanTexture
	{
	public:
		void Init(void* pData, UInt32 width, UInt32 height, UInt32 channelCount);
		void Destroy();

		void bind(VkDescriptorSet descriptorSet, uint32_t binding);

	protected:
		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_imageMemory;
		VkImageView m_imageView;
	};

	class Vulkan_UIRenderer : public UIRenderer
	{
		friend class Shader;
		friend class VertexBuffer;
		friend class UniformBuffer;
		friend class VulkanTexture;

	public:
		virtual ~Vulkan_UIRenderer();

		virtual void Init(int width, int height) override;
		virtual void ProcessCurrentDrawList() override;
		virtual void Destroy() override;
		virtual bool requestToClose() override;
		virtual UInt32 createRendererTexture(void* pAddress, UInt32 width, UInt32 height, UInt32 channelCount) override;
		virtual void destroyTexture(UInt32 textureHandle);
		virtual void destroyTextures();
		virtual void* getWindowContext() override { return m_window; }
		virtual void resizeWindow(int width, int height);

	private:
		void createVulkanInstance();
		void pickVulkanPhysicalDevice();
		void createVulkanLogicalDevice();
		void createSwapChain();
		void createSwapChainImageViews();
		void createRenderPass();
		void createCommandPool();
		void createDepthResource();
		void createFrameBuffers();
		void createDescriptorPools();
		void createSampler();
		void createCommandBuffers();
		void createSyncObjects();

		void recreateSwapChain();
		void cleanupSwapChain();

		void beginRender();
		void endRender();
		void present();

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
			VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryAllocateFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, UInt32 width, UInt32 height);


		VertexBuffer* getOrCreateNextVertexBuffer(VkDeviceSize size);

		VkDescriptorSet createDescriptorSet(VkDescriptorSetLayout layout);

		VertexUniformBuffer* getOrCreateNextVertexUniformBuffer();
		FragmentUniformBuffer* getOrCreateNextFragmentUniformBuffer();

		void BindShader(VkCommandBuffer commandBuffer, Shader* shader);

	protected:

		VertexBuffer* setDrawData(const UIArray<UIVertex>& vertices);
		VertexBuffer* setTextData(const UIArray<UIVertex>& vertices);
		VertexBuffer* setInstanceDrawData(const UIArray<UIDrawInstance>& instances);
		void processDrawItem(UIDrawItem* drawItem);
		void pushMask();
		void popMask();

		void destroyAllInstanceBuffers();
		void destroyVertexUniformBufferMap();
		void destroyFragmentUniformBufferMap();

		int m_width;
		int m_height;

		uint32_t m_currentSwapChainIndex = 0;

		GLFWwindow* m_window;

		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;

		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		std::vector<VkImageView> m_swapChainImageViews;

		VkRenderPass m_renderPass;

		VkCommandPool m_commandPool;

		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		std::vector<VkFramebuffer> m_swapChainFrameBuffers;

		Shader* m_drawShader;
		Shader* m_drawInstanceShader;
		Shader* m_drawTexShader;
		Shader* m_textShader;
		Shader* m_drawInstanceTexShader;
		Shader* m_textInstanceShader;

		Shader* m_currentShader;

		VkBuffer m_rectBuffer;
		VkDeviceMemory m_rectBufferMemory;

		VkBuffer m_rectTextBuffer;
		VkDeviceMemory m_rectTextBufferMemory;

		// VertexBuffers
		size_t m_currentBufferIndex = 0;
		std::vector<std::vector<VertexBuffer>> m_buffers;

		// UniformBuffers
		SharedUniformBuffer m_globalUniformBuffer;
		VertexUniformBuffer m_nullVertexUBO;
		FragmentUniformBuffer m_nullFragmentUBO;
		std::vector<std::vector<VertexUniformBuffer>> m_vertexUniformBuffersMap;
		std::vector<std::vector<FragmentUniformBuffer>> m_fragmentUniformBuffersMap;
		size_t m_currentVertexUBOIndex = 0;
		size_t m_currentFragmentUBOIndex = 0;
		
		// Descriptor Pools
		std::vector<VkDescriptorPool> m_descriptorPools;

		VkSampler m_sampler;

		std::vector<VkCommandBuffer> m_commandBuffers;

		// Sync objects
		std::vector<VkSemaphore> m_imageAvailSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;

		UInt32 maskCount = 0;

		UIArray<VulkanTexture*> m_textures;
		UIArray<UInt32> m_freeTextureIndices;
	};

	extern Vulkan_UIRenderer* gVulkanRenderer;
}
#endif
