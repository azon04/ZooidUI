#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <array>

#include <vulkan/vulkan.h>

#include "UI/ZooidUI.h"

namespace ZE
{
	class Shader
	{
	public:

		// Constructor reads and builds the shader
		Shader(VkDevice device, VkRenderPass renderPass, const char* vertexPath, const char* fragmentPath, const char* geometryPath)
		{
			Init(device, renderPass, vertexPath, fragmentPath, geometryPath);
		}

		virtual ~Shader();

		static void Init(VkDevice device);
		static void Destroy(VkDevice device);

		// Use the program
		void Use(VkCommandBuffer commandBuffer);
		VkDescriptorSet CreateDescriptorSet();
		void BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet);

		void Init(VkDevice device, VkRenderPass renderPass, const char* vertexPath, const char* fragmentPath, const char* geometryPath);

	protected:
		Shader() {}

		virtual void SetupVertexInputInfo(VkPipelineVertexInputStateCreateInfo& vertexInputInfo);

		VkShaderModule createShaderModule(VkDevice device, std::vector<char>& code);
		void CreateGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkShaderModule geometryShader);

		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		static VkDescriptorSetLayout ms_descriptorSetLayout;
	};

	class InstanceShader : public Shader
	{
	public:
		InstanceShader(VkDevice device, VkRenderPass renderPass, const char* vertexPath, const char* fragmentPath, const char* geometryPath)
		{
			Init(device, renderPass, vertexPath, fragmentPath, geometryPath);
		}

	protected:
		virtual void SetupVertexInputInfo(VkPipelineVertexInputStateCreateInfo& vertexInputInfo) override;
	};
}
#endif

