#ifndef VKSBUFFER_HPP
#define VKSBUFFER_HPP

#include "VkEngine.hpp"
#include <optional>
#include <memory>

class VksBuffer : protected VkEngine
{
public:
    static constexpr bool value = true;
    static std::shared_ptr<VksBuffer> createBuffer( VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags flags );

    virtual ~VksBuffer();
    const VkBuffer getVkBuffer() const {
        return m_buffer;
    }

    const VkDeviceSize getVkBufferSize() const {
        return m_bufferSize;
    }

    VkDeviceMemory getVkBufferMemory() const {
        return m_bufferMemory;
    }
    
    const VkMemoryPropertyFlags getMemoryFlags()
    {
        return m_memoryFlags;
    }

    bool copyFromBuffer( const VksBuffer& srcBuffer );
    void mapMemory( int offset, VkDeviceSize size, void** ppData );
    void unMapMemory();
    
    bool copyHostDataToBuffer( void* pData, VkDeviceSize size, int offset = 0 );
protected:
    VksBuffer();
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
    VkDeviceSize m_bufferSize = 0;
    VkMemoryPropertyFlags m_memoryFlags;

    std::optional<uint32_t> __findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    bool __canMapped();
};

#endif
