#ifndef VKSSHADERPROGRAM
#define VKSSHADERPROGRAM

#include "VkEngine.hpp"
#include "VksBuffer.hpp"
#include "VksTexture.hpp"
#include <tuple>
#include <vector>

class VksShaderProgram: protected VkEngine, public std::enable_shared_from_this<VksShaderProgram>
{
public:
    using DescriptorPool = std::tuple< VkDescriptorType, uint32_t >;
    using SetLayoutBinding = std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>;

protected:
    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragShader = VK_NULL_HANDLE;
    VkShaderModule m_computeShader = VK_NULL_HANDLE;

    VkDescriptorPool m_descPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    
    std::vector<VkDescriptorSet> m_descSets;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageInfos;
    bool m_inited = false;
protected:
    VkShaderModule __createShaderModule( const std::vector<char>& code );
    void __initDescPool( const std::vector<DescriptorPool>& poolValue, int swapChainCount);
    void __initSetLayout( const std::vector<SetLayoutBinding>& setLayoutBinding, int swapChainCount );
public:
    VksShaderProgram( const std::vector<char>& vertexCode, const std::vector<char>& fragCode );
    VksShaderProgram( const std::string& vertexFilePath, const std::string& fragFilePath);
    VksShaderProgram( const std::vector<char>& computeCode);
    VksShaderProgram( const std::string& computeFilePath );
    ~VksShaderProgram();

    void initialize( const std::vector<SetLayoutBinding>& layoutBindings, const std::vector<DescriptorPool>& poolValues, int swapChainCount );

    void updateShaderUniform( int index, uint32_t binding, VkDescriptorType type, const VksBuffer& buffer );
    void updateSampler( int index, uint32_t binding, VkDescriptorType type, const VksTexture& texture );
    
    const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageCreateInfo();
    
//    VkDescriptorSetLayout getDescriptorSetLayout();
    VkPipelineLayout getPipelineLayout();
    VkDescriptorSet getDescriptorSet( int index );
private:
    std::optional<std::vector<char> > __readShaderFile( const char* path );
};

#endif
