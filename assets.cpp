
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct texture_load_params
{
    u32 ForceComponentCount;
    b32 FlipY;
    b32 FloatType;
};

internal vk_image TextureLoad(char* ImagePath, VkFormat Format, u32 ComponentSize, texture_load_params LoadParams)
{
    // IMPORTANT: We assume this is happening inside of a transfer op
    vk_image Result = {};

    i32 NumComponents = 0;
    i32 Width = 0;
    i32 Height = 0;
    u8* Pixels = 0;
    if (LoadParams.FloatType)
    {
        Pixels = (u8*)stbi_loadf(ImagePath, &Width, &Height, &NumComponents, LoadParams.ForceComponentCount);
    }
    else
    {
        Pixels = (u8*)stbi_load(ImagePath, &Width, &Height, &NumComponents, LoadParams.ForceComponentCount);
    }
    Assert(Pixels);
    
    if (LoadParams.ForceComponentCount > 0)
    {
        NumComponents = LoadParams.ForceComponentCount;
    }
    
    Result = VkImageCreate(RenderState->Device, &RenderState->GpuArena, Width, Height, Format,
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    // TODO: Better barrier here pls
    u8* GpuMemory = VkCommandsPushWriteImage(&RenderState->Commands, Result.Image, Width, Height, ComponentSize*NumComponents, 
                                             VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                             BarrierMask(VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT));

    u32 ImageSize = Width*Height*NumComponents*ComponentSize;

    if (LoadParams.FlipY)
    {
        u32 NumPixelBytes = ComponentSize * NumComponents;
        u8* Src = Pixels;
        u8* Dst = GpuMemory + ImageSize - 1;
        for (i32 Y = 0; Y < Height; ++Y)
        {
            for (i32 X = 0; X < Width; ++X)
            {
                for (u32 PixelByte = 0; PixelByte < NumPixelBytes; ++PixelByte)
                {
                    *Dst-- = *Src++;
                }
            }
        }
    }
    else
    {
        Copy(Pixels, GpuMemory, ImageSize);
    }
    
    // NOTE: Delete file from memory
    stbi_image_free(Pixels);

    return Result;
}

inline vk_image TextureLoad(char* ImagePath, VkFormat Format, b32 FlipY, u32 ComponentSize, u32 ForceComponentCount = 0)
{
    vk_image Result = {};
    
    texture_load_params LoadParams = {};
    LoadParams.ForceComponentCount = ForceComponentCount;
    LoadParams.FlipY = FlipY;
    LoadParams.FloatType = false;
    Result = TextureLoad(ImagePath, Format, ComponentSize, LoadParams);

    return Result;
}

//
// NOTE: Debug Meshes
//

inline procedural_mesh AssetsPushQuad()
{
    procedural_mesh Result = {};
    
    // IMPORTANT: Its assumed this is happening during a transfer operation
    {
        f32 Vertices[] = 
            {
                -0.5, -0.5, 0,   0, 0, 1,   0, 0,
                0.5, -0.5, 0,   0, 0, 1,   1, 0,
                0.5,  0.5, 0,   0, 0, 1,   1, 1,
                -0.5,  0.5, 0,   0, 0, 1,   0, 1,
            };

        Result.Vertices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                         sizeof(Vertices));
        void* GpuMemory = VkCommandsPushWrite(&RenderState->Commands, Result.Vertices, sizeof(Vertices),
                                              BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                              BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
        Copy(Vertices, GpuMemory, sizeof(Vertices));
    }
            
    {
        u32 Indices[] =
            {
                0, 1, 2,
                2, 3, 0,
            };

        Result.NumIndices = ArrayCount(Indices);
        Result.Indices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                        sizeof(Indices));
        void* GpuMemory = VkCommandsPushWrite(&RenderState->Commands, Result.Indices, sizeof(Indices),
                                              BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                              BarrierMask(VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
        Copy(Indices, GpuMemory, sizeof(Indices));
    }

    return Result;
}

inline procedural_mesh AssetsPushCube()
{
    procedural_mesh Result = {};
        
    // IMPORTANT: Its assumed this is happening during a transfer operation
    {
        f32 Vertices[] = 
            {
                // NOTE: Front face
                -0.5, -0.5, 0.5, 0, 0, -1, 0, 0,
                0.5, -0.5, 0.5, 0, 0, -1, 0, 0,
                0.5, 0.5, 0.5, 0, 0, -1, 0, 0,
                -0.5, 0.5, 0.5, 0, 0, -1, 0, 0,

                // NOTE: Back face
                -0.5, -0.5, -0.5, 0, 0, 1, 0, 0,
                0.5, -0.5, -0.5, 0, 0, 1, 0, 0,
                0.5, 0.5, -0.5, 0, 0, 1, 0, 0,
                -0.5, 0.5, -0.5, 0, 0, 1, 0, 0,

                // NOTE: Left face
                -0.5, -0.5, -0.5, -1, 0, 0, 0, 0,
                -0.5, 0.5, -0.5, -1, 0, 0, 0, 0,
                -0.5, 0.5, 0.5, -1, 0, 0, 0, 0,
                -0.5, -0.5, 0.5, -1, 0, 0, 0, 0,

                // NOTE: Right face
                0.5, -0.5, -0.5, 1, 0, 0, 0, 0,
                0.5, 0.5, -0.5, 1, 0, 0, 0, 0,
                0.5, 0.5, 0.5, 1, 0, 0, 0, 0,
                0.5, -0.5, 0.5, 1, 0, 0, 0, 0,

                // NOTE: Top face
                -0.5, 0.5, -0.5, 0, 1, 0, 0, 0,
                0.5, 0.5, -0.5, 0, 1, 0, 0, 0,
                0.5, 0.5, 0.5, 0, 1, 0, 0, 0,
                -0.5, 0.5, 0.5, 0, 1, 0, 0, 0,

                // NOTE: Bottom face
                -0.5, -0.5, -0.5, 0, -1, 0, 0, 0,
                0.5, -0.5, -0.5, 0, -1, 0, 0, 0,
                0.5, -0.5, 0.5, 0, -1, 0, 0, 0,
                -0.5, -0.5, 0.5, 0, -1, 0, 0, 0,
            };

        Result.Vertices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                         sizeof(Vertices));
        void* GpuMemory = VkCommandsPushWrite(&RenderState->Commands, Result.Vertices, sizeof(Vertices),
                                              BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                              BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
        Copy(Vertices, GpuMemory, sizeof(Vertices));
    }
            
    {
        u32 Indices[] =
            {
                // NOTE: Front face
                0, 1, 2,
                2, 3, 0,

                // NOTE: Back face
                4, 5, 6,
                6, 7, 4,

                // NOTE: Left face
                8, 9, 10,
                10, 11, 8,

                // NOTE: Right face
                12, 13, 14,
                14, 15, 12,

                // NOTE: Top face
                16, 17, 18,
                18, 19, 16,

                // NOTE: Bottom face
                20, 21, 22,
                22, 23, 20,
            };

        Result.NumIndices = ArrayCount(Indices);
        Result.Indices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                        sizeof(Indices));
        void* GpuMemory = VkCommandsPushWrite(&RenderState->Commands, Result.Indices, sizeof(Indices),
                                              BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                              BarrierMask(VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
        Copy(Indices, GpuMemory, sizeof(Indices));
    }

    return Result;
}

inline procedural_mesh AssetsPushSphere(i32 NumXSegments, i32 NumYSegments)
{
    procedural_mesh Result = {};
    
    i32 VerticesSize = (NumXSegments + 1)*(NumYSegments + 1)*(2*sizeof(v3) + sizeof(v2));

    // NOTE: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/6.pbr/1.1.lighting/lighting.cpp
    Result.Vertices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VerticesSize);
    f32* Vertices = (f32*)VkCommandsPushWrite(&RenderState->Commands, Result.Vertices, VerticesSize,
                                              BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                              BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));

    for (i32 Y = 0; Y <= NumYSegments; ++Y)
    {
        for (i32 X = 0; X <= NumXSegments; ++X)
        {
            v2 Segment = V2(X, Y) / V2(NumXSegments, NumYSegments);
            v3 Pos = V3(Cos(Segment.x * 2.0f * Pi32) * Sin(Segment.y * Pi32),
                        Cos(Segment.y * Pi32),
                        Sin(Segment.x * 2.0f * Pi32) * Sin(Segment.y * Pi32));

            // NOTE: Write position
            *Vertices++ = Pos.x;
            *Vertices++ = Pos.y;
            *Vertices++ = Pos.z;

            // NOTE: Write normal
            *Vertices++ = Pos.x;
            *Vertices++ = Pos.y;
            *Vertices++ = Pos.z;

            // NOTE: Write uv
            *Vertices++ = Segment.x;
            *Vertices++ = Segment.y;
        }
    }

    Result.NumIndices = NumYSegments*(NumXSegments + 1)*6;
    Result.Indices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    Result.NumIndices*sizeof(u32));
    u32* Indices = VkCommandsPushWriteArray(&RenderState->Commands, Result.Indices, u32, Result.NumIndices,
                                            BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                            BarrierMask(VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));

    u32* CurrIndex = Indices;
    for (i32 Y = 0; Y < NumYSegments; ++Y)
    {
        for (i32 X = 0; X <= NumXSegments; ++X)
        {
            *CurrIndex++ = (Y+1)*(NumXSegments + 1) + X;
            *CurrIndex++ = (Y+1)*(NumXSegments + 1) + X+1;
            *CurrIndex++ = (Y)*(NumXSegments + 1) + X+1;

            *CurrIndex++ = (Y)*(NumXSegments + 1) + X+1;
            *CurrIndex++ = (Y)*(NumXSegments + 1) + X;
            *CurrIndex++ = (Y+1)*(NumXSegments + 1) + X;
        }
    }

    return Result;
}

