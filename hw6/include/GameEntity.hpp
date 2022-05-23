#pragma once
#include <cstdint>
#include <cmath>

struct Vec2
        {
            float x;
            float y;
        };

static float SqLen(Vec2& a)
{
    return a.x*a.x + a.y*a.y;
}

static float Len(Vec2& a)
{
    return sqrtf(SqLen(a));
}

struct GameEntity
        {
            Vec2 pos;
            Vec2 target;
            Vec2 vel;
            uint32_t size;
            bool is_player;
            bool dead;
        };
