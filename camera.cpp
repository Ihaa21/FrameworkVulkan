/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

inline camera CameraCreate_(v3 Pos, v3 View, v3 Up, v3 Right, b32 IsPerspective)
{
    camera Result = {};

    Result.IsPerspective = IsPerspective;
    Result.Pos = Pos;
    Result.View = View;
    Result.Up = Up;
    Result.Right = Right;

    Result.GpuBuffer = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      sizeof(camera_input));

    return Result;
}

inline camera CameraFpsCreate(v3 Pos, v3 View, b32 IsPerspective, f32 TurningVelocity, f32 Velocity)
{
    // NOTE: https://github.com/Erkaman/gl-movable-camera
    v3 Up = V3(0, 1, 0);
    camera Result = CameraCreate_(Pos, View, Up, Normalize(Cross(Up, View)), IsPerspective);

    Result.Type = CameraType_Fps;
    Result.Fps.TurningVelocity = TurningVelocity;
    Result.Fps.Velocity = Velocity;

    return Result;
}

inline camera CameraTopDownCreate(v3 Pos, f32 Angle, b32 IsPerspective, f32 MoveVelocity, f32 ZoomVelocity)
{
    q4 Orientation = Q4AxisAngle(V3(1, 0, 0), Angle);
    v3 View = RotateVec(V3(0, 0, 1), Orientation);
    v3 Up = RotateVec(V3(0, 1, 0), Orientation);
    v3 Right = RotateVec(V3(1, 0, 0), Orientation);
    
    camera Result = CameraCreate_(Pos, View, Up, Right, IsPerspective);
    
    Result.Type = CameraType_TopDown;
    Result.TopDown.Angle = Angle;
    Result.TopDown.MoveVelocity = MoveVelocity;
    Result.TopDown.ZoomVelocity = ZoomVelocity;

    return Result;
}

inline camera CameraFlatCreate(v3 Pos, f32 Radius, f32 MoveVelocity, f32 ZoomVelocity, f32 Near, f32 Far)
{
    v3 View = V3(0, 0, 1);
    v3 Up = V3(0, 1, 0);
    v3 Right = V3(1, 0, 0);
    
    camera Result = CameraCreate_(Pos, View, Up, Right, false);
    
    Result.Type = CameraType_Flat;
    Result.Flat.MoveVelocity = MoveVelocity;
    Result.Flat.ZoomVelocity = ZoomVelocity;

    f32 OrthoRadiusX = Radius;
    f32 OrthoRadiusY = OrthoRadiusX / RenderState->WindowAspectRatio;
    CameraSetOrtho(&Result, -OrthoRadiusX, OrthoRadiusX, OrthoRadiusY, -OrthoRadiusY, Near, Far);

    return Result;
}

inline void CameraSetPersp(camera* Camera, f32 AspectRatio, f32 Fov, f32 Near, f32 Far)
{
    Camera->PerspNear = Near;
    Camera->PerspFar = Far;
    Camera->PerspAspectRatio = AspectRatio;
    Camera->PerspFov = DegreeToRadians(Fov);
}

inline void CameraSetOrtho(camera* Camera, f32 Left, f32 Right, f32 Top, f32 Bottom, f32 Near, f32 Far)
{
    Camera->OrthoNear = Near;
    Camera->OrthoFar = Far;
    Camera->OrthoLeft = Left;
    Camera->OrthoRight = Right;
    Camera->OrthoTop = Top;
    Camera->OrthoBottom = Bottom;
}

inline m4 CameraGetP(camera* Camera)
{
    m4 Result = {};
    if (Camera->IsPerspective)
    {
        Result = VkPerspProjM4(Camera->PerspAspectRatio, Camera->PerspFov, Camera->PerspNear, Camera->PerspFar);
    }
    else
    {
        Result = VkOrthoProjM4(Camera->OrthoLeft, Camera->OrthoRight, Camera->OrthoTop, Camera->OrthoBottom, Camera->OrthoNear, Camera->OrthoFar);
    }
    return Result;
}

