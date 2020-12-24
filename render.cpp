
//
// NOTE: Render Target
//

inline render_target_entry RenderTargetEntryCreate(u32 Width, u32 Height, VkFormat Format, VkImageView View)
{
    render_target_entry Result = {};
    Result.Width = Width;
    Result.Height = Height;
    Result.Format = Format;
    Result.View = View;

    return Result;
}

inline void RenderTargetEntryCreate(vk_linear_arena* Arena, u32 Width, u32 Height, VkFormat Format, VkImageUsageFlags Usage,
                                    VkImageAspectFlags AspectMask, VkImage* OutImage, render_target_entry* OutEntry)
{
    vk_image Image = VkImageCreate(RenderState->Device, Arena, Width, Height, Format, Usage, AspectMask);
    *OutImage = Image.Image;
    *OutEntry = RenderTargetEntryCreate(Width, Height, Format, Image.View);
}

inline render_target_entry RenderTargetCubeEntryCreate(u32 Width, u32 Height, VkFormat Format, VkImageView View)
{
    render_target_entry Result = {};
    Result.Width = Width;
    Result.Height = Height;
    Result.Format = Format;
    Result.View = View;

    return Result;
}

inline void RenderTargetCubeEntryCreate(vk_linear_arena* Arena, u32 Width, u32 Height, VkFormat Format, VkImageUsageFlags Usage,
                                        VkImageAspectFlags AspectMask, VkImage* OutImage, render_target_entry* OutEntry)
{
    vk_image Image = VkCubeMapCreate(RenderState->Device, Arena, Width, Height, Format, Usage, AspectMask, 1);
    *OutImage = Image.Image;
    *OutEntry = RenderTargetCubeEntryCreate(Width, Height, Format, Image.View);
}

inline void RenderTargetEntryDestroy(render_target_entry Entry)
{
    vkDestroyImageView(RenderState->Device, Entry.View, 0);
}

// TODO: FIX
#if 0
inline void RenderTargetEntryReCreate(vk_linear_arena* Arena, u32 Width, u32 Height, VkFormat Format, VkImageUsageFlags Usage,
                                      VkImageAspectFlags AspectMask, render_target_entry* OutEntry)
{
    Assert((OutEntry->Flags & RenderTargetEntry_SwapChain) == 0);
    if (OutEntry->View != VK_NULL_HANDLE)
    {
        RenderTargetEntryDestroy(*OutEntry);
    }
    *OutEntry = RenderTargetEntryCreate(Arena, Width, Height, Format, Usage, AspectMask);
}
#endif

inline render_target_entry RenderTargetSwapChainEntryCreate(u32 Width, u32 Height, VkFormat Format)
{
    render_target_entry Result = {};
    Result.Width = Width;
    Result.Height = Height;
    Result.Format = Format;
    Result.Flags = RenderTargetEntry_SwapChain;

    return Result;
}

inline void RenderTargetUpdateEntries(linear_arena* TempArena, render_target* RenderTarget)
{
    temp_mem TempMem = BeginTempMem(TempArena);

    // NOTE: Create temp array to pass to VK
    VkImageView* Views = PushArray(TempArena, VkImageView, RenderTarget->NumEntries);
    for (u32 ViewId = 0; ViewId < RenderTarget->NumEntries; ++ViewId)
    {
        Views[ViewId] = RenderTarget->Entries[ViewId]->View;
    }

    // NOTE: Update size and recreate FBO
    RenderTarget->Width = RenderTarget->Entries[0]->Width;
    RenderTarget->Height = RenderTarget->Entries[0]->Height;
    VkFboReCreate(RenderState->Device, RenderTarget->RenderPass, Views, RenderTarget->NumEntries, &RenderTarget->FrameBuffer,
                  RenderTarget->Width, RenderTarget->Height);

    EndTempMem(TempMem);
}

