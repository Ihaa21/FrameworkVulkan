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
};

struct fps_camera
{
    q4 Orientation;
    f32 TurningVelocity;
    f32 Velocity;
};

struct top_down_camera
{
    f32 MoveVelocity;
    f32 ZoomVelocity;
};

struct camera
{
    f32 AspectRatio;
    f32 Near;
    f32 Far;
    f32 Fov;
    
    v3 Pos;
    v3 View;
    v3 Up;
    v3 Right;

    camera_type Type;
    union
    {
        fps_camera Fps;
        top_down_camera TopDown;
    };

    VkBuffer GpuBuffer;
};

struct camera_input
{
    v3 CameraPos;
};