/*
    NOTE: For rendering, we want multiple materials per model. We can either have the material id
          per vertex, or we can break the model up into prim sets, each with one material.

          Decision: Break the model up. I don't think for regular models that we care about blending
                    materials in the art pipeline. This may change, but I don't think it will.

          Then, we have the following data:

          Model: Has a array of meshes, each with a corresponding material. Each mesh stores a array
                 of vertices, each material has BRDF data in it.

          Obj files are annoying because they account for everything. They are not compact in their
          storage of model data.

          We can have multiple objects + object groups in a obj file. For this loader, we will ignore
          these completely.

          The file can declare materials during the face part, and it can declare the same material
          twice in different places. The loader has to aggregate everything then. 
 */

// TODO: This is a hack rn, do this better
#undef internal
#undef global
#undef local_global

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define internal static
#define global static
#define local_global static

internal procedural_mesh AssetsPushModel(char* ModelPath)
{
    // IMPORTANT: Its assumed that this is called inside of a tranfser operation
    // NOTE: https://learnopengl.com/Model-Loading/Model
    procedural_mesh Result = {};
    
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(ModelPath, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_GenSmoothNormals);
    if(!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) 
    {
        const char* Error = Importer.GetErrorString();
        InvalidCodePath;
    }

    // NOTE: Count number of vertices + flatten everything to one mesh
    u64 TotalNumVertices = 0;
    {
        for (u64 MeshId = 0; MeshId < Scene->mNumMeshes; MeshId++)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];
            TotalNumVertices += SrcMesh->mNumVertices;

            for (u32 FaceId = 0; FaceId < SrcMesh->mNumFaces; FaceId++)
            {
                aiFace Face = SrcMesh->mFaces[FaceId];

                Result.NumIndices += Face.mNumIndices;
            }
        }
    }

    // NOTE: Create GPU buffers
    vertex* Vertices = {};
    u32* Indices = {};
    {
        Result.Vertices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                         sizeof(vertex)*TotalNumVertices);
        Vertices = VkCommandsPushWriteArray(&RenderState->Commands, Result.Vertices, vertex, TotalNumVertices,
                                            BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                            BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));

        Result.Indices = VkBufferCreate(RenderState->Device, &RenderState->GpuArena, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                        sizeof(u32)*Result.NumIndices);
        Indices = VkCommandsPushWriteArray(&RenderState->Commands, Result.Indices, u32, Result.NumIndices,
                                           BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                           BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
    }
    
    // NOTE: Write Vertices
    {
        // NOTE: Load vertices
        f32 MaxPosAxis = 0.0f;
        vertex* Vertex = Vertices;
        for (u32 MeshId = 0; MeshId < Scene->mNumMeshes; MeshId++)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];

            // NOTE: Process vertices
            Assert(SrcMesh->mNumUVComponents[1] == 0);
            for (u32 VertId = 0; VertId < SrcMesh->mNumVertices; ++VertId, ++Vertex)
            {
                Vertex->Pos = V3(SrcMesh->mVertices[VertId].x,
                                 SrcMesh->mVertices[VertId].y,
                                 SrcMesh->mVertices[VertId].z);
             
                // NOTE: We mul by 2.0f to map to -0.5, 0.5 range, not -1, 1 range
                MaxPosAxis = Max(MaxPosAxis, 2.0f*Abs(Vertex->Pos.x));
                MaxPosAxis = Max(MaxPosAxis, 2.0f*Abs(Vertex->Pos.y));
                MaxPosAxis = Max(MaxPosAxis, 2.0f*Abs(Vertex->Pos.z));

                Vertex->Normal = V3(SrcMesh->mNormals[VertId].x,
                                    SrcMesh->mNormals[VertId].y,
                                    SrcMesh->mNormals[VertId].z);

                if (SrcMesh->mTextureCoords[0])
                {
                    Vertex->Uv = V2(SrcMesh->mTextureCoords[0][VertId].x,
                                    SrcMesh->mTextureCoords[0][VertId].y);
                }
                else
                {
                    // NOTE: Some meshes have no tex coords for some reason
                    Vertex->Uv = V2(0, 0);
                }
            }
        }
    
        // NOTE: Normalize all positions of the model
        Assert(MaxPosAxis != 0.0f);
        Vertex = Vertices;
        for (u32 PosId = 0; PosId < TotalNumVertices; ++PosId, ++Vertex)
        {
            // NOTE: This maps the model to be in -0.5, 0.5 space
            Vertex->Pos = Vertex->Pos / (MaxPosAxis);
        }
    }

    // NOTE: Write indicies
    {
        u32 IndexOffset = 0;
        u32 TmpIndexOffset = 0;
        u32* Index = Indices;
        for (u32 MeshId = 0; MeshId < Scene->mNumMeshes; MeshId++)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];
            
            // NOTE: Process indicies
            for (u32 FaceId = 0; FaceId < SrcMesh->mNumFaces; FaceId++)
            {
                aiFace Face = SrcMesh->mFaces[FaceId];
                for (u32 IndexId = 0; IndexId < Face.mNumIndices; IndexId++)
                {
                    *Index++ = Face.mIndices[IndexId] + IndexOffset;
                }

                TmpIndexOffset += Face.mNumIndices;
            }

            IndexOffset = TmpIndexOffset;
        }
    }
        
    return Result;
}

