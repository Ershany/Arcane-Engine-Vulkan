#pragma once

#include "arcpch.h"

namespace Arcane
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 colour;
		glm::vec2 uv;

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // This is where you can use instance rendering

			// TODO: Add another slot with binding = 1, that contains the instance data that will be fed to the vertex shader at a per instance rate

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription()
		{
			std::vector<VkVertexInputAttributeDescription> attributesDescription(3);

			attributesDescription[0].binding = 0;
			attributesDescription[0].location = 0;
			attributesDescription[0].offset = offsetof(Vertex, pos);
			attributesDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributesDescription[1].binding = 0;
			attributesDescription[1].location = 1;
			attributesDescription[1].offset = offsetof(Vertex, colour);
			attributesDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributesDescription[2].binding = 0;
			attributesDescription[2].location = 2;
			attributesDescription[2].offset = offsetof(Vertex, uv);
			attributesDescription[2].format = VK_FORMAT_R32G32_SFLOAT;

			// TODO: Add instance bindings if needed

			return attributesDescription;
		}
	};
}
