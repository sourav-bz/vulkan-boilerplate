#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <functional>

namespace std {
    template<> struct hash<glm::vec2> {
        size_t operator()(glm::vec2 const& v) const {
            return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1);
        }
    };
    
    template<> struct hash<glm::vec3> {
        size_t operator()(glm::vec3 const& v) const {
            return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1) ^ (hash<float>()(v.z) << 2);
        }
    };
}

// Basic vertex with position only
struct BasicVertex {
    glm::vec3 pos;

    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(BasicVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
        
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(BasicVertex, pos);
        
        return attributeDescriptions;
    }

    bool operator==(const BasicVertex& other) const {
        return pos == other.pos;
    }
};

// Vertex with position and color
struct ColoredVertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ColoredVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ColoredVertex, pos);
        
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ColoredVertex, color);
        
        return attributeDescriptions;
    }

    bool operator==(const ColoredVertex& other) const {
        return pos == other.pos && color == other.color;
    }
};

// Standard vertex with position, color, and texture coordinates (your current Vertex)
struct StandardVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(StandardVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
        
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(StandardVertex, pos);
        
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(StandardVertex, color);
        
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(StandardVertex, texCoord);
        
        return attributeDescriptions;
    }

    bool operator==(const StandardVertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

// Hash functions for unordered_map usage
namespace std {
    template<> struct hash<BasicVertex> {
        size_t operator()(BasicVertex const& vertex) const {
            return hash<glm::vec3>()(vertex.pos);
        }
    };
    
    template<> struct hash<ColoredVertex> {
        size_t operator()(ColoredVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)));
        }
    };
    
    template<> struct hash<StandardVertex> {
        size_t operator()(StandardVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
