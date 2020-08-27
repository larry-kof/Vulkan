#include "VksAttribute.hpp"


VksAttribute::VksAttribute(int stride, std::vector<VkVertexInputAttributeDescription>& attriDescs)
    :VkEngine(), m_vertexBuffer( nullptr )
{
    memset(&m_vertexInputInfo, 0, sizeof(m_vertexInputInfo));
    memset(&m_inputAssembly, 0, sizeof(m_inputAssembly));
    memset(&m_bindingDesc, 0, sizeof(m_bindingDesc));
    
    m_attributeDesc.resize( attriDescs.size() );
    memcpy(m_attributeDesc.data(), attriDescs.data(), sizeof( VkVertexInputAttributeDescription ) * m_attributeDesc.size() );
    
    m_bindingDesc.binding = 0;
    m_bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_bindingDesc.stride = stride;
    
    for (int i = 0; i < attriDescs.size(); i++) {
        attriDescs[i].binding = 0;
    }
    
    m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertexInputInfo.vertexBindingDescriptionCount = 1;
    m_vertexInputInfo.pVertexBindingDescriptions = &m_bindingDesc;
    
    m_vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDesc.size());
    m_vertexInputInfo.pVertexAttributeDescriptions = m_attributeDesc.data();
    
    m_inputAssembly.sType =VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_inputAssembly.primitiveRestartEnable = false;
    m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VksAttribute::~VksAttribute()
{
    if( m_vertexBuffer ) m_vertexBuffer.reset();
}

void VksAttribute::setTopology(VkPrimitiveTopology topology)
{
    m_inputAssembly.topology = topology;
}

void VksAttribute::createVertexBuffer(const void *data, int byteSize)
{
    VkDeviceSize bufferSize = byteSize;
    auto stagingBuffer = VksBuffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    void *stagingData = nullptr;
    stagingBuffer->mapMemory(0, byteSize, &stagingData);
    
    if( stagingData != nullptr )
    {
        memcpy(stagingData, data, byteSize);
    }
    stagingBuffer->unMapMemory();
    
    m_vertexBuffer = VksBuffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    m_vertexBuffer->copyFromBuffer( *stagingBuffer );
}

std::shared_ptr<VksBuffer> VksAttribute::getVertexBuffer() const
{
    if( m_vertexBuffer ) return m_vertexBuffer;
    return nullptr;
}

const VkPipelineVertexInputStateCreateInfo& VksAttribute::getVertexInputState()
{
    return m_vertexInputInfo;
}

const VkPipelineInputAssemblyStateCreateInfo& VksAttribute::getInputAssembly()
{
    return m_inputAssembly;
}
