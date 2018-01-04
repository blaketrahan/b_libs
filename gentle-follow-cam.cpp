#include "multi_axis_camera.h"
#include "ICameraSceneNode.h"
#include "ISceneNode.h"
#include "ISceneManager.h"

using namespace irr;
using namespace core;
using namespace scene;

inline void add_time(const float elapsed_time, float &clock, const float divisor)
{
    clock += elapsed_time/divisor;
    if (clock >= 1.0) {
        clock = 1.0;
    }
}

inline void limted_orbit_reset_values(LimitedOrbitAxis &axis)
{
    axis.clock = 0.0;
    axis.inertia_clock = 0.0;
    axis.start = axis.current;
    axis.inertia_target = axis.current;
    axis.returning_to_origin = false;
}

inline void limted_orbit_inertia_decelerate(LimitedOrbitAxis &axis, const float elapsed_time)
{
    if (axis.inertia_clock == 0.0)
    {
        if (axis.inertia_target > axis.max[2]) {
            axis.inertia_target = axis.max[2];
        } else if (axis.inertia_target < axis.max[0]) {
            axis.inertia_target = axis.max[0];
        }
    }

    add_time(elapsed_time, axis.inertia_clock, axis.inertia_divisor);

    float v = axis.inertia_clock;
    v = 1.0 - (1.0 - v) * (1.0 - v);
    axis.current = (axis.inertia_target * v) + (axis.start * (1.0 - v));

    if (axis.inertia_clock >= 1.0)
    {
        limted_orbit_reset_values(axis);
    }
}

inline float smoothstep(float x)
{
    return ((x) * (x) * (3.0 - 2.0 * (x)));
}

inline void limted_orbit_interpolate(LimitedOrbitAxis &axis)
{
    float v = smoothstep(axis.clock);
    axis.current = (axis.target * v) + (axis.start * (1.0 - v));
    // 0.5 - (0.5 - clock): smoothstep behavior, accelerates until 0.5,
    // then decelerates. Therefore, 0.1 should behave like 0.9
    // Mulitipled by 1/3 the distance planning to be traveled for a faked inertia.
    axis.inertia_target = axis.current + ((0.5 - ( 0.5 - axis.clock )) * (0.3 * (axis.target - axis.start)));
}

inline void set_lerp_speed(LimitedOrbitAxis &axis)
{
    if (axis.returning_to_origin) {
        axis.divisor = axis.return_divisor;
    } else {
        // plots divisor between min_divisor and max_divisor by the distance to the target
        axis.divisor = axis.min_divisor
                      + (((fabs(axis.target - axis.current)) / (axis.max[2] - axis.max[0]))
                      * (axis.max_divisor - axis.min_divisor));
    }
}

static void limited_orbit_lerp(LimitedOrbitAxis &axis, const float elapsed_time,
                                 const bool moving = false, const bool rotating = false)
{
    if (moving)
    {
        // if player is trying to look somewhere other than forward AFTER moving
        if (axis.target != axis.max[1]/*origin*/)
        {
            axis.looking_while_moving = true;
            axis.returning_to_origin = false;
        }
        // player just started moving, so values needs to be reset
        else if (!axis.returning_to_origin && !axis.looking_while_moving)
        {
            limted_orbit_reset_values(axis);
            axis.returning_to_origin = true;
        }
    }

    if (rotating)
    {
        axis.looking_while_moving = false;
    }

    // don't return to origin when target is not pointing to origin
    if (axis.target != axis.max[1]/*origin*/ && axis.returning_to_origin)
    {
        axis.returning_to_origin = false;
    }

    // if there is no new target AND camera is not in process of returning to origin
    if (axis.target == axis.max[1]/*origin*/ && !axis.returning_to_origin)
    {
        // if there's inertia leftover, use it
        limted_orbit_inertia_decelerate(axis,elapsed_time);
    }
    else
    {
        // the player has just pressed a direction key, no time has passed
        if (axis.clock == 0.0)
        {
            axis.inertia_clock = 0.0;
            set_lerp_speed(axis);
        }
        // if interpolation is done
        else if (axis.clock >= 1.0)
        {
            if (axis.target == axis.max[1])
            {
                axis.sleep = true;
            }
            limted_orbit_reset_values(axis);
        }

        add_time(elapsed_time, axis.clock, axis.divisor);

        limted_orbit_interpolate(axis);
    }
}

inline void line_lerp(LineAxis &axis, const float elapsed_time)
{
    axis.clock += elapsed_time/axis.divisor;

    if (axis.clock >= 1.0)
    {
        axis.clock = 0.0;
        axis.start = axis.current;
        axis.current = axis.target;
    } else {
        float v;
        v = smoothstep(axis.clock);
        axis.current = (axis.target * v) + (axis.start * (1.0 - v));
    }
}

inline void set_lerp_target(LimitedOrbitAxis &axis, const float new_target, bool &active)
{
    if (axis.target == new_target) { return; }
    
    active = true;
    axis.sleep = false;
    axis.clock = 0.0;
    axis.target = new_target;
    axis.start = axis.current;
}