inline void RenderTargetPassBegin(render_target* RenderTarget, vk_commands Commands, u32 Flags)
{
    VkRenderPassBeginInfo BeginInfo = {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    BeginInfo.renderPass = RenderTarget->RenderPass;
    BeginInfo.framebuffer = RenderTarget->FrameBuffer;
    BeginInfo.renderArea.offset = {0, 0};
    BeginInfo.renderArea.extent = { RenderTarget->Width, RenderTarget->Height };
    BeginInfo.clearValueCount = RenderTarget->NumEntries;
    BeginInfo.pClearValues = RenderTarget->ClearValues;
    vkCmdBeginRenderPass(Commands.Buffer, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (Flags & RenderTargetRenderPass_SetViewPort)
    {
        VkViewport ViewPort = {};
        ViewPort.x = 0;
        ViewPort.y = 0;
        ViewPort.width = f32(RenderTarget->Width);
        ViewPort.height = f32(RenderTarget->Height);
        // TODO: How do we want to handle min/max depth?
        ViewPort.minDepth = 0.0f;
        ViewPort.maxDepth = 1.0f;
        vkCmdSetViewport(Commands.Buffer, 0, 1, &ViewPort);
    }
    
    if (Flags & RenderTargetRenderPass_SetScissor)
    {
        VkRect2D Scissor = {};
        Scissor.offset = {};
        Scissor.extent = { RenderTarget->Width, RenderTarget->Height };
        vkCmdSetScissor(Commands.Buffer, 0, 1, &Scissor);
    }
}

inline void RenderTargetNextSubPass(vk_commands Commands)
{
    vkCmdNextSubpass(Commands.Buffer, VK_SUBPASS_CONTENTS_INLINE);
}

inline void RenderTargetPassEnd(vk_commands Commands)
{
    vkCmdEndRenderPass(Commands.Buffer);
}

//
// NOTE: Render Target Builder
//

inline render_target_builder RenderTargetBuilderBegin(linear_arena* Arena, linear_arena* TempArena, u32 Width, u32 Height)
{
    render_target_builder Result = {};

    Result.Arena = Arena;
    Result.TempArena = TempArena;
    Result.TempMem = BeginTempMem(TempArena);
    Result.MaxNumEntries = 10;
    Result.ClearValues = PushArray(TempArena, VkClearValue, Result.MaxNumEntries);
    Result.Entries = PushArray(TempArena, render_target_entry*, Result.MaxNumEntries);
    
    Result.Width = Width;
    Result.Height = Height;

    return Result;
}

inline void RenderTargetAddTarget(render_target_builder* Builder, render_target_entry* Entry, VkClearValue ClearValue)
{
    Assert(Builder->NumEntries < Builder->MaxNumEntries);

    Builder->Entries[Builder->NumEntries] = Entry;
    Builder->ClearValues[Builder->NumEntries] = ClearValue;
    Builder->HasSwapChain = Builder->HasSwapChain || Entry->Flags & RenderTargetEntry_SwapChain;
    
    Builder->NumEntries += 1;
}

inline render_target RenderTargetBuilderEnd(render_target_builder* Builder, VkRenderPass RenderPass)
{
    render_target Result = {};

    Result.Width = Builder->Width;
    Result.Height = Builder->Height;
    
    Result.NumEntries = Builder->NumEntries;
    Result.Entries = PushArray(Builder->Arena, render_target_entry*, Result.NumEntries);
    CopyArray(Builder->Entries, Result.Entries, render_target_entry*, Result.NumEntries);
    Result.ClearValues = PushArray(Builder->Arena, VkClearValue, Result.NumEntries);
    CopyArray(Builder->ClearValues, Result.ClearValues, VkClearValue, Result.NumEntries);

    Result.RenderPass = RenderPass;
    if (!Builder->HasSwapChain)
    {
        RenderTargetUpdateEntries(Builder->TempArena, &Result);
    }
    
    EndTempMem(Builder->TempMem);

    return Result;
}

//
// NOTE: FullScreen Pass
//

inline render_fullscreen_pass FullScreenPassCreate(char* FragmentShader, char* MainFuncName, render_target* RenderTarget, u32 NumLayouts,
                                                   VkDescriptorSetLayout* Layouts, u32 NumDescriptorSets, VkDescriptorSet* DescriptorSets)
{
    render_fullscreen_pass Result = {};

    Result.RenderTarget = RenderTarget;
    Result.NumDescriptorSets = NumDescriptorSets;
    if (Result.NumDescriptorSets > 0)
    {
        Result.DescriptorSets = PushArray(&RenderState->CpuArena, VkDescriptorSet, Result.NumDescriptorSets);
        CopyArray(DescriptorSets, Result.DescriptorSets, VkDescriptorSet, Result.NumDescriptorSets);
    }
    
    // NOTE: Create pipeline
    {
        vk_pipeline_builder Builder = VkPipelineBuilderBegin(&RenderState->CpuArena);
        
        // NOTE: Shaders
        // TODO: Use a custom build file for the win32 exe and for getting the prebuilt shaders to where we need them
        VkPipelineShaderAdd(&Builder, "..\\libs\\framework_vulkan\\fullscreen_pass.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);
        VkPipelineShaderAdd(&Builder, FragmentShader, MainFuncName, VK_SHADER_STAGE_FRAGMENT_BIT);
                
        // NOTE: Specify input vertex data format
        VkPipelineVertexBindingBegin(&Builder);
        VkPipelineVertexAttributeAdd(&Builder, VK_FORMAT_R32G32B32_SFLOAT, sizeof(v3));
        VkPipelineVertexAttributeAdd(&Builder, VK_FORMAT_R32G32_SFLOAT, sizeof(v2));
        VkPipelineVertexBindingEnd(&Builder);
                
        // NOTE: Set the blending state
        VkPipelineColorAttachmentAdd(&Builder, VK_FALSE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO,
                                     VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);

        Result.Pipeline = VkPipelineBuilderEnd(&Builder, RenderState->Device, &RenderState->PipelineManager, RenderTarget->RenderPass, 0,
                                               Layouts, NumLayouts);
    }

    return Result;
}

inline void FullScreenPassRender(vk_commands Commands, render_fullscreen_pass* Pass)
{
    RenderTargetPassBegin(Pass->RenderTarget, Commands, RenderTargetRenderPass_SetViewPort | RenderTargetRenderPass_SetScissor);
    
    vkCmdBindPipeline(Commands.Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pass->Pipeline->Handle);
    if (Pass->NumDescriptorSets > 0)
    {
        vkCmdBindDescriptorSets(Commands.Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pass->Pipeline->Layout, 0, Pass->NumDescriptorSets,
                                Pass->DescriptorSets, 0, 0);
    }

    VkDeviceSize Offset = 0;
    vkCmdBindVertexBuffers(Commands.Buffer, 0, 1, &RenderState->FullScreenVbo, &Offset);
    vkCmdDraw(Commands.Buffer, 6, 1, 0, 0);

    RenderTargetPassEnd(Commands);
}

//
// NOTE: Vulkan Init
//

inline void VkSwapChainReCreate(linear_arena* TempArena, u32 WindowWidth, u32 WindowHeight, VkPresentModeKHR InPresentMode)
{
    temp_mem TempMem = BeginTempMem(TempArena);
    
    RenderState->WindowWidth = WindowWidth;
    RenderState->WindowHeight = WindowHeight;
    RenderState->WindowAspectRatio = (f32)WindowWidth / (f32)WindowHeight;

    // NOTE: Init Swap Chain
    {
        VkSurfaceCapabilitiesKHR SurfaceCapabilities;
        VkCheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(RenderState->PhysicalDevice, RenderState->WindowSurface, &SurfaceCapabilities));

        u32 FormatsCount;
        VkCheckResult(vkGetPhysicalDeviceSurfaceFormatsKHR(RenderState->PhysicalDevice, RenderState->WindowSurface, &FormatsCount, 0));
        Assert(FormatsCount != 0);

        VkSurfaceFormatKHR* SurfaceFormats = PushArray(TempArena, VkSurfaceFormatKHR, FormatsCount);
        VkCheckResult(vkGetPhysicalDeviceSurfaceFormatsKHR(RenderState->PhysicalDevice, RenderState->WindowSurface, &FormatsCount, SurfaceFormats));

        u32 PresentModesCount;
        VkCheckResult(vkGetPhysicalDeviceSurfacePresentModesKHR(RenderState->PhysicalDevice, RenderState->WindowSurface, &PresentModesCount, 0));
        Assert(PresentModesCount != 0);

        VkPresentModeKHR* PresentModes = PushArray(TempArena, VkPresentModeKHR, PresentModesCount);
        VkCheckResult(vkGetPhysicalDeviceSurfacePresentModesKHR(RenderState->PhysicalDevice, RenderState->WindowSurface, &PresentModesCount, PresentModes));

        // NOTE: We define how many images we want in the swap chain
        u32 ImageCount = SurfaceCapabilities.minImageCount + 1;
        if (SurfaceCapabilities.maxImageCount > 0 &&
            ImageCount > SurfaceCapabilities.maxImageCount)
        {
            ImageCount = SurfaceCapabilities.maxImageCount;
        }

        // NOTE: Select image format
        VkSurfaceFormatKHR ChosenFormat = {};
        {
            if (FormatsCount == 1 && SurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
            {
                // NOTE: No preferred format, choose what we want
                ChosenFormat = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
            }

            b32 Found = false;
            for (u32 FormatId = 0; FormatId < FormatsCount; ++FormatId)
            {
                if (SurfaceFormats[FormatId].format == VK_FORMAT_R8G8B8A8_UNORM ||
                    (SurfaceFormats[FormatId].format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
                     SurfaceFormats[FormatId].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
                {
                    ChosenFormat = SurfaceFormats[FormatId];
                    Found = true;
                    break;
                }
            }

            if (!Found)
            {
                ChosenFormat = SurfaceFormats[0];
            }
        }

        // NOTE: Select swap chain image size
        VkExtent2D SwapChainExtent = { WindowWidth, WindowHeight };
        if (SwapChainExtent.width < SurfaceCapabilities.minImageExtent.width)
        {
            SwapChainExtent.width = SurfaceCapabilities.minImageExtent.width;
        }
        if (SwapChainExtent.width > SurfaceCapabilities.maxImageExtent.width)
        {
            SwapChainExtent.width = SurfaceCapabilities.maxImageExtent.width;
        }
        if (SwapChainExtent.height < SurfaceCapabilities.minImageExtent.height)
        {
            SwapChainExtent.height = SurfaceCapabilities.minImageExtent.height;
        }
        if (SwapChainExtent.height > SurfaceCapabilities.maxImageExtent.height)
        {
            SwapChainExtent.height = SurfaceCapabilities.maxImageExtent.height;
        }

        RenderState->WindowWidth = SwapChainExtent.width;
        RenderState->WindowHeight = SwapChainExtent.height;
    
        // NOTE: Ask for the render target to have a color buffer, and to be cleared
        VkImageUsageFlags Flags = {};
        if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        {
            Flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        else
        {
            InvalidCodePath;
        }

        // NOTE: For mobile, we want our swap chain to be transformed (rotation)
        VkSurfaceTransformFlagBitsKHR SurfaceTransform = {};
        if (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            SurfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else
        {
            SurfaceTransform = SurfaceCapabilities.currentTransform;
        }

        // NOTE: Select a presentation mode (we choose triple buffering)
        VkPresentModeKHR PresentMode = {};
        for (u32 PresentationId = 0; PresentationId < PresentModesCount; ++PresentationId)
        {
            if (PresentModes[PresentationId] == InPresentMode)
            {
                PresentMode = PresentModes[PresentationId];
            }
        }
        Assert(PresentMode == InPresentMode);

        // NOTE: Finally create the swap chain
        VkSwapchainKHR OldSwapChain = RenderState->SwapChain;
        VkSwapchainCreateInfoKHR SwapChainCreateInfo =
            {
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                0,
                0,
                RenderState->WindowSurface,
                ImageCount,
                ChosenFormat.format,
                ChosenFormat.colorSpace,
                SwapChainExtent,
                1,
                Flags,
                VK_SHARING_MODE_EXCLUSIVE,
                0,
                0,
                SurfaceTransform,
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                PresentMode,
                VK_TRUE,
                OldSwapChain,
            };
        VkCheckResult(vkCreateSwapchainKHR(RenderState->Device, &SwapChainCreateInfo, 0, &RenderState->SwapChain));

        if (OldSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(RenderState->Device, OldSwapChain, 0);
        }

        // NOTE: Set global meta data about swap chain
        RenderState->SwapChainFormat = ChosenFormat.format;
        RenderState->SwapChainExtent = SwapChainExtent;
        RenderState->NumSwapChainImgs = 0;
        VkCheckResult(vkGetSwapchainImagesKHR(RenderState->Device, RenderState->SwapChain, &RenderState->NumSwapChainImgs, 0));
        Assert(RenderState->NumSwapChainImgs < RenderState->MaxNumSwapChainImgs);
        VkCheckResult(vkGetSwapchainImagesKHR(RenderState->Device, RenderState->SwapChain, &RenderState->NumSwapChainImgs, RenderState->SwapChainImgs));
            
        // NOTE: Create image views for swap chain images
        for (u32 ImgId = 0; ImgId < RenderState->NumSwapChainImgs; ++ImgId)
        {
            VkImageViewCreateInfo ImageViewCreateInfo =
                {
                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    0,
                    0,
                    RenderState->SwapChainImgs[ImgId],
                    VK_IMAGE_VIEW_TYPE_2D,
                    RenderState->SwapChainFormat,
                    {
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    {
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        0,
                        1,
                        0,
                        1,
                    },
                };

            VkCheckResult(vkCreateImageView(RenderState->Device, &ImageViewCreateInfo, 0, &RenderState->SwapChainViews[ImgId]));
        }
    }

    EndTempMem(TempMem);
}

inline void VkGetGlobalFunctionPointers(HMODULE VulkanLib)
{
#define VK_EXPORTED_FUNC(func)                              \
    func = (PFN_##func)GetProcAddress(VulkanLib, #func);    \
    if (!func)                                              \
    {                                                       \
        OutputDebugString(#func);                           \
        InvalidCodePath;                                    \
    }                                                       \
    
#define VK_GLOBAL_LEVEL_FUNC(func)                      \
    func = (PFN_##func)vkGetInstanceProcAddr(0, #func); \
    if (!func)                                          \
    {                                                   \
        OutputDebugString(#func);                       \
        InvalidCodePath;                                \
    }                                                   \

#define VK_INSTANCE_LEVEL_FUNC
#define VK_DEVICE_LEVEL_FUNC
        
    VULKAN_FUNC_LIST;

#undef VK_EXPORTED_FUNC
#undef VK_GLOBAL_LEVEL_FUNC
#undef VK_INSTANCE_LEVEL_FUNC
#undef VK_DEVICE_LEVEL_FUNC
}

inline void VkGetInstanceFunctionPointers()
{
#define VK_INSTANCE_LEVEL_FUNC(func)                                    \
        func = (PFN_##func)vkGetInstanceProcAddr(RenderState->Instance, #func); \
        if (!func)                                                      \
        {                                                               \
            InvalidCodePath;                                            \
        }                                                               \

#define VK_EXPORTED_FUNC
#define VK_GLOBAL_LEVEL_FUNC
#define VK_DEVICE_LEVEL_FUNC
        
        VULKAN_FUNC_LIST;
        
#undef VK_EXPORTED_FUNC
#undef VK_GLOBAL_LEVEL_FUNC
#undef VK_INSTANCE_LEVEL_FUNC
#undef VK_DEVICE_LEVEL_FUNC
}

inline void VkGetDeviceFunctionPointers()
{
#define VK_DEVICE_LEVEL_FUNC(func)                                      \
            func = (PFN_##func)vkGetDeviceProcAddr(RenderState->Device, #func); \
            if (!func)                                                  \
            {                                                           \
                InvalidCodePath;                                        \
            }                                                           \

#define VK_EXPORTED_FUNC
#define VK_GLOBAL_LEVEL_FUNC
#define VK_INSTANCE_LEVEL_FUNC(func)
        
            VULKAN_FUNC_LIST;
        
#undef VK_EXPORTED_FUNC
#undef VK_GLOBAL_LEVEL_FUNC
#undef VK_INSTANCE_LEVEL_FUNC
#undef VK_DEVICE_LEVEL_FUNC
}

inline void VkInit(HMODULE VulkanLib, HINSTANCE hInstance, HWND WindowHandle, linear_arena* Arena, linear_arena* TempArena,
                   render_init_params InitParams)
{
    temp_mem TempMem = BeginTempMem(TempArena);

    //
    // NOTE: Init Vulkan API
    //
    {
        VkGetGlobalFunctionPointers(VulkanLib);
        
        // NOTE: Create a vulkan instance
        {
            // NOTE: Check if instance extensions supported
            u32 GpuExtensionCount = 0;
            VkCheckResult(vkEnumerateInstanceExtensionProperties(0, &GpuExtensionCount, 0));
            VkExtensionProperties* GpuInstanceExtensions = PushArray(TempArena, VkExtensionProperties, GpuExtensionCount);
            VkCheckResult(vkEnumerateInstanceExtensionProperties(0, &GpuExtensionCount, GpuInstanceExtensions));

            u32 InstanceExtensionCount = 2 + InitParams.InstanceExtensionCount;
            const char** InstanceExtensions = PushArray(TempArena, const char*, InstanceExtensionCount);
            Copy(InitParams.InstanceExtensions, InstanceExtensions, InitParams.InstanceExtensionCount*sizeof(*InstanceExtensions));
            InstanceExtensions[InitParams.InstanceExtensionCount + 0] = VK_KHR_SURFACE_EXTENSION_NAME;
            InstanceExtensions[InitParams.InstanceExtensionCount + 1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            
            for (u32 RequiredId = 0; RequiredId < InstanceExtensionCount; ++RequiredId)
            {
                b32 Found = false;
                for (u32 ExtensionId = 0; ExtensionId < GpuExtensionCount; ++ExtensionId)
                {
                    if (strcmp(InstanceExtensions[RequiredId], GpuInstanceExtensions[ExtensionId].extensionName))
                    {
                        Found = true;
                        break;
                    }
                }

                if (!Found)
                {
                    InvalidCodePath;
                }
            }
        
            // NOTE: Create a instance of vulkan
            VkApplicationInfo AppInfo = {};
            AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            AppInfo.pNext = 0;
            AppInfo.pApplicationName = "Dragon";
            AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            AppInfo.pEngineName = "Dragon";
            AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            AppInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

            u32 LayerCount = InitParams.LayerCount + InitParams.ValidationEnabled ? 1 : 0;
            const char** Layers = PushArray(TempArena, const char*, LayerCount);
            Copy(InitParams.Layers, Layers, InitParams.LayerCount*sizeof(*Layers));
            if (InitParams.ValidationEnabled)
            {
                Layers[InitParams.LayerCount + 0] = "VK_LAYER_KHRONOS_validation";
            }
            
            VkInstanceCreateInfo InstanceCreateInfo = {};
            InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            InstanceCreateInfo.pNext = 0;
            InstanceCreateInfo.flags = 0;
            InstanceCreateInfo.pApplicationInfo = &AppInfo;
            InstanceCreateInfo.enabledLayerCount = LayerCount;
            InstanceCreateInfo.ppEnabledLayerNames = Layers;
            InstanceCreateInfo.enabledExtensionCount = InstanceExtensionCount;
            InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions;
            VkCheckResult(vkCreateInstance(&InstanceCreateInfo, 0, &RenderState->Instance));
        }

        VkGetInstanceFunctionPointers();
        
        // NOTE: Create a surface
        {
            VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo =
                {
                    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                    0,
                    0,
                    hInstance,
                    WindowHandle,
                };

            VkCheckResult(vkCreateWin32SurfaceKHR(RenderState->Instance, &SurfaceCreateInfo, 0, &RenderState->WindowSurface));
        }
        
        // NOTE: Select a physical device for our app
        {
            VkPhysicalDeviceFeatures SelectedFeatures = {};
            VkPhysicalDevice SelectedPhysicalDevice = VK_NULL_HANDLE;
            u32 NumDevices = 0;
            VkCheckResult(vkEnumeratePhysicalDevices(RenderState->Instance, &NumDevices, 0));
            Assert(NumDevices != 0);

            VkPhysicalDevice* PhysicalDevices = PushArray(TempArena, VkPhysicalDevice, NumDevices);
            VkCheckResult(vkEnumeratePhysicalDevices(RenderState->Instance, &NumDevices, PhysicalDevices));
            
            u32 DeviceExtensionCount = InitParams.DeviceExtensionCount + 1;
            const char** DeviceExtensions = PushArray(TempArena, const char*, DeviceExtensionCount);
            Copy(InitParams.DeviceExtensions, DeviceExtensions, InitParams.DeviceExtensionCount*sizeof(*DeviceExtensions));
            DeviceExtensions[InitParams.DeviceExtensionCount + 0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            
            RenderState->GraphicsQueueFamId = UINT32_MAX;
            RenderState->PresentQueueFamId = UINT32_MAX;
            for (u32 DeviceId = 0; DeviceId < NumDevices; ++DeviceId)
            {
                VkPhysicalDevice CurrDevice = PhysicalDevices[DeviceId];

                // NOTE: Check if supports extensions we want
                u32 GpuExtensionCount = 0;
                VkCheckResult(vkEnumerateDeviceExtensionProperties(CurrDevice, 0, &GpuExtensionCount, 0));

                VkExtensionProperties* GpuExtensions = PushArray(TempArena, VkExtensionProperties, GpuExtensionCount);
                VkCheckResult(vkEnumerateDeviceExtensionProperties(CurrDevice, 0, &GpuExtensionCount, GpuExtensions));

                for (u32 RequiredId = 0; RequiredId < DeviceExtensionCount; ++RequiredId)
                {
                    b32 Found = false;
                    for (u32 ExtensionId = 0; ExtensionId < GpuExtensionCount; ++ExtensionId)
                    {
                        if (strcmp(DeviceExtensions[RequiredId], GpuExtensions[ExtensionId].extensionName))
                        {
                            Found = true;
                            break;
                        }
                    }

                    if (!Found)
                    {
                        InvalidCodePath;
                    }
                }
                
                // NOTE: Figure out api version, limit on data stuff (textures), whether features are
                // supported like geometry shaders
                VkPhysicalDeviceProperties DeviceProperties;
                VkPhysicalDeviceFeatures DeviceFeatures;

                vkGetPhysicalDeviceProperties(CurrDevice, &DeviceProperties);
                vkGetPhysicalDeviceFeatures(CurrDevice, &DeviceFeatures);

                u32 MajorVer = VK_VERSION_MAJOR(DeviceProperties.apiVersion);
                u32 MinorVer = VK_VERSION_MINOR(DeviceProperties.apiVersion);
                u32 PatchVer = VK_VERSION_PATCH(DeviceProperties.apiVersion);

                if (MajorVer < 1 && DeviceProperties.limits.maxImageDimension2D < 4096)
                {
                    InvalidCodePath;
                }

                RenderState->DeviceLimits = DeviceProperties.limits;

                // NOTE: Check the command buffer queue types we have available
                u32 QueueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(CurrDevice, &QueueFamilyCount, 0);
                if (QueueFamilyCount == 0)
                {
                    InvalidCodePath;
                }

                // NOTE: Check queue's for graphics, compute, transfer, etc support
                VkQueueFamilyProperties* QueueFamilyProperties = PushArray(TempArena, VkQueueFamilyProperties, QueueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(CurrDevice, &QueueFamilyCount, QueueFamilyProperties);
                for (u32 FamilyId = 0; FamilyId < QueueFamilyCount; ++FamilyId)
                {
                    // NOTE: Check if queue supports swap chain as well
                    u32 PresentQueueSupport = UINT32_MAX;
                    vkGetPhysicalDeviceSurfaceSupportKHR(CurrDevice, FamilyId, RenderState->WindowSurface, &PresentQueueSupport);

                    // NOTE: Select queue that supports graphics
                    if (QueueFamilyProperties[FamilyId].queueCount > 0 &&
                        QueueFamilyProperties[FamilyId].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    {
                        // IMPORTANT: We can have diff queues for graphics, present. We may want
                        // to handle that case
                        // NOTE: We take queue that supports graphics and present
                        if (PresentQueueSupport)
                        {
                            RenderState->GraphicsQueueFamId = FamilyId;
                            RenderState->PresentQueueFamId = FamilyId;
                            SelectedPhysicalDevice = CurrDevice;
                            SelectedFeatures = DeviceFeatures;
                        }
                    }
                }

                // NOTE: Check if we found a device
                if (SelectedPhysicalDevice != VK_NULL_HANDLE)
                {
                    break;
                }
            }

            if (SelectedPhysicalDevice == VK_NULL_HANDLE)
            {
                InvalidCodePath;
            }
            
            RenderState->PhysicalDevice = SelectedPhysicalDevice;

            // NOTE: Enable/Disable requested features
            for (u32 OptionId = 0; OptionId < u32(sizeof(SelectedFeatures) / sizeof(VkBool32)); ++OptionId)
            {
                VkBool32* Option = (VkBool32*)&SelectedFeatures + OptionId;
                VkBool32 EnableOption = ((VkBool32*)&InitParams.EnableFeatures)[OptionId];
                VkBool32 DisableOption = ((VkBool32*)&InitParams.DisableFeatures)[OptionId];
                Assert(!(EnableOption == DisableOption && EnableOption == VK_TRUE));

                if (EnableOption == VK_TRUE)
                {
                    *Option = VK_TRUE;
                }
                else if (DisableOption == VK_TRUE)
                {
                    *Option = VK_FALSE;
                }
            }
            
            // NOTE: Create a device
            u32 CurrQueueId = 0;
            f32 QueuePriorities[] = {1.0f};
            VkDeviceQueueCreateInfo QueueCreateInfos[2];

            QueueCreateInfos[CurrQueueId] = {};
            QueueCreateInfos[CurrQueueId].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            QueueCreateInfos[CurrQueueId].pNext = 0;
            QueueCreateInfos[CurrQueueId].flags = 0;
            QueueCreateInfos[CurrQueueId].queueFamilyIndex = RenderState->GraphicsQueueFamId;
            QueueCreateInfos[CurrQueueId].queueCount = ArrayCount(QueuePriorities);
            QueueCreateInfos[CurrQueueId].pQueuePriorities = QueuePriorities;
            ++CurrQueueId;

            if (RenderState->GraphicsQueueFamId != RenderState->PresentQueueFamId)
            {
                QueueCreateInfos[CurrQueueId] = {};
                QueueCreateInfos[CurrQueueId].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                QueueCreateInfos[CurrQueueId].pNext = 0;
                QueueCreateInfos[CurrQueueId].flags = 0;
                QueueCreateInfos[CurrQueueId].queueFamilyIndex = RenderState->PresentQueueFamId;
                QueueCreateInfos[CurrQueueId].queueCount = ArrayCount(QueuePriorities);
                QueueCreateInfos[CurrQueueId].pQueuePriorities = QueuePriorities;
                ++CurrQueueId;
            }
            
            VkDeviceCreateInfo DeviceCreateInfo =
                {
                    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    0,
                    0,
                    CurrQueueId,
                    QueueCreateInfos,
                    0,
                    0,
                    DeviceExtensionCount,
                    DeviceExtensions,
                    &SelectedFeatures,
                };

            VkCheckResult(vkCreateDevice(SelectedPhysicalDevice, &DeviceCreateInfo, 0, &RenderState->Device));
            VkGetDeviceFunctionPointers();
            
            // NOTE: Get device queue
            vkGetDeviceQueue(RenderState->Device, RenderState->GraphicsQueueFamId, 0, &RenderState->GraphicsQueue);
            vkGetDeviceQueue(RenderState->Device, RenderState->PresentQueueFamId, 0, &RenderState->PresentQueue);
            
            // NOTE: Get Physical device mem properties
            vkGetPhysicalDeviceMemoryProperties(RenderState->PhysicalDevice, &RenderState->MemoryProperties);
        }
    }

    // NOTE: Get available memory heaps
    {
        RenderState->StagingMemoryId = VkGetMemoryType(&RenderState->MemoryProperties, 0xFFFFFFFF, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        RenderState->LocalMemoryId = VkGetMemoryType(&RenderState->MemoryProperties, 0xFFFFFFFF, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
    
    // NOTE: Allocate CPU memory
    {
        u64 CpuMemSize = MegaBytes(50);
        RenderState->CpuArena = LinearSubArena(Arena, CpuMemSize);
    }
    
    // NOTE: Allocate host memory
    {
        u64 HeapSize = GigaBytes(1);
        RenderState->HostArena = VkLinearArenaCreate(VkMemoryAllocate(RenderState->Device, RenderState->StagingMemoryId, HeapSize), HeapSize);
        VkCheckResult(vkMapMemory(RenderState->Device, RenderState->HostArena.Memory, 0, HeapSize, 0, (void**)&RenderState->HostPtr));
    }
    
    // NOTE: Allocate GPU memory
    {
        u64 HeapSize = GigaBytes(1);
        RenderState->GpuArena = VkLinearArenaCreate(VkMemoryAllocate(RenderState->Device, RenderState->LocalMemoryId, HeapSize), HeapSize);
    }

    // NOTE: Create Managers
    RenderState->BarrierManager = VkBarrierManagerCreate(&RenderState->CpuArena, 1000);
    RenderState->DescriptorManager = VkDescriptorManagerCreate(&RenderState->CpuArena, 100);
    RenderState->TransferManager = VkTransferManagerCreate(RenderState->Device, RenderState->StagingMemoryId, &RenderState->CpuArena,
                                                          &RenderState->GpuArena, u32(RenderState->DeviceLimits.nonCoherentAtomSize),
                                                          InitParams.StagingBufferSize, 100, 100);
    RenderState->PipelineManager = VkPipelineManagerCreate(&RenderState->CpuArena);
    
    // NOTE: Init memory for swap chain images
    {
        RenderState->MaxNumSwapChainImgs = 32;
        RenderState->SwapChainImgs = PushArray(&RenderState->CpuArena, VkImage, RenderState->MaxNumSwapChainImgs);
        RenderState->SwapChainViews = PushArray(&RenderState->CpuArena, VkImageView, RenderState->MaxNumSwapChainImgs);
        RenderState->PresentMode = InitParams.PresentMode;
        VkSwapChainReCreate(TempArena, InitParams.WindowWidth, InitParams.WindowHeight, InitParams.PresentMode);
    }
    
    // NOTE: Init command buffer data
    {
        VkCommandPoolCreateInfo CmdPoolCreateInfo = {};
        CmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        CmdPoolCreateInfo.pNext = 0;
        CmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        CmdPoolCreateInfo.queueFamilyIndex = RenderState->GraphicsQueueFamId;
        VkCheckResult(vkCreateCommandPool(RenderState->Device, &CmdPoolCreateInfo, 0, &RenderState->CommandPool));

        RenderState->Commands = VkCommandsCreate(RenderState->Device, RenderState->CommandPool);

        VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
        SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        SemaphoreCreateInfo.pNext = 0;
        SemaphoreCreateInfo.flags = 0;
        VkCheckResult(vkCreateSemaphore(RenderState->Device, &SemaphoreCreateInfo, 0, &RenderState->FinishedRenderingSemaphore));
        VkCheckResult(vkCreateSemaphore(RenderState->Device, &SemaphoreCreateInfo, 0, &RenderState->ImageAvailableSemaphore));
    }

    // NOTE: Init descriptor pool
    {
        VkDescriptorPoolSize Pools[5] = {};
        Pools[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        Pools[0].descriptorCount = 1000;
        Pools[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        Pools[1].descriptorCount = 1000;
        Pools[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        Pools[2].descriptorCount = 1000;
        Pools[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        Pools[3].descriptorCount = 1000;
        Pools[4].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        Pools[4].descriptorCount = 1000;
            
        VkDescriptorPoolCreateInfo CreateInfo = {};
        CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        CreateInfo.maxSets = 1000;
        CreateInfo.poolSizeCount = ArrayCount(Pools);
        CreateInfo.pPoolSizes = Pools;
        VkCheckResult(vkCreateDescriptorPool(RenderState->Device, &CreateInfo, 0, &RenderState->DescriptorPool));
    }
    
    // NOTE: Cube Map Globals
    {
        vk_descriptor_layout_builder Builder = VkDescriptorLayoutBegin(&RenderState->GlobalCubeMapDescLayout);
        VkDescriptorLayoutAdd(&Builder, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        VkDescriptorLayoutEnd(RenderState->Device, &Builder);
    }
    
    // NOTE: Upload render state specific assets
    {
        VkCommandsBegin(RenderState->Device, RenderState->Commands);

        // NOTE: Full Screen Data
        {
            f32 Vertices[] =
                {
                    -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
                     1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
                     1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
                     1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
                    -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
                };
            
            RenderState->FullScreenVbo = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                       sizeof(Vertices));

            f32* Data = VkTransferPushWriteArray(&RenderState->TransferManager, RenderState->FullScreenVbo, f32, ArrayCount(Vertices),
                                                 BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                                 BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
            Copy(Vertices, Data, sizeof(Vertices));
        }

        // NOTE: Push global cube map data
        {
            u32 EntrySize = u32(Max(RenderState->DeviceLimits.minUniformBufferOffsetAlignment, sizeof(global_cubemap_input_entry)));
            RenderState->GlobalCubeMapData = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                            EntrySize*6);
            u8* Data = VkTransferPushWrite(&RenderState->TransferManager, RenderState->GlobalCubeMapData, EntrySize*6,
                                           BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                           BarrierMask(VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT));

            m4 PerspectiveTransform = VkPerspProjM4(1.0f, DegreeToRadians(90.0f), 0.1f, 10.0f);
            m4 Transforms[6] =
            {
                PerspectiveTransform * LookAtM4(V3(-1, 0, 0), V3(0, 1, 0), V3(0, 0, 0)), // NOTE: Left
                PerspectiveTransform * LookAtM4(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 0)), // NOTE: Right
                PerspectiveTransform * LookAtM4(V3(0, 1, 0), V3(0, 0, -1), V3(0, 0, 0)), // NOTE: Top
                PerspectiveTransform * LookAtM4(V3(0, -1, 0), V3(0, 0, 1), V3(0, 0, 0)), // NOTE: Bottom
                PerspectiveTransform * LookAtM4(V3(0, 0, 1), V3(0, 1, 0), V3(0, 0, 0)), // NOTE: Front
                PerspectiveTransform * LookAtM4(V3(0, 0, -1), V3(0, 1, 0), V3(0, 0, 0)), // NOTE: Back
            };

            for (u32 FaceId = 0; FaceId < 6; ++FaceId)
            {
                global_cubemap_input_entry* Inputs = (global_cubemap_input_entry*)Data;
                Inputs->WVPTransform = Transforms[FaceId];
                Inputs->LayerId = FaceId;

                Data += EntrySize;
            }
            
            RenderState->GlobalCubeMapDescriptor = VkDescriptorSetAllocate(RenderState->Device, RenderState->DescriptorPool, RenderState->GlobalCubeMapDescLayout);
            VkDescriptorBufferWrite(&RenderState->DescriptorManager, RenderState->GlobalCubeMapDescriptor, 0,
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, RenderState->GlobalCubeMapData);
        }
        
        VkTransferManagerFlush(&RenderState->TransferManager, RenderState->Device, RenderState->Commands.Buffer, &RenderState->BarrierManager);
        VkCommandsSubmit(RenderState->GraphicsQueue, RenderState->Commands);
    }
    
    EndTempMem(TempMem);
}