#if 0

//
// NOTE: Animation loading functions
//

inline m4 AssimpLoadM4(aiMatrix4x4t<f32> Mat)
{
    m4 Result = {};
    
    // NOTE: Copy over matrix (converting row major to column major)
    Result.e[0] = Mat.a1;
    Result.e[1] = Mat.b1;
    Result.e[2] = Mat.c1;
    Result.e[3] = Mat.d1;

    Result.e[4] = Mat.a2;
    Result.e[5] = Mat.b2;
    Result.e[6] = Mat.c2;
    Result.e[7] = Mat.d2;

    Result.e[8] = Mat.a3;
    Result.e[9] = Mat.b3;
    Result.e[10] = Mat.c3;
    Result.e[11] = Mat.d3;

    Result.e[12] = Mat.a4;
    Result.e[13] = Mat.b4;
    Result.e[14] = Mat.c4;
    Result.e[15] = Mat.d4;

    return Result;
}

inline i32 AssimpGetLoadedBoneId(loader_model* Model, string BoneName)
{
    i32 Result = -1;
    for (i32 StoredBoneId = 0; StoredBoneId < (i32)Model->NumBones; ++StoredBoneId)
    {
        if (StringsEqual(BoneName, Model->BoneArray[StoredBoneId].Name))
        {
            Result = StoredBoneId;
            break;
        }
    }

    return Result;
}

