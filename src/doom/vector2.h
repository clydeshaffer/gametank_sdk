#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct Vector2
{
    float x;
    float y;
} Vector2;

Vector2 vec_add(Vector2 a, Vector2 b);

Vector2 vec_sub(Vector2 a, Vector2 b);

Vector2 vec_scale_v(Vector2 a, Vector2 b);

Vector2 vec_scale_s(Vector2 a, float b);

Vector2 vec_rotate_slow(Vector2 v, float radians);

Vector2 vec_rotate_faster(Vector2 v, float sin, float cos);

float vec_square_mag(Vector2 v);

float vec_scross(Vector2 a, Vector2 b);

float vec_dot(Vector2 a, Vector2 b);

#endif