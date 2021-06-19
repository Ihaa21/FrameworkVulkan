#pragma once

/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

//
// NOTE: Camera Interactions
//

struct drag_fps_camera
{
    v2 PrevMousePixelPos;
};

//
// NOTE: Camera Types
//

enum camera_type
{
    CameraType_None,

    CameraType_Fps,
    CameraType_TopDown,
    CameraType_Flat,
};

struct fps_camera
{
    q4 Orientation;
    f32 TurningVelocity;
    f32 Velocity;
};

struct top_down_camera
{
    f32 Angle;
    f32 MoveVelocity;
    f32 ZoomVelocity;
};

struct flat_camera
{
    f32 MoveVelocity;
    f32 ZoomVelocity;
};

struct camera
{
    b32 IsPerspective;

    f32 PerspNear;
    f32 PerspFar;
    f32 PerspAspectRatio;
    f32 PerspFov;
    
    f32 OrthoNear;
    f32 OrthoFar;
    f32 OrthoLeft;
    f32 OrthoRight;
    f32 OrthoTop;
    f32 OrthoBottom;
    
    v3 Pos;
    v3 View;
    v3 Up;
    v3 Right;

    camera_type Type;
    union
    {
        fps_camera Fps;
        top_down_camera TopDown;
        flat_camera Flat;
    };

    VkBuffer GpuBuffer;
};

struct camera_input
{
    v3 CameraPos;
};

inline void CameraSetOrtho(camera* Camera, f32 Left, f32 Right, f32 Top, f32 Bottom, f32 Near, f32 Far);
