#include "VksBuffer.hpp"
#include "VksCommand.hpp"

VksBuffer::VksBuffer()
{

}

VksBuffer::~VksBuffer()
{
    if( m_bufferMemory )
    {
        vkFreeMemory(m_logicDevice, m_bufferMemory, nullptr);
    }

    if( m_buffer )
    {
        vkDestroyBuffer(m_logicDevice, m_buffer, nullptr);
    }
}

std::optional<uint32_t> VksBuffer::__findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties ) {
            return i;
        }
    }

    return std::nullopt;
}

std::shared_ptr<VksBuffer> VksBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags flags)
{
    std::shared_ptr<VksBuffer> buffer( new VksBuffer );

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = bufferUsage;
    bufferInfo.size = size;

    buffer->m_bufferSize = size;

    VK_CHECK( vkCreateBuffer(m_logicDevice, &bufferInfo, nullptr, &buffer->m_buffer) )

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_logicDevice, buffer->m_buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    
    std::optional<uint32_t> memoIndex = buffer->__findMemoryType(memRequirements.memoryTypeBits, flags );

    if( memoIndex == std::nullopt )
    {
        throw std::runtime_error("");
    }

    allocInfo.memoryTypeIndex = memoIndex.value();

    if( vkAllocateMemory(m_logicDevice, &allocInfo, nullptr, &buffer->m_bufferMemory) != VK_SUCCESS )
    {
        throw std::runtime_error("fail to alloc memory");
    }
    
    buffer->m_memoryFlags = flags;
    
    vkBindBufferMemory(m_logicDevice, buffer->m_buffer, buffer->m_bufferMemory, 0);
    return buffer;
}

bool VksBuffer::copyFromBuffer(const VksBuffer &srcBuffer)
{
    if( srcBuffer.getVkBufferSize() != this->getVkBufferSize() )
        return false;
    VkCommandBuffer commandBuffer = m_graphicCommand->beginOnceSubmitBuffer();
    
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = srcBuffer.getVkBufferSize();
    
    vkCmdCopyBuffer(commandBuffer, srcBuffer.getVkBuffer(), m_buffer, 1, &copyRegion);
    
    m_graphicCommand->endOnceSubmitBuffer(commandBuffer);
    
    return true;
}

void VksBuffer::mapMemory(int offset, VkDeviceSize size, void** ppData)
{
    if( !__canMapped() )
    {
        *ppData = nullptr;
        return;
    }
    
    vkMapMemory(m_logicDevice, m_bufferMemory, offset, size, 0, ppData);
}

void VksBuffer::unMapMemory()
{
    if( __canMapped() )
        vkUnmapMemory(m_logicDevice, m_bufferMemory);
}

bool VksBuffer::__canMapped()
{
    VkMemoryRequirements memoryReq;
    vkGetBufferMemoryRequirements(m_logicDevice, m_buffer, &memoryReq);
    
    auto ret = __findMemoryType(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
    return ret != std::nullopt;
}

bool VksBuffer::copyHostDataToBuffer(void *pData, VkDeviceSize size, int offset)
{
    if( !__canMapped() )
        return false;
    void *bufferAddr;
    mapMemory(offset, size, &bufferAddr);
    memcpy( bufferAddr, pData, size );
    unMapMemory();
    
    return true;
}
