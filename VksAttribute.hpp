//
//  VksAttribute.hpp
//  Vulkan
//
//

#ifndef VksAttribute_h
#define VksAttribute_h

#include "VkEngine.hpp"
#include "VksBuffer.hpp"
#include <tuple>
#include <vector>

class VksAttribute : protected VkEngine
{
public:
//    using AttributeElement = std::tuple<int/*binding*/, int/*location*/, VkFormat ,int/*offset*/>;
    
    VksAttribute(int stride, std::vector<VkVertexInputAttributeDescription>& attriDescs);
    ~VksAttribute();
    void setTopology(VkPrimitiveTopology topology);
    
    void createVertexBuffer( const void* data, int byteSize );
    
    std::shared_ptr<VksBuffer> getVertexBuffer() const;
    
    const VkPipelineVertexInputStateCreateInfo& getVertexInputState();
    const VkPipelineInputAssemblyStateCreateInfo& getInputAssembly();
private:
    VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
    VkVertexInputBindingDescription m_bindingDesc;
    std::vector<VkVertexInputAttributeDescription> m_attributeDesc;
    
    std::shared_ptr<VksBuffer> m_vertexBuffer;
};

#endif /* VksAttribute_h */