inline void set_simple_lerp_target(LineAxis &axis, int target)
{
    if (axis.target == axis.max[target]) { return; }
    
    axis.clock = 0.0;
    axis.target = axis.max[target];
    axis.start = axis.current;
}

inline void orbit_reset_values(OrbitAxis &axis)
{ 
    axis.inertia_wclock = 0.0;
    axis.inertia_wstart = 0.0;
    axis.inertia_wtarget = 0.0; 
    axis.is_decelerating = false;
    axis.returning_to_origin = false;
}

inline void set_orbit_target(OrbitAxis &axis, int input_yaw, bool &active)
{
    axis.direction = input_yaw;
    if (axis.direction != 0)
    {
        active = true;
        axis.sleep = false;
        axis.wtarget = axis.max_wtarget;
        orbit_reset_values(axis);
    } else {
        axis.wtarget = 0;
    }
}

inline void weighted_average(float &current, float target, float weight, const float dt)
{
    // Note: for variable frame rate, use this for smooth camera movement.
    float amount = exp(-dt * weight);
    current = (amount) * current + (1-amount) * target;

    // ... and for capped frame rate (60 fps) use
    // current = ((current * (weight - 1)) + target) / weight; 
}

inline void orbit_decelerate(OrbitAxis &axis, const float elapsed_time, float &target)
{
    if (axis.inertia_wclock == 0.0)
    {
        axis.inertia_wtarget = 0.0;
        axis.inertia_wclock = 0.0;//1.0 - axis.wclock;
        axis.inertia_wstart = axis.wcurrent;
    }

    add_time(elapsed_time, axis.inertia_wclock, 1.0);

    float v = axis.inertia_wclock;
    v = 1.0 - (1.0 - v) * (1.0 - v);
    float current_weighted_target = (axis.inertia_wtarget * v) + (axis.inertia_wstart * (1.0 - v));

    target = axis.current + (current_weighted_target * axis.last_direction);

    if (axis.inertia_wclock >= 1.0)
    {
        orbit_reset_values(axis);
    }
}

inline void orbit_accelerate(OrbitAxis &axis, const float elapsed_time, float &target)
{
    add_time(elapsed_time, axis.wclock, 1.0);
    float v = axis.wclock;
    axis.wcurrent = (axis.wtarget * v) + (axis.wstart * (1.0 - v));
    
    target += (axis.wcurrent * axis.direction);
}

static void orbit_lerp(OrbitAxis &axis, const float elapsed_time,
                         float player_yaw, bool moving = false,
                         bool rotating = false)
{
    if (moving || rotating)
    {
        if (axis.direction == 0 && !axis.looking_while_moving)
        {
            axis.returning_to_origin = true;
        } else
        {
            axis.looking_while_moving = true;
        }
    }
    
    if (axis.direction != 0)
    {
        axis.returning_to_origin = false;
    } else if (rotating) {
        axis.looking_while_moving = false;
    } else if (!rotating && !moving) {
        axis.looking_while_moving = false;
    }

    float target = axis.current;

    if (axis.returning_to_origin)
    {
        target = player_yaw;
        //orbit_accelerate(axis, elapsed_time, target);            

        weighted_average(axis.current, target, axis.divisor, elapsed_time);
        if (axis.current > player_yaw - 1.0 && axis.current < player_yaw + 1.0)
        {
            axis.sleep = true;
        }
    }
    else
    {
        if (axis.direction == 0 && axis.is_decelerating) {
            orbit_decelerate(axis, elapsed_time, target);
            axis.wclock = axis.wclock - axis.inertia_wclock;
            axis.wclock = axis.wclock < 0.0 ? 0.0 : axis.wclock;
        } else if (axis.direction != 0) {
            orbit_accelerate(axis, elapsed_time, target);            

            axis.last_direction = axis.direction;
            axis.is_decelerating = true;
            axis.inertia_wclock = 0.0;
        } else {
            orbit_reset_values(axis);
        }

        weighted_average(axis.current, target, axis.divisor, elapsed_time);

        if (axis.current > target - 1.0 && axis.current < target + 1.0)
        {
            axis.is_decelerating = false;
            axis.inertia_wclock = 0.0;
        }
    } 
}

