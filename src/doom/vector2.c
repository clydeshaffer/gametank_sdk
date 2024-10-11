#include "Vector2.h"
#include <math.h>

Vector2 vec_add(Vector2 a, Vector2 b) {
    Vector2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

Vector2 vec_sub(Vector2 a, Vector2 b) {
    Vector2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

Vector2 vec_scale_v(Vector2 a, Vector2 b) {
    Vector2 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}

Vector2 vec_scale_s(Vector2 a, float b) {
    Vector2 result;
    result.x = a.x * b;
    result.y = a.y * b;
    return result;
}

Vector2 vec_rotate_slow(Vector2 v, float radians) {
    float mycos = cos(-radians);
    float mysin = sin(-radians);
    Vector2 result;
    result.x = (mycos * v.x) + (mysin * v.y);
    result.y = (mycos * v.y) - (mysin * v.x);
    return result;
}

Vector2 vec_rotate_faster(Vector2 v, float mysin, float mycos) {
    Vector2 result;
    result.x = (mycos * v.x) + (mysin * v.y);
    result.y = (mycos * v.y) - (mysin * v.x);
    return result;
}

float vec_square_mag(Vector2 v) {
    return v.x * v.x + v.y * v.y;
}

float vec_scross(Vector2 a, Vector2 b) {
    return a.x * b.y - a.y * b.x;
}

float vec_dot(Vector2 a, Vector2 b) {
    return a.x * b.x + a.y * b.y;
}