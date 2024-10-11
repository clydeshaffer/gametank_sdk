#include "geom.h"
#include "bytesine.h"

point3* pnt_223(point2 *p, coord_t y, point3 *res) {
    res->x = p->x;
    res->y = y;
    res->z = p->y;
    return res;
}

point2* rotate2(point2 *p, unsigned char angle, point2 *result) {
    coord_t bsine = sine_table[angle], bcosine = cosine_table[angle];
    result->x = ((bcosine * p->x) - (bsine * p->y)) >> SINE_SCALE;
    result->y = ((bcosine * p->y) + (bsine * p->x)) >> SINE_SCALE;
    return result;
}

point3* rotate3(point3 *p, unsigned char angle, point3 *result) {
    coord_t bsine = sine_table[angle], bcosine = cosine_table[angle];
    result->x = ((bcosine * p->x) - (bsine * p->z)) >> SINE_SCALE;
    result->y = p.y;
    result->z = ((bcosine * p->z) + (bsine * p->x)) >> SINE_SCALE;
    return result;
}

point2 pnt2_shift(point2 p_in, int shiftby, point2* p) {
    *p = *p_in;
    if(shiftby > 0) {
        p->x = p->x << shiftby;
        p->y = p->y << shiftby;
        return p;
    } else {
        p->x = p->x >> (-shiftby);
        p->y = p->y >> (-shiftby);
        return p;
    }
}

/* x will be left-right
   y will be up-down
   z will go into screen */

point3* worldToLocal(point3 *p, point3 *camera_pos, unsigned char angle, point3* out) {
    p->x -= camera_pos->x;
    p->y -= camera_pos->y;
    p->z -= camera_pos->z;
    return rotate3(p, angle, out);
}

point2* localToScreen(point3 *localized, int screenw, int screenh, point2* res) {
    res->x = (localized->x << 7) / localized->z;
    res->y = (localized->y << 7) / localized->z;
    res->x += (screenw>>1);
    res->y = (screenh>>1) - res->y;
    return res;
}

point2* pnt_sub(point2 *a, point2 *b, point2 *out) {
    *out = *a;
    out->x -= b.x;
    out->y -= b.y;
    return out;
}

coord_t pnt_scross(point2 a, point2 b) {
    return a.x * b.y - a.y * b.x;
}

coord_t pnt_dot(point2 a, point2 b) {
    return a.x * b.x + a.y * b.y;
}

point2 pnt_neg(point2 a) {
    a.x *= -1;
    a.y *= -1;
    return a;
}

point2 pnt_flat(point3 a) {
    point2 flat = {0, 0};
    flat.x = a.x;
    flat.y = a.z;
    return flat;
}


bool intersect(point2 a, point2 b, point2 c, point2 d, point2 *output)
{
    coord_t dotProd = pnt_dot(pnt_sub(b, a), pnt_sub(d, c));
    point2 a2b = pnt_sub(b, a);
    point2 c2d = pnt_sub(d, c);
    point2 a2c = pnt_sub(c, a);
    point2 c2a = pnt_sub(a, c);
    coord_t aXb, cXd, s, t;
    if(dotProd == 0) return false;
    aXb = pnt_scross(a2b, c2d);
    cXd = pnt_scross(c2d, a2b);
    s = pnt_scross(a2c,c2d);
    t = pnt_scross(c2a,a2b);
    if(aXb == 0) return false;
    if(s < 0 || s > aXb) return false;
    if(t < 0 || t > cXd) return false;
    a.x += a2b.x * s / aXb;
    a.y += a2b.y * s / aXb;
    *output = a;
    return true;
}

/*bool intersect_originray(point2 a, point2 b, point2 c2d, point2 *output)
{
    point2 a2b = pnt_sub(b, a);
    point2 a2c = pnt_neg(a);
    coord_t dotProd = pnt_dot(a2b, c2d);
    coord_t aXb, s;
    if(dotProd == 0) return false;
    aXb = pnt_scross(a2b, c2d);
    s = pnt_scross(a2c,c2d);
    if(aXb == 0) return false;
    if(s < 0 || s > aXb) return false;
    a.x += a2b.x * s / aXb;
    a.y += a2b.y * s / aXb;
    *output = a;
    return true;
}*/

bool intersect_originray(point2 a, point2 b, point2 c2d, point2 *output)
{
    point2 v1 = pnt_neg(a);
    point2 v2 = pnt_sub(b, a);
    point2 v3;
    coord_t dotProd, t1, t2;
    coord_t s = 1;
    v3.x = -c2d.y;
    v3.y = c2d.x;
    dotProd = pnt_dot(v2, v3);
    if(dotProd == 0) return false;
    if(dotProd < 0) s = -1;
    t1 = pnt_scross(v2, v1) * s;
    t2 = pnt_dot(v1, v3) * s;
    if((t1 < 0) || (t2 < 0) || (t2 > (dotProd*s))) {
        /*printf("\t%ld\n%ld\n", a.x, a.y);
        printf("%ld\n%ld\n", b.x, b.y);
        printf("%ld\n%ld\n", c2d.x, c2d.y);
        printf("%ld\n%ld\n%ld\n", t1, t2, dotProd);*/
        return false;
    }
    output->x = a.x + (v2.x * t2 * s / dotProd);
    output->y = a.y + (v2.y * t2 * s / dotProd);
    return true;
}

/* 45 degree cone for simplicity */
bool point_in_view_cone(point3 a) {
    return ((a.z > a.x) && (a.z > -a.x));
}

bool left_of_cone(point2 a) {
    return (a.y > -a.x);
}

bool view_cone(point3 *a, point3 *b) {
    bool acone = point_in_view_cone(*a), bcone = point_in_view_cone(*b);
    point2 aflat = pnt_flat(*a), bflat = pnt_flat(*b);
    point2 left = {-1000, 1000}, right = {1000, 1000};
    if(acone && bcone) {
        return true;
    }
    if(acone) {
        if(intersect_originray(aflat, bflat, left, &bflat) || intersect_originray(aflat, bflat, right, &bflat)) {
            b->x = bflat.x;
            b->z = bflat.y;
            return true;
        } else {
            return false;
        }
    } else if(bcone) {
        if(intersect_originray(aflat, bflat, left, &aflat) || intersect_originray(aflat, bflat, right, &aflat)) {
            a->x = aflat.x;
            a->z = aflat.y;
            return true;
        } else {
            return false;
        }
    } else {
        /*possible line still passes through view cone*/
        if(left_of_cone(aflat) ^ left_of_cone(bflat)) {
            point2 temp_aflat = aflat, temp_bflat = bflat;
            if(intersect_originray(temp_aflat, temp_bflat, left, &aflat) &&
            intersect_originray(temp_aflat, temp_bflat, right, &bflat)) {
                if(a->x < b->x) {
                    a->x = aflat.x;
                    a->z = aflat.y;
                    b->x = bflat.x;
                    b->z = bflat.y;
                    return true;
                } else {
                    a->x = bflat.x;
                    a->z = bflat.y;
                    b->x = aflat.x;
                    b->z = aflat.y;
                    return true;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
}



coord_t line_side(point2 p, point2 a, point2 b) {
    return pnt_scross(pnt_sub(b, a), pnt_sub(p,a));
}