inline void AssimpLoadBoneTransforms(loader_model* Model, m4* DstBoneTransforms, u32 FrameId,
                                     aiAnimation* AiAnimation, aiNode* AiNode, m4 ParentTransform,
                                     u32* NumBonesWritten)
{
    string NodeName = String(AiNode->mName.data);
    i32 DstBoneId = AssimpGetLoadedBoneId(Model, NodeName);
    
    m4 NodeTransform = {};
    if (DstBoneId == -1)
    {
        // NOTE: This node is just a transform
        NodeTransform = AssimpLoadM4(AiNode->mTransformation);
    }
    else
    {
        // NOTE: This node is a bone, find our transform for this frame
        aiNodeAnim* Channel = 0;
        for (u32 NodeAnimId = 0; NodeAnimId < AiAnimation->mNumChannels; NodeAnimId++)
        {
            aiNodeAnim* AiNodeAnim = AiAnimation->mChannels[NodeAnimId];
            if (StringsEqual(NodeName, String(AiNodeAnim->mNodeName.data)))
            {
                Channel = AiNodeAnim;
            }
        }
        Assert(Channel != 0);

#define ASSIMP_LOAD_ATTRIBUTE(AttribName)                               \
        if (Channel->mNum##AttribName##Keys == 1)                       \
        {                                                               \
            Ai##AttribName##Key = Channel->m##AttribName##Keys[0];      \
        }                                                               \
        else                                                            \
        {                                                               \
            for (u32 AttribName##FrameId = 0;                           \
                 AttribName##FrameId < Channel->mNum##AttribName##Keys; \
                 AttribName##FrameId++)                                 \
            {                                                           \
                if ((f32)FrameId <= (f32)Channel->m##AttribName##Keys[AttribName##FrameId].mTime) \
                {                                                       \
                    Ai##AttribName##Key = Channel->m##AttribName##Keys[AttribName##FrameId]; \
                        break;                                          \
                }                                                       \
            }                                                           \
        }

        aiVectorKey AiPositionKey = {};
        ASSIMP_LOAD_ATTRIBUTE(Position);

        aiVectorKey AiScalingKey = {};
        ASSIMP_LOAD_ATTRIBUTE(Scaling);

        aiQuatKey AiRotationKey = {};
        ASSIMP_LOAD_ATTRIBUTE(Rotation);
        
#undef ASSIMP_LOAD_ATTRIBUTE
                    
        v3 Pos = V3(AiPositionKey.mValue.x, AiPositionKey.mValue.y, AiPositionKey.mValue.z) / Model->MaxPosAxis;
        v3 Scale = V3(AiScalingKey.mValue.x, AiScalingKey.mValue.y, AiScalingKey.mValue.z);
        q4 Rotation = Q4(AiRotationKey.mValue.x, AiRotationKey.mValue.y, AiRotationKey.mValue.z, AiRotationKey.mValue.w);

        NodeTransform = M4Pos(Pos)*Q4ToM4(Rotation)*M4Scale(Scale);
        
        *NumBonesWritten += 1;
    }
    
    m4 GlobalTransform = ParentTransform * NodeTransform;
    if (DstBoneId != -1)
    {
        loader_bone* Bone = Model->BoneArray + DstBoneId;
        m4 FinalTransform = Model->GlobalInverseTransform * GlobalTransform * Bone->ModelToBone;
        DstBoneTransforms[DstBoneId] = FinalTransform;
    }

    for (u32 ChildId = 0; ChildId < AiNode->mNumChildren; ChildId++)
    {
        AssimpLoadBoneTransforms(Model, DstBoneTransforms, FrameId, AiAnimation, AiNode->mChildren[ChildId],
                                 GlobalTransform, NumBonesWritten);
    }
}

internal render_model AssimpLoadModel(char* ModelPath)
{
    // IMPORTANT: Its assumed that this is called inside of a tranfser operation
    // NOTE: https://learnopengl.com/Model-Loading/Model
    render_model Result = {};
    loader_model LoaderModel = {};
    
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(ModelPath, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_GenSmoothNormals);
    if(!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) 
    {
        const char* Error = Importer.GetErrorString();
        InvalidCodePath;
    }

    // NOTE: Count number of vertices + populate some mesh meta data
    u64 TotalNumIndices = 0;
    u64 TotalNumVertices = 0;
    {
        Result.NumMeshes = Scene->mNumMeshes;
        Result.MeshArray = PushArray(&RenderState->CpuArena, render_mesh, Result.NumMeshes);

        for (u64 MeshId = 0; MeshId < Scene->mNumMeshes; MeshId++)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];
            render_mesh* DstMesh = Result.MeshArray + MeshId;
            *DstMesh = {};

            DstMesh->NumVertices = SrcMesh->mNumVertices;
            TotalNumVertices += SrcMesh->mNumVertices;
            
            DstMesh->MaterialId = SrcMesh->mMaterialIndex;

            // NOTE: Count num indices
            for (u32 FaceId = 0; FaceId < SrcMesh->mNumFaces; FaceId++)
            {
                aiFace Face = SrcMesh->mFaces[FaceId];

                DstMesh->NumIndices += Face.mNumIndices;
            }
            TotalNumIndices += DstMesh->NumIndices;
        }
    }

    // NOTE: Create GPU buffers
    b32 HasAnimations = Scene->mNumAnimations > 0;
    linear_arena VertexArena = {};
    {
        // TODO: This is currently hardcoded. Make it recognize the data from the model
        u64 VerticesSize = 2*sizeof(v3) + sizeof(v2);
        if (HasAnimations)
        {
            VerticesSize += sizeof(v4) + sizeof(v4u);
        }
        VerticesSize *= TotalNumVertices;
        
        Result.Vertices = VkBufferCreate(RenderState->Device, &RenderState->HostArena, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                         VerticesSize);
        u8* VerticesData = VkTransferPushWriteArray(&RenderState->TransferManager, Result.Vertices, u8, VerticesSize,
                                                    BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                                    BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
        VertexArena = LinearArenaCreate(VerticesData, VerticesSize);
    }
    
    // NOTE: Load mesh data
    v4u* BoneIdArray = 0;
    v4* BoneWeightArray = 0;
    {
        // NOTE: Allocate geometry
        v3* PosArray = PushArray(&VertexArena, v3, TotalNumVertices);
        v3* NormalArray = PushArray(&VertexArena, v3, TotalNumVertices);
        v2* TexCoordArray = PushArray(&VertexArena, v2, TotalNumVertices);
        if (HasAnimations)
        {
            BoneIdArray = PushArray(&VertexArena, v4u, TotalNumVertices);
            BoneWeightArray = PushArray(&VertexArena, v4, TotalNumVertices);
            
            // NOTE: Set default values on the bone arrays
            ZeroMem(BoneIdArray, sizeof(v4u)*TotalNumVertices);
            ZeroMem(BoneWeightArray, sizeof(v4)*TotalNumVertices);
        }
    
        // NOTE: Load vertices
        u32 CurrVertId = 0;
        for (u32 MeshId = 0; MeshId < Scene->mNumMeshes; MeshId++)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];

            // NOTE: Process vertices
            Assert(SrcMesh->mNumUVComponents[1] == 0);
            for (u32 VertId = 0; VertId < SrcMesh->mNumVertices; ++VertId, ++CurrVertId)
            {
                v3 WritePos = V3(SrcMesh->mVertices[CurrVertId].x,
                                 SrcMesh->mVertices[CurrVertId].y,
                                 SrcMesh->mVertices[CurrVertId].z);
                PosArray[CurrVertId] = WritePos;

                // NOTE: We mul by 2.0f to map to -0.5, 0.5 range, not -1, 1 range
                LoaderModel.MaxPosAxis = Max(LoaderModel.MaxPosAxis, 2.0f*Abs(WritePos.x));
                LoaderModel.MaxPosAxis = Max(LoaderModel.MaxPosAxis, 2.0f*Abs(WritePos.y));
                LoaderModel.MaxPosAxis = Max(LoaderModel.MaxPosAxis, 2.0f*Abs(WritePos.z));
                
                NormalArray[CurrVertId] = V3(SrcMesh->mNormals[VertId].x,
                                             SrcMesh->mNormals[VertId].y,
                                             SrcMesh->mNormals[VertId].z);

                if (SrcMesh->mTextureCoords[0])
                {
                    TexCoordArray[CurrVertId] = V2(SrcMesh->mTextureCoords[0][VertId].x,
                                                   SrcMesh->mTextureCoords[0][VertId].y);
                }
                else
                {
                    // NOTE: Some meshes have no tex coords for some reason
                    TexCoordArray[CurrVertId] = V2(0, 0);
                }
            }
        }
    
        // NOTE: Normalize all positions of the model
        Assert(LoaderModel.MaxPosAxis != 0.0f);
        for (u32 PosId = 0; PosId < TotalNumVertices; ++PosId)
        {
            // NOTE: This maps the model to be in -0.5, 0.5 space
            PosArray[PosId] = PosArray[PosId] / (LoaderModel.MaxPosAxis);
        }
    }

    // NOTE: Allocate + Load indicies
    {
        Result.Indices = VkBufferCreate(RenderState->Device, &RenderState->HostArena, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                        sizeof(u32)*TotalNumIndices);
        u32* IndexData = VkTransferPushWriteArray(&RenderState->TransferManager, Result.Indices, u32, TotalNumIndices,
                                                  BarrierMask(VkAccessFlagBits(0), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                                  BarrierMask(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT));
        u32* CurrIndex = IndexData;

        for (u32 MeshId = 0; MeshId < Scene->mNumMeshes; MeshId++)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];
            render_mesh* DstMesh = Result.MeshArray + MeshId;

            // NOTE: Process indicies
            for (u32 FaceId = 0; FaceId < SrcMesh->mNumFaces; FaceId++)
            {
                aiFace Face = SrcMesh->mFaces[FaceId];

                DstMesh->NumIndices += Face.mNumIndices;
                for (u32 IndexId = 0; IndexId < Face.mNumIndices; IndexId++)
                {
                    *CurrIndex++ = Face.mIndices[IndexId];
                }
            }
        }
    }
    
    if (HasAnimations)
    {
#if 0
        // TODO: Fix this guy
        // NOTE: Find the root ai node for bones
        aiNode* AiBoneRootNode = 0;
        for (u32 ChildId = 0; ChildId < Scene->mRootNode->mNumChildren; ++ChildId)
        {
            aiNode* Child = Scene->mRootNode->mChildren[ChildId];
            if (StringsEqual(String("Armature"), String(Child->mName.data)))
            {
                AiBoneRootNode = Child;
                break;
            }
        }
    
        // NOTE: Load bones metadata
        Result.BoneArray = (loader_bone*)(State.TempArena.Mem + State.TempArena.Used);
        {
            // IMPORTANT: This is hacky but to prevent having a separate loop to count how many bones we have, we write into memory and
            // then reserve the allocation after
            for (u32 MeshId = 0; MeshId < Scene->mNumMeshes; ++MeshId)
            {
                aiMesh* SrcMesh = Scene->mMeshes[MeshId];
                for (u32 BoneId = 0; BoneId < SrcMesh->mNumBones; ++BoneId)
                {
                    aiBone* AiBone = SrcMesh->mBones[BoneId];
                    string BoneName = String(AiBone->mName.data);

                    // NOTE: Check if we already added this bone
                    i32 CurrGlobalBoneId = AssimpGetLoadedBoneId(&Result, BoneName);
                    if (CurrGlobalBoneId == -1)
                    {
                        // NOTE: This is a new bone so append it
                        CurrGlobalBoneId = Result.NumBones++;
                        loader_bone* DstBone = Result.BoneArray + CurrGlobalBoneId;

                        // NOTE: Load the bones metadata
                        DstBone->Name = BoneName;
                        DstBone->ModelToBone = AssimpLoadM4(AiBone->mOffsetMatrix);

                        // NOTE: Scale the ModelToBones pos relative to our normalization const
                        DstBone->ModelToBone.v[3].xyz /= Result.MaxPosAxis;
                    }            
                }
            }

            Assert(Result.NumBones > 0);
            Result.BoneArray = PushArray(&State.TempArena, loader_bone, Result.NumBones);
        }
        
        // NOTE: Load per vertex bone data
        u32 CurrVertId = 0;
        for (u32 MeshId = 0; MeshId < Scene->mNumMeshes; ++MeshId)
        {
            aiMesh* SrcMesh = Scene->mMeshes[MeshId];

            for (u32 BoneId = 0; BoneId < SrcMesh->mNumBones; ++BoneId)
            {
                aiBone* AiBone = SrcMesh->mBones[BoneId];

                string BoneName = String(AiBone->mName.data);
                i32 CurrGlobalBoneId = AssimpGetLoadedBoneId(&Result, BoneName);
                Assert(CurrGlobalBoneId != -1);

                // NOTE: Populate per vertex data
                for (u32 WeightId = 0; WeightId < AiBone->mNumWeights; WeightId++)
                {
                    u32 NewVertexId = AiBone->mWeights[WeightId].mVertexId + CurrVertId;
                    f32 NewWeight = AiBone->mWeights[WeightId].mWeight;

                    for (u32 StoreId = 0; StoreId < 4; ++StoreId)
                    {
                        u32* StoredBoneId = BoneIdArray[NewVertexId].e + StoreId;
                        f32* StoredWeight = BoneWeightArray[NewVertexId].e + StoreId;

                        if (NewWeight > *StoredWeight)
                        {
                            // NOTE: Shift all other bone ids and weights down by 1
                            u32 OldBoneId = *StoredBoneId;
                            f32 OldWeight = *StoredWeight;
                            for (u32 I = StoreId + 1; I < 4; ++I)
                            {
                                u32 TempBoneId = BoneIdArray[NewVertexId].e[I];
                                f32 TempWeight = BoneWeightArray[NewVertexId].e[I];

                                BoneIdArray[NewVertexId].e[I] = OldBoneId;
                                BoneWeightArray[NewVertexId].e[I] = OldWeight;

                                OldBoneId = TempBoneId;
                                OldWeight = TempWeight;
                            }
                            
                            // NOTE: Put in our new value
                            *StoredBoneId = (u32)CurrGlobalBoneId;
                            *StoredWeight = NewWeight;

                            break;
                        }
                    }
                }
            }
            
            CurrVertId += SrcMesh->mNumVertices;
        }
        
        // NOTE: Pass through all our vertices and make sure that the chosen bones weigh up to 1
        for (u32 VertexId = 0; VertexId < TotalNumVertices; ++VertexId)
        {
            v4 BoneWeights = BoneWeightArray[VertexId];
            f32 TotalWeight = BoneWeights.e[0] + BoneWeights.e[1] + BoneWeights.e[2] + BoneWeights.e[3];
            BoneWeightArray[VertexId] = BoneWeights / TotalWeight;
        }

        // NOTE: Load animations (we store frames and transforms in one segment for each animation)
        Result.GlobalInverseTransform = Inverse(AssimpLoadM4(AiBoneRootNode->mTransformation));
        Result.NumAnimations = Scene->mNumAnimations;
        file_animation* AnimationArray = StatePushArray(file_animation, Result.NumAnimations, &Result.AnimationOffset);
        
        Assert(Result.NumAnimations == NumAnimationMappings);
        
        aiAnimation** SrcAnimation_ = Scene->mAnimations;
        file_animation* DstAnimation = AnimationArray;
        u32 SavedNumAnimations = Result.NumAnimations;
        for (u32 AnimationId = 0; AnimationId < SavedNumAnimations; ++AnimationId, ++SrcAnimation_, ++DstAnimation)
        {
            if (AnimationMappings[AnimationId] == AnimationId_None)
            {
                Result.NumAnimations -= 1;
                DstAnimation -= 1;
                continue;
            }

            aiAnimation* SrcAnimation = *SrcAnimation_;
            
            DstAnimation->AnimationId = AnimationMappings[AnimationId];
            DstAnimation->TotalTime = (f32)SrcAnimation->mDuration/(f32)SrcAnimation->mTicksPerSecond;
            DstAnimation->NumFrames = (u32)SrcAnimation->mDuration;
            
            // NOTE: Load animation frames
            file_animation_frame* FrameArray = StatePushArray(file_animation_frame, DstAnimation->NumFrames, &DstAnimation->FrameOffset);
            for (u32 FrameId = 0; FrameId < DstAnimation->NumFrames; ++FrameId)
            {
                file_animation_frame* Frame = FrameArray + FrameId;
                Frame->TimeOffset = (f32)FrameId / (f32)SrcAnimation->mTicksPerSecond;
            }

            // NOTE: Load transforms
            Result.AnimationTransformSize += sizeof(m4)*Result.NumBones*DstAnimation->NumFrames;
            DstAnimation->TransformOffset = State.CurrentDataOffset;
            for (u32 FrameId = 0; FrameId < DstAnimation->NumFrames; ++FrameId)
            {
                m4* BoneTransforms = StatePushArray(m4, Result.NumBones, 0);
            
                u32 NumBonesWritten = 0;
                AssimpLoadBoneTransforms(&Result, BoneTransforms, FrameId, SrcAnimation, AiBoneRootNode, M4Identity(), &NumBonesWritten);
                Assert(NumBonesWritten == Result.NumBones);
            }
        }
#endif
    }
    
    return Result;
}

