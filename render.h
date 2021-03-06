#pragma once

#define VK_EXPORTED_FUNC(func) PFN_##func func;
#define VK_GLOBAL_LEVEL_FUNC(func) PFN_##func func;
#define VK_INSTANCE_LEVEL_FUNC(func) PFN_##func func;
#define VK_DEVICE_LEVEL_FUNC(func) PFN_##func func;

#define VULKAN_FUNC_LIST                                                \
    VK_EXPORTED_FUNC(       vkGetInstanceProcAddr);                     \
    VK_GLOBAL_LEVEL_FUNC(   vkCreateInstance);                          \
    VK_GLOBAL_LEVEL_FUNC(   vkEnumerateInstanceExtensionProperties);    \
    VK_GLOBAL_LEVEL_FUNC(   vkEnumerateInstanceLayerProperties);        \
    VK_INSTANCE_LEVEL_FUNC( vkDestroyInstance );                        \
    VK_INSTANCE_LEVEL_FUNC( vkEnumeratePhysicalDevices );               \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceProperties );            \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceFeatures );              \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceQueueFamilyProperties ); \
    VK_INSTANCE_LEVEL_FUNC( vkCreateDevice );                           \
    VK_INSTANCE_LEVEL_FUNC( vkGetDeviceProcAddr );                      \
    VK_INSTANCE_LEVEL_FUNC( vkEnumerateDeviceExtensionProperties );     \
    VK_INSTANCE_LEVEL_FUNC( vkDestroySurfaceKHR );                      \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceSurfaceSupportKHR );     \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceSurfaceCapabilitiesKHR ); \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceSurfaceFormatsKHR );     \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceSurfacePresentModesKHR ); \
    VK_INSTANCE_LEVEL_FUNC( vkCreateWin32SurfaceKHR );                  \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceMemoryProperties );      \
    VK_INSTANCE_LEVEL_FUNC( vkGetPhysicalDeviceFormatProperties );      \
    VK_DEVICE_LEVEL_FUNC(   vkGetDeviceQueue );                         \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyDevice );                          \
    VK_DEVICE_LEVEL_FUNC(   vkDeviceWaitIdle );                         \
    VK_DEVICE_LEVEL_FUNC(   vkQueueWaitIdle );                          \
    VK_DEVICE_LEVEL_FUNC(   vkCreateSwapchainKHR );                     \
    VK_DEVICE_LEVEL_FUNC(   vkDestroySwapchainKHR );                    \
    VK_DEVICE_LEVEL_FUNC(   vkGetSwapchainImagesKHR );                  \
    VK_DEVICE_LEVEL_FUNC(   vkAcquireNextImageKHR );                    \
    VK_DEVICE_LEVEL_FUNC(   vkQueuePresentKHR );                        \
    VK_DEVICE_LEVEL_FUNC(   vkCreateSemaphore );                        \
    VK_DEVICE_LEVEL_FUNC(   vkQueueSubmit );                            \
    VK_DEVICE_LEVEL_FUNC(   vkCreateCommandPool );                      \
    VK_DEVICE_LEVEL_FUNC(   vkAllocateCommandBuffers );                 \
    VK_DEVICE_LEVEL_FUNC(   vkBeginCommandBuffer );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCmdPipelineBarrier );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCmdClearColorImage );                     \
    VK_DEVICE_LEVEL_FUNC(   vkEndCommandBuffer );                       \
    VK_DEVICE_LEVEL_FUNC(   vkCreateImageView );                        \
    VK_DEVICE_LEVEL_FUNC(   vkCreateRenderPass );                       \
    VK_DEVICE_LEVEL_FUNC(   vkCreateFramebuffer );                      \
    VK_DEVICE_LEVEL_FUNC(   vkCreateShaderModule );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCreatePipelineLayout );                   \
    VK_DEVICE_LEVEL_FUNC(   vkCreateGraphicsPipelines );                \
    VK_DEVICE_LEVEL_FUNC(   vkCmdBeginRenderPass );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCmdBindPipeline );                        \
    VK_DEVICE_LEVEL_FUNC(   vkCmdDraw );                                \
    VK_DEVICE_LEVEL_FUNC(   vkCmdEndRenderPass );                       \
    VK_DEVICE_LEVEL_FUNC(   vkCreateFence );                            \
    VK_DEVICE_LEVEL_FUNC(   vkCreateBuffer );                           \
    VK_DEVICE_LEVEL_FUNC(   vkGetBufferMemoryRequirements );            \
    VK_DEVICE_LEVEL_FUNC(   vkAllocateMemory );                         \
    VK_DEVICE_LEVEL_FUNC(   vkBindBufferMemory );                       \
    VK_DEVICE_LEVEL_FUNC(   vkMapMemory );                              \
    VK_DEVICE_LEVEL_FUNC(   vkFlushMappedMemoryRanges );                \
    VK_DEVICE_LEVEL_FUNC(   vkUnmapMemory );                            \
    VK_DEVICE_LEVEL_FUNC(   vkCmdSetViewport );                         \
    VK_DEVICE_LEVEL_FUNC(   vkCmdSetScissor );                          \
    VK_DEVICE_LEVEL_FUNC(   vkCmdBindVertexBuffers );                   \
    VK_DEVICE_LEVEL_FUNC(   vkWaitForFences );                          \
    VK_DEVICE_LEVEL_FUNC(   vkResetFences );                            \
    VK_DEVICE_LEVEL_FUNC(   vkFreeMemory );                             \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyBuffer );                          \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyFence );                           \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyFramebuffer );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCmdCopyBuffer );                          \
    VK_DEVICE_LEVEL_FUNC(   vkCreateImage );                            \
    VK_DEVICE_LEVEL_FUNC(   vkGetImageMemoryRequirements );             \
    VK_DEVICE_LEVEL_FUNC(   vkBindImageMemory );                        \
    VK_DEVICE_LEVEL_FUNC(   vkCreateSampler );                          \
    VK_DEVICE_LEVEL_FUNC(   vkCmdCopyBufferToImage );                   \
    VK_DEVICE_LEVEL_FUNC(   vkCreateDescriptorSetLayout );              \
    VK_DEVICE_LEVEL_FUNC(   vkCreateDescriptorPool );                   \
    VK_DEVICE_LEVEL_FUNC(   vkAllocateDescriptorSets );                 \
    VK_DEVICE_LEVEL_FUNC(   vkUpdateDescriptorSets );                   \
    VK_DEVICE_LEVEL_FUNC(   vkCmdBindDescriptorSets );                  \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyDescriptorPool );                  \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyDescriptorSetLayout );             \
    VK_DEVICE_LEVEL_FUNC(   vkDestroySampler );                         \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyImage );                           \
    VK_DEVICE_LEVEL_FUNC(   vkCmdNextSubpass );                         \
    VK_DEVICE_LEVEL_FUNC(   vkFreeCommandBuffers );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCmdExecuteCommands );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCreateComputePipelines );                 \
    VK_DEVICE_LEVEL_FUNC(   vkCmdDispatch );                            \
    VK_DEVICE_LEVEL_FUNC(   vkCmdFillBuffer );                          \
    VK_DEVICE_LEVEL_FUNC(   vkCmdPushConstants );                       \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyShaderModule );                    \
    VK_DEVICE_LEVEL_FUNC(   vkCreateBufferView );                       \
    VK_DEVICE_LEVEL_FUNC(   vkCmdBindIndexBuffer );                     \
    VK_DEVICE_LEVEL_FUNC(   vkCmdDrawIndexed );                         \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyImageView );                       \
    VK_DEVICE_LEVEL_FUNC(   vkCreateQueryPool );                        \
    VK_DEVICE_LEVEL_FUNC(   vkCmdResetQueryPool );                      \
    VK_DEVICE_LEVEL_FUNC(   vkGetQueryPoolResults );                    \
    VK_DEVICE_LEVEL_FUNC(   vkCmdWriteTimestamp );                      \
    VK_DEVICE_LEVEL_FUNC(   vkDestroyRenderPass );                      \
    VK_DEVICE_LEVEL_FUNC(   vkCmdDrawIndirect );                        \