inline m4 CameraGetV(camera* Camera)
{
    m4 Result = LookAtM4(Camera->View, Camera->Up, Camera->Right, Camera->Pos);
    return Result;
}

inline m4 CameraGetVP(camera* Camera)
{
    m4 Result = {};
    m4 VTransform = CameraGetV(Camera);
    m4 PTransform = CameraGetP(Camera);

    Result = PTransform*VTransform;

    return Result;
}

inline void CameraUpdate(camera* Camera, frame_input* CurrInput, frame_input* PrevInput, f32 FrameTime)
{
    // TODO: Add frame time mul to all these vels
    // NOTE: Apply camera rotation
    switch (Camera->Type)
    {
        case CameraType_Fps:
        {
            v3 NewView = Camera->View;
            v3 NewRight = Camera->Right;
            if (CurrInput->MouseDown)
            {
                f32 Head = (f32)(CurrInput->MouseNormalizedPos.x - PrevInput->MouseNormalizedPos.x);
                f32 Pitch = (f32)(CurrInput->MouseNormalizedPos.y - PrevInput->MouseNormalizedPos.y);

                // NOTE: Rotate about the up vector and right vector
                NewView = RotateVectorAroundAxis(Camera->View, Camera->Up, -Head*Camera->Fps.TurningVelocity);
                NewView = RotateVectorAroundAxis(NewView, Camera->Right, -Pitch*Camera->Fps.TurningVelocity);
        
                // NOTE: Update right vector
                NewRight = Normalize(Cross(Camera->Up, NewView));
            }
    
            // NOTE: Apply camera translation
            b32 MoveForward = CurrInput->KeysDown['W'];
            b32 MoveLeft = CurrInput->KeysDown['A'];
            b32 MoveBackward = CurrInput->KeysDown['S'];
            b32 MoveRight = CurrInput->KeysDown['D'];
            b32 SpeedUp = CurrInput->KeysDown['M'];
            b32 SlowDown = CurrInput->KeysDown['N'];
            
            if (SpeedUp)
            {
                Camera->Fps.Velocity *= 1.01f;
            }
            if (SlowDown)
            {
                Camera->Fps.Velocity /= 1.01f;
                Camera->Fps.Velocity = Max(Camera->Fps.Velocity, 0.00001f);
            }
    
            f32 Velocity = Camera->Fps.Velocity;
            v3 MoveVel = {};
            if (MoveForward)
            {
                MoveVel += Velocity*Camera->View;
            }
            if (MoveBackward)
            {
                MoveVel -= Velocity*Camera->View;
            }

            if (MoveRight)
            {
                MoveVel += Velocity*Camera->Right;
            }
            if (MoveLeft)
            {
                MoveVel -= Velocity*Camera->Right;
            }
    
            Camera->Pos += MoveVel;

            Camera->View = NewView;
            Camera->Right = NewRight;
            Camera->Up = Normalize(Cross(NewView, NewRight));
        } break;

        case CameraType_TopDown:
        {
            // NOTE: Setup camera angle
            q4 Orientation = Q4AxisAngle(V3(1, 0, 0), Camera->TopDown.Angle);
            Camera->View = RotateVec(V3(0, 0, 1), Orientation);
            Camera->Up = RotateVec(V3(0, 1, 0), Orientation);
            Camera->Right = RotateVec(V3(1, 0, 0), Orientation);
            
            // NOTE: Apply camera translation
            b32 MoveForward = CurrInput->KeysDown['W'];
            b32 MoveLeft = CurrInput->KeysDown['A'];
            b32 MoveBackward = CurrInput->KeysDown['S'];
            b32 MoveRight = CurrInput->KeysDown['D'];
            
            f32 Velocity = Camera->TopDown.MoveVelocity;
            v3 MoveVel = {};
            if (MoveForward)
            {
                MoveVel += Velocity*V3(0, 0, 1);
            }
            if (MoveBackward)
            {
                MoveVel -= Velocity*V3(0, 0, 1);
            }

            if (MoveRight)
            {
                MoveVel += Velocity*V3(1, 0, 0);
            }
            if (MoveLeft)
            {
                MoveVel -= Velocity*V3(1, 0, 0);
            }
    
            Camera->Pos += MoveVel;
        } break;

        case CameraType_Flat:
        {
            // NOTE: Apply camera zoom
            f32 ZoomChange = -CurrInput->MouseScroll * Camera->Flat.ZoomVelocity * FrameTime;
            
            if (ZoomChange != 0.0f)
            {
                f32 OrthoRadiusX = Camera->OrthoRight + ZoomChange;
                f32 OrthoRadiusY = OrthoRadiusX / RenderState->WindowAspectRatio;

                Camera->OrthoLeft = -OrthoRadiusX;
                Camera->OrthoRight = OrthoRadiusX;
                Camera->OrthoTop = OrthoRadiusY;
                Camera->OrthoBottom = -OrthoRadiusY;
            }
            
            // NOTE: Apply camera translation
            b32 MoveUp = CurrInput->KeysDown['W'];
            b32 MoveLeft = CurrInput->KeysDown['A'];
            b32 MoveDown = CurrInput->KeysDown['S'];
            b32 MoveRight = CurrInput->KeysDown['D'];
            
            f32 Velocity = Camera->Flat.MoveVelocity;
            v3 MoveVel = {};
            if (MoveUp)
            {
                MoveVel += Velocity*V3(0, 1, 0);
            }
            if (MoveDown)
            {
                MoveVel -= Velocity*V3(0, 1, 0);
            }

            if (MoveRight)
            {
                MoveVel += Velocity*V3(1, 0, 0);
            }
            if (MoveLeft)
            {
                MoveVel -= Velocity*V3(1, 0, 0);
            }
    
            Camera->Pos += MoveVel * FrameTime;
        } break;
    }

    Assert(Abs(Dot(Camera->View, Camera->Right)) <= 0.0001f);
    Assert(Abs(Dot(Camera->Right, Camera->Up)) <= 0.0001f);
    Assert(Abs(Dot(Camera->Up, Camera->View)) <= 0.0001f);
}

