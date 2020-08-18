#pragma once

struct render_mesh
{
    u32 VertexOffset;
    u32 NumVertices;
    u32 IndexOffset;
    u32 NumIndices;

    u32 MaterialId;
};

struct render_model
{
    VkBuffer Vertices;
    VkBuffer Indices;

    u32 NumMeshes;
    render_mesh* MeshArray;
    
    u32 NumTextures;
    VkImage* TextureArray;
};

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
    VkImage Image;
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
};

inline void RenderTargetUpdateEntries(linear_arena* TempArena, render_target* RenderTarget);

//
// NOTE: Fullscreen Passes
//

struct render_fullscreen_pass
{
    render_target* RenderTarget;
    vk_pipeline* Pipeline;
    u32 NumDescriptorSets;
    VkDescriptorSet* DescriptorSets;
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
    vk_gpu_linear_arena HostArena;
    u32* HostPtr;
    vk_gpu_linear_arena GpuArena;
    
    // NOTE: Global Managers
    vk_barrier_manager BarrierManager;
    vk_descriptor_manager DescriptorManager;
    vk_transfer_manager TransferManager;
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
};