VULKAN_FUNC_LIST;

#undef VK_EXPORTED_FUNC
#undef VK_GLOBAL_LEVEL_FUNC
#undef VK_INSTANCE_LEVEL_FUNC
#undef VK_DEVICE_LEVEL_FUNC

#include "graphics_utils\vulkan\vulkan_utils.h"

//
// NOTE: Render Target
//

enum render_target_entry_flags
{
    RenderTargetEntry_SwapChain = 1 << 0,
};

struct render_target_entry
{
    u32 Flags;
    u32 Width;
    u32 Height;
    VkFormat Format;
    VkImageView View;
};

enum render_target_render_pass_flags
{
    RenderTargetRenderPass_SetViewPort = 1 << 0,
    RenderTargetRenderPass_SetScissor = 1 << 1,
};

struct render_target
{
    u32 Width;
    u32 Height;

    u32 NumEntries;
    VkClearValue* ClearValues;
    render_target_entry** Entries;

    VkFramebuffer FrameBuffer;
    VkRenderPass RenderPass;
};

struct render_target_builder
{
    linear_arena* Arena;
    linear_arena* TempArena;
    temp_mem TempMem;

    b32 HasSwapChain;
    u32 Width;
    u32 Height;
    
    u32 MaxNumEntries;
    u32 NumEntries;
    render_target_entry** Entries;
    VkClearValue* ClearValues;

