#include "VksShaderProgram.hpp"
#include <fstream>

VksShaderProgram::VksShaderProgram( const std::vector<char>& vertexCode, const std::vector<char>& fragCode )
{
    m_vertexShader = __createShaderModule( vertexCode );
    m_fragShader = __createShaderModule( fragCode );
}

VksShaderProgram::VksShaderProgram( const std::vector<char>& computeCode)
{
    m_computeShader = __createShaderModule( computeCode );
}

VksShaderProgram::VksShaderProgram( const std::string& vertexFilePath, const std::string& fragFilePath)
{
    auto vertexCode = __readShaderFile(vertexFilePath.c_str());
    auto fragCode = __readShaderFile(fragFilePath.c_str());
    
    if( vertexCode.has_value() && fragCode.has_value() )
    {
        m_vertexShader = __createShaderModule( vertexCode.value() );
        m_fragShader = __createShaderModule( fragCode.value() );
    }
}

VksShaderProgram::VksShaderProgram( const std::string& computeFilePath )
{
    auto computeCode = __readShaderFile( computeFilePath.c_str() );
    if( computeCode.has_value() )
    {
        m_computeShader = __createShaderModule( computeCode.value() );
    }
}

std::optional< std::vector<char> > VksShaderProgram::__readShaderFile(const char *path)
{
    std::ifstream is(path, std::ifstream::binary);
    if( !is ) return std::nullopt;
    is.seekg( 0, is.end);
    int length = static_cast<int>( is.tellg() );
    is.seekg( 0, is.beg);
    
    std::vector<char> buffer( length );
    
    is.read( buffer.data() , length);
    
    if( !is ) return std::nullopt;
    is.close();
    return buffer;
}

VksShaderProgram::~VksShaderProgram()
{
    if( m_vertexShader )
    {
        vkDestroyShaderModule(m_logicDevice, m_vertexShader, nullptr);
    }
    if( m_fragShader )
    {
        vkDestroyShaderModule(m_logicDevice, m_fragShader, nullptr);
    }
    if( m_computeShader )
    {
        vkDestroyShaderModule(m_logicDevice, m_computeShader, nullptr);
    }

    if( m_descSetLayout )
    {
        vkDestroyDescriptorSetLayout(m_logicDevice, m_descSetLayout, nullptr );
    }

    if( m_descPool )
    {
        vkDestroyDescriptorPool( m_logicDevice, m_descPool, nullptr );
    }
    
    if( m_pipelineLayout )
    {
        vkDestroyPipelineLayout(m_logicDevice, m_pipelineLayout, nullptr);
    }
}

void VksShaderProgram::__initDescPool(const std::vector<DescriptorPool>& poolValues, int swapChainCount )
{
    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    std::vector< VkDescriptorPoolSize > poolSize( poolValues.size() );
    memset( poolSize.data(), 0, sizeof( VkDescriptorPoolSize ) * poolSize.size() );
    for( int i = 0; i < poolValues.size(); i++ )
    {
        DescriptorPool poolSizeInfo = poolValues[i];
        VkDescriptorType type;
        uint32_t descriptorCount;
        std::tie(type, descriptorCount) = poolSizeInfo;

        poolSize[i].type = type;
        poolSize[i].descriptorCount = descriptorCount;
    }

    poolCreateInfo.poolSizeCount = static_cast<uint32_t>( poolSize.size() );
    poolCreateInfo.pPoolSizes = poolSize.data();

    poolCreateInfo.maxSets = swapChainCount;

    VK_CHECK( vkCreateDescriptorPool( m_logicDevice, &poolCreateInfo, nullptr, &m_descPool ) );
}