void MultiAxisCamera::render_update(const float elapsed_time, float player_yaw, 
                               vector3df player_position, float player_velocity,
                               float player_turn_velocity )
{
    // 360 and 0 should be treated as adjacent numbers, rather than being at opposite ends.
    if (player_yaw - yaw.current > 360 + yaw.current - player_yaw) {
        yaw.current += 360;
    } else if (yaw.current - player_yaw > 360 + player_yaw  - yaw.current) {
        yaw.current -= 360;
    }

    line_lerp(radius_offset, elapsed_time);
    line_lerp(side_offset, elapsed_time);

    // Determines how each axis should be behaving
    if (active)
    {
        limited_orbit_lerp(pitch, elapsed_time, player_velocity, player_turn_velocity);
        limited_orbit_lerp(lookat_y, elapsed_time, player_velocity, player_turn_velocity);
        orbit_lerp(yaw, elapsed_time, player_yaw, player_velocity, player_turn_velocity);

        if (pitch.sleep && lookat_y.sleep && yaw.sleep) {
            active = false;
        }
    }
    else
    {
        weighted_average(yaw.current, player_yaw, yaw.divisor, elapsed_time);
    }

    // Get position on sphere: polar to cartesian
    float posZ = (radius + radius_offset.current) * cos(pitch.current); // x = r × cos( theta )
    float posY = (radius + radius_offset.current) * sin(pitch.current); // y = r × sin( theta )
    vector3df position_on_sphere(0, posY, posZ); // Create a positional vector with X=0

    // rotate around Y axis the position_on_sphere
    const float degrees_to_radians = 3.14159265359 / 180.0;
    matrix4 yaw_matrix;
    yaw_matrix.setRotationRadians(vector3df(0, yaw.current * degrees_to_radians, 0));
    yaw_matrix.transformVect(position_on_sphere);

    position_on_sphere += vector3df(player_position.X, player_position.Y + camera_height, player_position.Z) ;
    node->setPosition(position_on_sphere);

    vector3df targetLookAt = vector3df (player_position.X,
                                        player_position.Y + look_over_shoulder - lookat_y.current,
                                        player_position.Z);

    vector3df right_vector = vector3df( -(targetLookAt.Z - position_on_sphere.Z),
                                        0,
                                        targetLookAt.X - position_on_sphere.X).normalize();

    node->setTarget(targetLookAt - (right_vector * side_offset.current));
}

void MultiAxisCamera::logic_update(int input_pitch, int input_yaw, float player_yaw, float player_moving)
{
    if (player_moving) {
        set_simple_lerp_target( side_offset, yaw.current > player_yaw ? 0 : yaw.current < player_yaw ? 1 : side_offset.target);
    }

    set_simple_lerp_target( radius_offset, player_moving > 0 ? 1 : 0  );

    set_lerp_target(pitch,pitch.max[input_pitch], active);
    set_lerp_target(lookat_y,lookat_y.max[input_pitch], active);

    set_orbit_target(yaw, input_yaw, active);
}

inline void set_values(LimitedOrbitAxis &axis,
                       float origin_0 = 0.0f, float max_A = -1.0f, float max_B = 1.0f,
                       float minimum_divisor = 0.5f, float maximum_divisor = 1.5f,
                       float return_to_center_divisor = 2.0f, float decelerate_divisor = 0.5f)
{
    axis.max[1] = origin_0;
    axis.max[0] = max_A;
    axis.max[2] = max_B;
    axis.min_divisor = minimum_divisor;
    axis.max_divisor = maximum_divisor;
    axis.return_divisor = return_to_center_divisor;
    axis.divisor = minimum_divisor;
    axis.inertia_divisor = decelerate_divisor;
    axis.current = origin_0;
    axis.target = origin_0;
    axis.start = origin_0;
    axis.inertia_target = origin_0;
    axis.clock = 0.0;
    axis.inertia_clock = 0.0;
    axis.returning_to_origin = true;
    axis.looking_while_moving = false;
    axis.sleep = true;
}

inline void set_values(LineAxis &axis,
                       float max_A = -1.0f, float max_B = 1.0f, float divisor = 4.0f)
{
    axis.max[0] = max_A;
    axis.max[1] = max_B;
    axis.divisor = divisor;
    axis.current = max_A;
    axis.target = max_B;
    axis.start = max_A;
    axis.clock = 0.0;
}

inline void set_values(OrbitAxis &axis, float max_weighted_target = 70.0)
{
    axis.current = 0.0;
    axis.direction = 0;
    axis.last_direction = 0; 
    /** source: http://gamedev.stackexchange.com/questions/80089/
                how-to-use-weighted-average-easing-correctly-with-variable-framerate */
    axis.divisor = log((18.0+1)/18.0)/0.032;
    axis.sleep = true;
    axis.returning_to_origin = false;
    axis.looking_while_moving = false;

    axis.max_wtarget = max_weighted_target;

    axis.inertia_wtarget = 0.0;
    axis.inertia_wclock = 0.0;
    axis.inertia_wstart = 0.0;
    axis.is_decelerating = false;
}

void MultiAxisCamera::create(ISceneManager* smgr)
{
    // Note: Start by editing these values to fit the scale of your game
    // These values work well with my player character that is 2.6 units tall and 0.5 units wide
    node = smgr->addCameraSceneNode( 0, vector3df( 0.0, 0.0, 0.0 ));

    set_values(pitch, 0.3,  -0.2, 0.7,  1.0, 1.6);
    set_values(lookat_y, -0.00,  -2.5, 1.0,  1.0, 1.6);

    set_values(radius_offset, -0.1, 1.5,  5.0);
    set_values(side_offset, -0.35, 0.35,  4.0);

    set_values(yaw, 70.0);

    look_over_shoulder = 2.2;
    camera_height = 1.55;
    radius = 5.0;
    active = false;
}