#if 0
inline void ModelCountNumAssets(char* ModelPath, char* ParentFolder, char* DefaultTextureName, u32* OutNumMaterials,
                                u32* OutNumUnIndexedTextures)
{
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(ModelPath, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_GenSmoothNormals);
    if(!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) 
    {
        const char* Error = Importer.GetErrorString();
        InvalidCodePath;
    }
    
    // TODO: Make sure we aren't loading the same textures more than once
    // TODO: Sometimes some materials are unused. Delete those
    // NOTE: Load materials
    // TODO: When we add a texture job, make sure its unique
    asset_texture_id DiffuseTextureId = AssetTextureId(true, Texture_White);
    if (DefaultTextureName)
    {
        DiffuseTextureId = AddUnIndexedTextureAsset(ParentFolder, DefaultTextureName);
    }
    
    for (u32 MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* AiMaterial = Scene->mMaterials[MaterialId];

        b32 IsIndexed = true;
        u32 NumDiffuseTextures = AiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
        if (NumDiffuseTextures > 0)
        {
            aiString DiffuseStr;
            AiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &DiffuseStr);

            IsIndexed = false;
            char* DiffuseStrCopy = (char*)malloc(sizeof(char)*(strlen(DiffuseStr.C_Str()) + 1));
            strcpy(DiffuseStrCopy, DiffuseStr.C_Str());
            DiffuseTextureId = AddUnIndexedTextureAsset(ParentFolder, DiffuseStrCopy);
        }

        AddUnIndexedMaterialAsset(DiffuseTextureId);
    }

    *OutNumMaterials = Scene->mNumMaterials;
}
#endif

#endif
