/*

  NOTE: http://jamie-wong.com/2016/08/05/webgl-fluid-simulation/

    This method uses a velocity field to init how a fluid moves initially. For init, we use 2 fields, vector field and color field.

    Advection: This is the process of moving the fluid via time step based on the velocity field. If we take each grid center of our
               texture and move it, we get points that are misaligned. Instead, we use the velocity field on each sample pos to figure
               out where it came from. The initial point isn't grid aligned but if we do bilinear filtering to grab the point, then we get
               the result we want.

               For velocity field updating, we just advect the velocity field with itself.

    Divergent Fields: If we do the above naively, we get divergent fields. The problem is that water is a incompressible fluid and the
                      above doesn't model that aspect (water can stretch/shrink to infinity). Divergence is given by du_x / dx + du_y / dy.
                      We require this operator to equal to 0 everywhere.

    Navier Stokes: These equations model fluids, specifically we have this formula for incompressible fluids:

                   du/dt = -u * div(u) - 1/density * div(pressure) + v*div(u)^2 + F, div(u) = 0

                   u is the velocity field, v is kinematic viscosity, and F is any external forces. We are modelling a fluid without
                   viscosity so we get the following:

                   du/dt = -u * div(u) - 1/density * div(pressure), div(u) = 0

                   We can write the above in a matrix form giving us the following:

                   |du_x / dt| = -|du_x/dx, du_x/dy| * |ux| - 1/density * |dpressure/dx|
                   |du_y / dt|    |du_y/dx, du_y/dy|   |uy|               |dpressure/dy|

                   |du_x / dt| = -|u_x * du_x/dx - u_y * du_x/dy - 1/density * dpressure/dx|
                   |du_y / dt|    |u_x * du_y/dx - u_y * du_y/dy - 1/density * dpressure/dy|

                   We can approximate /dx and /dy with discrete versions. For example, du_x/dt ~ [u_x(x, y, t + dt) - u_x(x, y, t)] / dt
                   We can apply this to the whole above equation with the dx, dy and dt values. We want to find the next velocity values
                   so we get the new velocity, but we also will have to solve for pressure since its a unknown.

                   u_x(x,y,t+dt)/dt =   u_x(x,y,t)
                                      - u_x(x,y,t)*(u_x(x+epsilon,y,t) - u_x(x-epsilon,y,t)) / 2epsilon
                                      - u_y(x,y,t)*(u_x(x,y+epsilon,t) - u_x(x,y-epsilon,t)) / 2epsilon
                                      - 1/density * (pressure(x+epsilon,y,t) - pressure(x-epsilon,y,t)) / 2epsilon

                   We can replace the first 3 terms with velocity after we advec it.
                   * COME BACK TO THIS

    Pressure: 
                   

                   
  
 */

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#if ADVECTION_SHADER

#endif

#if DIVERGENCE_SHADER

#endif

#if PRESSURE_SHADER

#endif

#if FINALIZE_SHADER

#endif
