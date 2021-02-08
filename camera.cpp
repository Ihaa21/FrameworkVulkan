/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

inline camera CameraCreate_(v3 Pos, v3 View, v3 Up, v3 Right, f32 AspectRatio, f32 Near, f32 Far, f32 Fov)
{
    camera Result = {};

    // TODO: Use in shadow map in playstate?

    Result.AspectRatio = AspectRatio;
    Result.Near = Near;
    Result.Far = Far;
    Result.Fov = DegreeToRadians(Fov);
    
    Result.Pos = Pos;
    Result.View = View;
    Result.Up = Up;
    Result.Right = Right;

    Result.GpuBuffer = VkBufferCreate(RenderState->Device, &RenderState->GpuArena,
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      sizeof(camera_input));

    return Result;
}

inline camera CameraFpsCreate(v3 Pos, v3 View, f32 AspectRatio, f32 Near, f32 Far, f32 Fov, f32 TurningVelocity, f32 Velocity)
{
    // NOTE: https://github.com/Erkaman/gl-movable-camera
    v3 Up = V3(0, 1, 0);
    camera Result = CameraCreate_(Pos, View, Up, Normalize(Cross(Up, View)), AspectRatio, Near, Far, Fov);

    Result.Type = CameraType_Fps;
    Result.Fps.TurningVelocity = TurningVelocity;
    Result.Fps.Velocity = Velocity;

    return Result;
}

inline camera CameraTopDownCreate(v3 Pos, f32 Angle, f32 AspectRatio, f32 Near, f32 Far, f32 Fov, f32 MoveVelocity, f32 ZoomVelocity)
{
    q4 Orientation = Q4AxisAngle(V3(1, 0, 0), Angle);

    v3 View = RotateVec(V3(0, 0, 1), Orientation);
    v3 Up = RotateVec(V3(0, 1, 0), Orientation);
    v3 Right = RotateVec(V3(1, 0, 0), Orientation);
    
    camera Result = CameraCreate_(Pos, View, Up, Right, AspectRatio, Near, Far, Fov);

    Result.Type = CameraType_TopDown;
    Result.TopDown.MoveVelocity = MoveVelocity;
    Result.TopDown.ZoomVelocity = ZoomVelocity;

    return Result;
}

inline m4 CameraGetP(camera* Camera)
{
    m4 Result = VkPerspProjM4(Camera->AspectRatio, Camera->Fov, Camera->Near, Camera->Far);
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

inline void CameraUpdate(camera* Camera, frame_input* CurrInput, frame_input* PrevInput)
{
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
