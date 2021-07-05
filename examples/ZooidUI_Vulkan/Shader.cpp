#include "Shader.h"

#include <string>
#include "Vulkan_UIRenderer.h"

namespace ZE
{
	VkDescriptorSetLayout Shader::ms_descriptorSetLayout;

	Shader::~Shader()
	{
		vkDestroyPipeline(gVulkanRenderer->m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(gVulkanRenderer->m_device, m_pipelineLayout, nullptr);
	}

	void Shader::Use(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	}

	VkDescriptorSet Shader::CreateDescriptorSet()
	{
		return gVulkanRenderer->createDescriptorSet(ms_descriptorSetLayout);
	}

	void Shader::BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void Shader::Init(VkDevice device, VkRenderPass renderPass, const char* vertexPath, const char* fragmentPath, const char* geometryPath)
	{
		// Retrieve the vertex/fragment source code from filePath
		std::vector<char> vertexCode;
		std::vector<char> fragmentCode;
		std::vector<char> geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// Open files
			vShaderFile.open(vertexPath, std::ios::ate | std::ios::binary);
			size_t fileSize = (size_t)vShaderFile.tellg();
			vertexCode.resize(fileSize);
			vShaderFile.seekg(0);
			vShaderFile.read(vertexCode.data(), fileSize);
			vShaderFile.close();

			fShaderFile.open(fragmentPath, std::ios::ate | std::ios::binary);
			fileSize = (size_t)fShaderFile.tellg();
			fragmentCode.resize(fileSize);
			fShaderFile.seekg(0);
			fShaderFile.read(fragmentCode.data(), fileSize);
			fShaderFile.close();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			std::cout << vertexPath << std::endl;
			std::cout << fragmentPath << std::endl;
			std::cout << geometryPath << std::endl;
		}

		if (geometryPath)
		{
			std::ifstream gShaderFile;
			gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				gShaderFile.open(geometryPath, std::ios::ate | std::ios::binary);
				size_t fileSize = (size_t)gShaderFile.tellg();
				geometryCode.resize(fileSize);
				gShaderFile.seekg(0);
				gShaderFile.read(geometryCode.data(), fileSize);
				gShaderFile.close();
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
				std::cout << geometryPath << std::endl;
			}
		}

		// Compile shader
		VkShaderModule vertex, fragment, geometry;

		// Vertex Shader
		vertex = createShaderModule(device, vertexCode);

		// Fragment Shader
		fragment = createShaderModule(device, fragmentCode);

		if (geometryPath)
		{
			// Geometry shader
			geometry = createShaderModule(device, geometryCode);
		}
		else
		{
			geometry = VK_NULL_HANDLE;
		}

		CreateGraphicsPipeline(device, renderPass, vertex, fragment, geometry);

		if (geometryPath)
		{
			vkDestroyShaderModule(device, geometry, nullptr);
		}

		vkDestroyShaderModule(device, vertex, nullptr);
		vkDestroyShaderModule(device, fragment, nullptr);
	}

	void Shader::Init(VkDevice device)
	{
		VkDescriptorSetLayoutBinding globalUboLayoutBinding{};
		globalUboLayoutBinding.binding = 0;
		globalUboLayoutBinding.descriptorCount = 1;
		globalUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalUboLayoutBinding.pImmutableSamplers = nullptr;
		globalUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 1;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding ubo1LayoutBinding{};
		ubo1LayoutBinding.binding = 2;
		ubo1LayoutBinding.descriptorCount = 1;
		ubo1LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo1LayoutBinding.pImmutableSamplers = nullptr;
		ubo1LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding texture0LayoutBinding{};
		texture0LayoutBinding.binding = 3;
		texture0LayoutBinding.descriptorCount = 1;
		texture0LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texture0LayoutBinding.pImmutableSamplers = nullptr;
		texture0LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 4> bindings = { globalUboLayoutBinding, uboLayoutBinding, ubo1LayoutBinding, texture0LayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VK_CHECK_SUCCESS(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &ms_descriptorSetLayout), "failed to create descriptor set layout!");
	}

	void Shader::Destroy(VkDevice device)
	{
		vkDestroyDescriptorSetLayout(device, ms_descriptorSetLayout, nullptr);
	}

	void Shader::SetupVertexInputInfo(VkPipelineVertexInputStateCreateInfo& vertexInputInfo)
	{
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		static VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescriptions();
		static auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	}

	VkShaderModule Shader::createShaderModule(VkDevice device, std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = (uint32_t*)(code.data());

		VkShaderModule shaderModule;
		VK_CHECK_SUCCESS(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create Shader Module");

		return shaderModule;
	}

	void Shader::CreateGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkShaderModule geometryShader)
	{
		VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderStageInfo.module = vertexShader;
		vertexShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderStageInfo.module = fragmentShader;
		fragmentShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo geometryShaderStageInfo{};
		geometryShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geometryShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geometryShaderStageInfo.module = geometryShader;
		geometryShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo, geometryShaderStageInfo };
		uint32_t shaderStageCount = geometryShader == VK_NULL_HANDLE ? 2 : 3;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		SetupVertexInputInfo(vertexInputInfo);

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Temporary, we're going to change it later before rendering
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = 100.0f;
		viewport.height = 800.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = { 800, 800 };

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
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

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
			VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
			VK_DYNAMIC_STATE_STENCIL_REFERENCE,
			VK_DYNAMIC_STATE_STENCIL_OP_EXT,
			VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT,
			VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 8;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &ms_descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			std::cout << "failed to create pipeline layout!" << std::endl;
			return;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_SUCCESS(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline), "failed to create graphics pipeline!");
	}

	void InstanceShader::SetupVertexInputInfo(VkPipelineVertexInputStateCreateInfo& vertexInputInfo)
	{
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		static auto bindingDescriptions = DrawInstanceHelper::GetBindingDescriptions();
		static auto attributeDescriptions = DrawInstanceHelper::GetAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	}

}