#if 0
INPUT_INTERACTION_HANDLER(CameraHandleInteraction)
{
    b32 Result = false;

    if (!(InputState->Hot.Type == Interaction_DragCamera ||
          InputState->Hot.Type == Interaction_ZoomCamera))
    {
        return Result;
    }

    input_frame* PrevInput = &InputState->PrevInput;
    input_frame* CurrInput = &InputState->CurrInput;
    f32 FrameTime = InputState->CurrInput.FrameTime;
    ui_state* UiState = &GameState->UiState;
    
    switch (InputState->Hot.Type)
    {
        case Interaction_DragCamera:
        {
            drag_camera_interaction* HotDragCameraInteraction = &InputState->Hot.DragCamera;
            camera* Camera = HotDragCameraInteraction->Camera;
            input_pointer* MainPointer = &CurrInput->MainPointer;

            switch (Camera->Type)
            {
                case CameraType_Fps:
                {
                    drag_fps_camera DragFpsCamera = HotDragCameraInteraction->FpsCamera;

                    f32 Head = -(f32)(MainPointer->PixelPos.x - DragFpsCamera.PrevMousePixelPos.x);
                    f32 Pitch = -(f32)(MainPointer->PixelPos.y - DragFpsCamera.PrevMousePixelPos.y);

                    // NOTE: Rotate about the up vector and right vector
                    Camera->View = RotateVectorAroundAxis(Camera->View, Camera->Up, Head*Camera->FpsCamera.TurningVelocity);
                    Camera->View = RotateVectorAroundAxis(Camera->View, Camera->Right, Pitch*Camera->FpsCamera.TurningVelocity);

                    // NOTE: Update right vector
                    Camera->Right = Normalize(Cross(Camera->Up, Camera->View));
                        
                    InputState->PrevHot = InputState->Hot;
                } break;
                    
                default:
                {
                    InvalidCodePath;
                } break;
            }
            
            Result = true;
        } break;
    }

    return Result;
}
#endif
