#ifndef MULTI_AXIS_CAMERA_H
#define MULTI_AXIS_CAMERA_H

#include "vector3d.h" 

namespace irr {
    namespace scene {
        class ICameraSceneNode;
        class ISceneManager;
    }
} 

struct LineAxis {
    float current; // current position
    float max[2]; // start, end of axis (example: -1.0, 1.0)

    float target;
    float start;
    float clock;
    float divisor; // modifies the speed of interpolation. larger value slows it down, smaller value makes it faster.
};

struct LimitedOrbitAxis {
    float current; // current position
    float max[3]; // start, middle, end of axis (example: -1.0, 0, 1.0)

    float target;
    float start;
    float clock;
    float divisor; // modifies the speed of interpolation. larger value slows it down, smaller value makes it faster.
    float max_divisor;
    float min_divisor;

    float inertia_target;
    float inertia_clock;
    float inertia_divisor; // modifies deceleration interpolation

    float return_divisor; // modifies interpolation when returning to origin

    bool returning_to_origin;
    bool looking_while_moving;
    bool sleep;
};

struct OrbitAxis {
    float current;
    int direction;
    int last_direction;
    float divisor;

    // weighted acceleration values
    float wcurrent;
    float wstart;
    float wtarget;
    float max_wtarget;
    float wclock;
    
    // weighted deceleration values
    float inertia_wtarget;
    float inertia_wclock;
    float inertia_wstart;
    float is_decelerating;

    bool sleep;
    bool looking_while_moving;
    bool returning_to_origin;
};

struct MultiAxisCamera {
    irr::scene::ICameraSceneNode* node;

    LimitedOrbitAxis pitch; // camera vertical position
    LimitedOrbitAxis lookat_y; // camera vertical look at

    OrbitAxis yaw; // camera horizontal orbiting position

    bool active; // true when player is manually moving camera

    LineAxis radius_offset; // offsets the radius, for zoom effect
    LineAxis side_offset; // left or right side of player

    float radius; // distance from player to camera
    float look_over_shoulder; // distance from player to camera
    float camera_height; // distance from player to camera

    void create(irr::scene::ISceneManager* smgr);
    
    void render_update(const float elapsed_time, float player_yaw, 
                       vector3df player_position, float player_velocity,
                       float player_turn_velocity);
    
    void logic_update(int input_pitch, int input_yaw,
                      float player_yaw, float player_moving);
};

#endif // MULTI_AXIS_CAMERA_H
