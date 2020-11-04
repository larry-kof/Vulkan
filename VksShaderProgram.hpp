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
//    using DescriptorPool = std::tuple< VkDescriptorType, uint32_t >;
//    using SetLayoutBinding = std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>;

    class DescriptorPoolInfo
    {
    public:
        VkDescriptorType shaderVarType;
        uint32_t varCount;
        
        DescriptorPoolInfo()
        : shaderVarType( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER )
        , varCount( 0 )
        {}
        
        DescriptorPoolInfo( VkDescriptorType type, uint32_t count )
        : shaderVarType( type )
        , varCount( count )
        {}
    };
    
    class UniformLayoutBinding
    {
    public:
        uint32_t binding;
        VkDescriptorType shaderVarType;
        VkShaderStageFlags shaderStageFlags;
        
        UniformLayoutBinding()
        : binding( 0 ), shaderVarType( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER )
        , shaderStageFlags( VK_SHADER_STAGE_ALL )
        {}
        
        UniformLayoutBinding( uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStage )
        : binding( binding ), shaderVarType( type ), shaderStageFlags( shaderStage )
        {}
    };
    
    
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
    void __initDescPool( const std::vector<DescriptorPoolInfo>& poolValue, int swapChainCount);
    void __initSetLayout( const std::vector<UniformLayoutBinding>& setLayoutBinding, int swapChainCount );
public:
    VksShaderProgram( const std::vector<char>& vertexCode, const std::vector<char>& fragCode );
    VksShaderProgram( const std::string& vertexFilePath, const std::string& fragFilePath);
    VksShaderProgram( const std::vector<char>& computeCode);
    VksShaderProgram( const std::string& computeFilePath );
    ~VksShaderProgram();

    void initialize( const std::vector<UniformLayoutBinding>& layoutBindings, const std::vector<DescriptorPoolInfo>& poolValues, int swapChainCount );

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
