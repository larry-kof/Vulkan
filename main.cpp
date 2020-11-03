

#include <iostream>
#include "VksSwapChain.hpp"

extern int triangleDemo( VksSwapChain& swapChain );
extern int textureDemo( VksSwapChain& swapChain );
extern int depthDemo( VksSwapChain& swapChain );

int main()
{
    VksSwapChain swapChain;
//    int ret = triangleDemo( swapChain );
    int ret = textureDemo( swapChain );
//    int ret = depthDemo( swapChain );
    return ret;
}
