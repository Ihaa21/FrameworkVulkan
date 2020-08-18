#pragma once

struct loader_mesh
{
    // NOTE: Geometry    
    u32 NumVertices;
    u32 NumIndices;

    // NOTE: Material
    u32 MaterialId;
};

struct loader_bone
{
    string Name;
    m4 ModelToBone;
};

struct loader_animation_frame
{
    f32 TimeOffset;
    m4* BoneTransforms;
};

struct loader_animation
{
    //animation_id AnimationId;
    f32 TotalTime;
    u32 NumFrames;
    loader_animation_frame* FrameArray;
};

struct loader_model
{
    // NOTE: Animation matrices
    m4 GlobalInverseTransform;

    f32 MaxPosAxis; // NOTE: Used for normalizing pos to be in [-1, 1] on each axis
    
    u32 NumBones;
    loader_bone* BoneArray;
    u32 NumAnimations;
    u32 AnimationTransformSize;
    u64 AnimationOffset;
};