void VksShaderProgram::__initSetLayout( const std::vector<SetLayoutBinding>& setLayoutBinds, int swapChainCount )
{
    std::vector<VkDescriptorSetLayoutBinding> uniformBindings( setLayoutBinds.size() );
    memset( uniformBindings.data(), 0, sizeof(VkDescriptorSetLayoutBinding) * uniformBindings.size() );
    for(int i = 0; i < setLayoutBinds.size(); i++)
    {
        uint32_t binding;
        VkDescriptorType type;
        VkShaderStageFlags stageFlags;
        std::tie( binding, type, stageFlags ) = setLayoutBinds[i];

        uniformBindings[i].binding = binding;
        uniformBindings[i].descriptorCount = 1;
        uniformBindings[i].descriptorType = type;
        uniformBindings[i].stageFlags = stageFlags;
        uniformBindings[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo setLayoutInfo = {};
    setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutInfo.bindingCount = static_cast<uint32_t>( uniformBindings.size() );
    setLayoutInfo.pBindings = uniformBindings.data();

    VK_CHECK( vkCreateDescriptorSetLayout( m_logicDevice, &setLayoutInfo, nullptr, &m_descSetLayout ) )

    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = m_descPool;

    std::vector<VkDescriptorSetLayout> layouts(swapChainCount, m_descSetLayout);
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>( swapChainCount );
    setAllocInfo.pSetLayouts = layouts.data();

    m_descSets.resize( swapChainCount );
    VK_CHECK( vkAllocateDescriptorSets(m_logicDevice, &setAllocInfo, m_descSets.data()) )
}

void VksShaderProgram::initialize(  const std::vector<SetLayoutBinding>& layoutBindings, const std::vector<DescriptorPool>& poolValues, int swapChainCount )
{
    __initDescPool( poolValues, swapChainCount );
    __initSetLayout( layoutBindings, swapChainCount );
    
    VkPipelineLayoutCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.setLayoutCount = 1;
    pipelineInfo.pSetLayouts = &m_descSetLayout;
    pipelineInfo.pushConstantRangeCount = 0;
    
    VK_CHECK( vkCreatePipelineLayout(m_logicDevice, &pipelineInfo, nullptr, &m_pipelineLayout) )
    
    m_inited = true;
}

VkShaderModule VksShaderProgram::__createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader;
    VK_CHECK( vkCreateShaderModule(m_logicDevice, &createInfo, nullptr, &shader) )

    return shader;
}
void VksShaderProgram::updateShaderUniform( int index, uint32_t binding, VkDescriptorType type, const VksBuffer& buffer )
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer.getVkBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = buffer.getVkBufferSize();

    VkWriteDescriptorSet writeDesc = {};
    writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDesc.dstSet = m_descSets[index];
    writeDesc.dstBinding = binding;
    writeDesc.descriptorType = type;
    
    writeDesc.dstArrayElement = 0;
    writeDesc.descriptorCount = 1;
    writeDesc.pBufferInfo = &bufferInfo;
    writeDesc.pImageInfo = nullptr; // Optional
    writeDesc.pTexelBufferView = nullptr; // Optional

    vkUpdateDescriptorSets(m_logicDevice, 1, &writeDesc, 0, nullptr);
}

const std::vector<VkPipelineShaderStageCreateInfo>& VksShaderProgram::getShaderStageCreateInfo()
{
    if( m_computeShader == VK_NULL_HANDLE )
    {
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.module = m_vertexShader;
        vertShaderStageInfo.pName = "main";
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.module = m_fragShader;
        fragShaderStageInfo.pName = "main";
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        m_shaderStageInfos.push_back(vertShaderStageInfo);
        m_shaderStageInfos.push_back(fragShaderStageInfo);
    }
    else
    {
        VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.module = m_computeShader;
        computeShaderStageInfo.pName = "main";
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        
        m_shaderStageInfos.push_back(computeShaderStageInfo);
    }
    
    return m_shaderStageInfos;
}

VkPipelineLayout VksShaderProgram::getPipelineLayout()
{
    return m_pipelineLayout;
}

VkDescriptorSet VksShaderProgram::getDescriptorSet(int index)
{
    return m_descSets[index];
}