    // NOTE: If we want to keep the VkRenderPass from before
    VkRenderPass CopyRenderPass;
};

inline void RenderTargetUpdateEntries(linear_arena* TempArena, render_target* RenderTarget);

//
// NOTE: Cube Map Data
//

struct global_cubemap_input_entry
{
    m4 WVPTransform;
    i32 LayerId;
    u32 Pad[15];
};

//
// NOTE: Vk Init
//

struct render_init_params
{
    b32 ValidationEnabled;
    
    u32 WindowWidth;
    u32 WindowHeight;
    VkPresentModeKHR PresentMode;

    u32 InstanceExtensionCount;
    const char** InstanceExtensions;

    u32 LayerCount;
    const char** Layers;
    
    u32 DeviceExtensionCount;
    const char** DeviceExtensions;

    u32 GpuLocalSize;
    
    // NOTE: Features to enable
    VkPhysicalDeviceFeatures EnableFeatures;
    VkPhysicalDeviceFeatures DisableFeatures;
};

struct render_state
{
    // NOTE: Global Vk Data
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    VkPhysicalDeviceLimits DeviceLimits;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    u32 GraphicsQueueFamId;
    VkQueue GraphicsQueue;
    u32 PresentQueueFamId;
    VkQueue PresentQueue;

    // NOTE: Swapchain data
    VkPresentModeKHR PresentMode;
    VkSurfaceKHR WindowSurface;
    VkSwapchainKHR SwapChain;
    VkFormat SwapChainFormat;
    VkExtent2D SwapChainExtent;
    u32 MaxNumSwapChainImgs;
    u32 NumSwapChainImgs;
    VkImage* SwapChainImgs;
    VkImageView* SwapChainViews;
    
    u32 WindowWidth;
    u32 WindowHeight;
    f32 WindowAspectRatio;

    // NOTE: Memory
    u32 StagingMemoryId;
    u32 LocalMemoryId;
    linear_arena CpuArena;
    platform_block_arena CpuBlockArena;
    vk_linear_arena HostArena;
    u32* HostPtr;
    vk_linear_arena GpuArena;
    
    // NOTE: Global Managers
    vk_descriptor_manager DescriptorManager;
    vk_pipeline_manager PipelineManager;

    // NOTE: Command Buffer data
    VkCommandPool CommandPool;
    vk_commands Commands;
    VkSemaphore FinishedRenderingSemaphore;
    VkSemaphore ImageAvailableSemaphore;

    // NOTE: Descriptor data
    VkDescriptorPool DescriptorPool;

    // NOTE: Fullscreen Pass
    VkBuffer FullScreenVbo;
    VkDescriptorSetLayout CopyImageDescLayout;
    VkDescriptorSetLayout ResolveDepthDescLayout;

    // NOTE: Cube Map Globals
    VkBuffer GlobalCubeMapData;
    VkDescriptorSet GlobalCubeMapDescriptor;
    VkDescriptorSetLayout GlobalCubeMapDescLayout;
};

global render_state* RenderState;
