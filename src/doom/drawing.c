#include "drawing.h"

#include <stdlib.h>

#define GRAD_NONE 0
#define GRAD_H 1
#define GRAD_V 2

#define WALL 1
#define CEILING 2
#define FLOOR 4

#define minOf(a,b) (a<b)?a:b
#define maxOf(a,b) (a>b)?a:b
#define sgn(x) (x>0)?1:-(x<0)
#define times320(y) ((y<<8)+(y<<6))
#define clamp(x,a,b) (a>x)?a:((x<b)?x:b)


byte trape_heights[SCREEN_WIDTH];
byte *gradient_buffer;

byte bayer[16] = {0, 8, 2, 10,
                12, 4, 14, 6,
                3, 11, 1, 9,
                15, 7, 13, 5};

const coord_t darkness_depth = 800;

void debug_delay() {
    wait_retrace();
    show_buffer();
}

/* return true if sub overlaps super, and set sub bounds to intersection
otherwise return false */
bool portal_overlap(viewport *sub, viewport *super) {
    viewport original = *sub;
    if(sub->top < super->top) sub->top = super->top;
    if(sub->bottom > super->bottom) sub->bottom = super->bottom;
    if(sub->left < super->left) sub->left = super->left;
    if(sub->right > super->right) sub->right = super->right;

    if(sub->right < sub->left) {
        *sub = original;
        return false;
    }
    if(sub->bottom < sub->top) {
        *sub = original;
        return false;
    }
    return true;
}

Outcode get_outcode(point2 v, viewport portal) {
    Outcode code = 0;
    if(v.y < portal.top) {
        code |= TOP;
    } else if(v.y > (portal.bottom)) { /* was SCREEN_HEIGHT-1*/
        code |= BOTTOM;
    }
    if(v.x < portal.left) {
        code |= LEFT;
    } else if(v.x > (portal.right)) { /*was SCREEN_WIDTH-1*/
        code |= RIGHT;
    }
    return code;
}

bool test_portal_viable(viewport portal) {
    if(portal.left >= portal.right) return false;
    if(portal.top >= portal.bottom) return false;
    return true;
}

void highlight_portal(viewport portal, byte color) {
    int i;
    memset(graphic_buffer + portal.left + times320(portal.top), color, portal.right - portal.left);
    memset(graphic_buffer + portal.left + times320(portal.bottom), color, portal.right - portal.left);
    for(i = portal.top; i <= portal.bottom; i++) {
        pixel(portal.left, i) = color;
        pixel(portal.right, i) = color;
    }
}

void printf_view(viewport portal) {
    printf("Portal is\n\t%d %d\n\t%d %d\n", portal);
}

void printf_point(point2 p) {
    printf("%ld, %ld", p);
}

/* cohen-sutherland algorithm */
bool clip_line_to_viewport(point2 *pointA, point2 *pointB, viewport portal) {
    int iter_limit = 10;
    Outcode outcodeA = get_outcode(*pointA, portal),
    outcodeB = get_outcode(*pointB, portal),
    outside_code;
    point2 intersect_point,
    originalA = *pointA,
    originalB = *pointB;

    if(!test_portal_viable(portal)) return false;

    while(iter_limit) {
        iter_limit--;
        if(!(outcodeA | outcodeB)) {
            /*line has been corrected*/
            return true;
        } else if(outcodeA & outcodeB) {
            /*return unmodified failing line*/
            *pointA = originalA;
            *pointB = originalB;
            return false;
        } else {
            outside_code = outcodeA ? outcodeA : outcodeB;

            if(outside_code & TOP) {
                if(pointB->y == pointA->y)  {
                    *pointA = originalA;
                    *pointB = originalB;
                    return false;
                }
                intersect_point.x = pointA->x + (pointB->x - pointA->x) * (portal.top - pointA->y) / (pointB->y - pointA->y);
                intersect_point.y = portal.top;
            } else if(outside_code & BOTTOM) {
                if(pointB->y == pointA->y)  {
                    *pointA = originalA;
                    *pointB = originalB;
                    return false;
                }
                intersect_point.x = pointA->x + (pointB->x - pointA->x) * (portal.bottom - pointA->y) / (pointB->y - pointA->y);
                intersect_point.y = portal.bottom;
            } else if(outside_code & LEFT) {
                if(pointB->x == pointA->x)  {
                    *pointA = originalA;
                    *pointB = originalB;
                    return false;
                }
                intersect_point.x = portal.left;
                intersect_point.y = pointA->y + (pointB->y - pointA->y) * (portal.left - pointA->x) / (pointB->x - pointA->x);
            } else if(outside_code & RIGHT) {
                if(pointB->x == pointA->x)  {
                    *pointA = originalA;
                    *pointB = originalB;
                    return false;
                }
                intersect_point.x = portal.right;
                intersect_point.y = pointA->y + (pointB->y - pointA->y) * (portal.right - pointA->x) / (pointB->x - pointA->x);
            }
        }

        if(outcodeA == outside_code) {
            *pointA = intersect_point;
            outcodeA = get_outcode(*pointA, portal);
        } else {
            *pointB = intersect_point;
            outcodeB = get_outcode(*pointB, portal);
        }
    }
    printf("coen-sutherland alg limit reached\n");
    printf_view(portal);
    printf_point(originalA);
    printf("\n");
    printf_point(originalB);
    printf("\n");
    *pointA = originalA;
    *pointB = originalB;
    return false;
}

byte sample_gradient(int x, int y, int t) {
    /*byte c = (byte) (gradient_buffer[t] >> 4);
    byte frac = (byte) (gradient_buffer[t] & 15);
    if(bayer[((y % 4) * 4) + (x % 4)] <= frac) c++;
    return c;*/
    return (gradient_buffer[t] >> 4) + (bayer[((y % 4) * 4) + (x % 4)] <= ((byte) (gradient_buffer[t] & 15)));
}

/*int failsBounds(int x, int y) {
    return (x < 0) || (x >= SCREEN_WIDTH) || (y < 0) || (y >= SCREEN_HEIGHT);
}*/

byte depth_to_shade(coord_t depth) {
    depth = 16 * min(depth, darkness_depth) / darkness_depth;
    if(depth > 15) depth = 15;
    else if(depth < 1) depth = 1;
    return (byte)((int) depth);
}

void vert(int xpos, int ystart, int ylen, byte color, int gradient_mode) {
    int i;
    int k, g;
    byte drawn_color[3];
    drawn_color[GRAD_NONE] = color;
    if(ylen <= 0) return;
    if(ystart + ylen < 0) return;
    ystart = maxOf(ystart, 0);
    g = ystart;
    k = (ystart << 8) + (ystart << 6);
    if(ystart + ylen > SCREEN_HEIGHT) {
        ylen = SCREEN_HEIGHT - ystart;
    }
    #ifdef DEBUGDELAY
    debug_delay();
    #endif
    for(i = 0; i < ylen; i++) {
        drawn_color[GRAD_V] = gradient_buffer[g + (xpos % 4)*SCREEN_WIDTH];
        /*drawn_color[GRAD_V] = gradient_buffer[g];*/
        drawn_color[GRAD_H] = gradient_buffer[xpos + (g % 4)*SCREEN_WIDTH];
        g++;
        graphic_buffer[k + xpos] = drawn_color[gradient_mode];
        k += SCREEN_WIDTH;
        
    }
    return;
}

void rect(int xpos, int ypos, int width, int height, byte color) {
    int i, addr = times320(ypos) + xpos;
    for(i = 0; i < height; i++) {
        memset(&(graphic_buffer[addr]), color, width);
        addr += SCREEN_WIDTH;
        #ifdef DEBUGDELAY
        debug_delay();
        #endif
    }
}

void grad_rect_h(int xpos, int ypos, int width, int height) {
    int i, k, addr = times320(ypos) + xpos;
    for(i = 0; i < height; i++) {
        /*memset(&(graphic_buffer[addr]), color, width);*/
        memcpy(&(graphic_buffer[addr]), gradient_buffer+xpos+(times320((ypos+i)%4)), width);
        /*for(k = 0; k < width; k++) {
            graphic_buffer[addr + k] = sample_gradient(xpos+k, ypos+i, xpos+k);
        }*/

        addr += SCREEN_WIDTH;
        #ifdef DEBUGDELAY
        debug_delay();
        #endif
    }
}


void grad_rect_v(int xpos, int ypos, int width, int height) {
    int i,k=0, addr = times320(ypos) + xpos, lim = width;
    while(lim > 1) {
        lim = lim >> 1;
        k++;
    }
    lim = 1 << k;
    for(i = 0; i < height; i++) {
        k=0;
        for(k = 0; (k < width) && (k < 4); k++) {
            graphic_buffer[addr + k] = gradient_buffer[ypos+i+times320((xpos+k)%4)];
            #ifdef DEBUGDELAY
            debug_delay();
            #endif
        }
        for(k = 4; k < lim; k*=2) {
            memcpy(&(graphic_buffer[addr+k]), &(graphic_buffer[addr]), k);
            #ifdef DEBUGDELAY
            debug_delay();
            #endif
        }
        memcpy(&(graphic_buffer[addr+k]), &(graphic_buffer[addr]), width - lim);
        addr += SCREEN_WIDTH;
    }
}

/*
void grad_rect_v(int xpos, int ypos, int width, int height) {
    int i,k=0, addr = times320(ypos) + xpos, lim = width;
    lim = 1 << k;
    for(i = 0; i < height; i++) {
        long pat = 0;
        for(k = 0; k < 4; k++) {
            graphic_buffer[addr + k] = gradient_buffer[ypos+i+times320((xpos+k)%4)];
        }
        pat = *((long*) (graphic_buffer+addr));
        
        for(k = 0; k < width; k += 4) {
            *((long*) (graphic_buffer+addr+k)) = pat;
        }
        addr += SCREEN_WIDTH;
    }
}*/

void draw_line2(int ax, int ay, int bx, int by, byte color) {
    int dx = bx - ax,
        dy = by - ay,
        absx = abs(dx),
        absy = abs(dy),
        sx = sgn(dx),
        sy = sgn(dy),
        stx = absx >> 1,
        sty = absy >> 1,
        i, px = ax, py = ay;

    int *dmajor, *smajor, *sminor, *stminor, *absmajor, *absminor, *pmajor, *pminor;

    if(abs(dx) > abs(dy)) {
        dmajor = &dx;
        smajor = &sx;
        sminor = &sy;
        stminor = &sty;
        absmajor = &absx;
        absminor = &absy;   
        pmajor = &px;
        pminor = &py;   
    } else {
        dmajor = &dy;
        smajor = &sy;
        sminor = &sx;
        stminor = &stx;
        absmajor = &absy;
        absminor = &absx;   
        pmajor = &py;
        pminor = &px;
    }

    for(i = 0; i <= abs(*dmajor); i ++) {
        *stminor += *absminor;
        if(*stminor >= *absmajor) {
            *stminor -= *absmajor;
            *pminor += *sminor;
        }
        *pmajor += *smajor;
        if(px < 0 || py < 0 || px >= SCREEN_WIDTH || py >= SCREEN_HEIGHT) return;
        pixel(px, py) = color;
    }
}


void draw_line_screen(point2 a, point2 b, byte color) {
    viewport fullscreen = {0, SCREEN_HEIGHT-1, 0, SCREEN_WIDTH-1};
    if(clip_line_to_viewport(&a, &b, fullscreen)) {
        if(a.x <= b.x) {
            draw_line2(a.x, a.y, b.x, b.y, color);
        } else {
            draw_line2(b.x, b.y, a.x, a.y, color);
        }
    }
}

void line_3d(point3 a, point3 b, byte color, point3 camerap, byte cnt) {
    point2 drawp, drawp2;
    point3 localized = worldToLocal(a, camerap, cnt);
    point3 localized2 = worldToLocal(b, camerap, cnt);
    /*if(localized.x > localized2.x) {
        point3 swp = localized2;
        localized2 = localized;
        localized = swp;
    }*/
    if(localized.z > 0 || localized2.z > 0) {
        if(view_cone(&localized, &localized2)) {
            if(localized.z > 0 && localized2.z > 0) {
                drawp = localToScreen(localized, SCREEN_WIDTH, SCREEN_HEIGHT);
                drawp2 = localToScreen(localized2, SCREEN_WIDTH, SCREEN_HEIGHT);
                draw_line_screen(drawp, drawp2, 10);
            }
        }
    }
}

void interpolate_buffer(int ax, int ay, int bx, int by, byte *target_array) {
    int dx = bx - ax,
        dy = by - ay,
        absx = abs(dx),
        absy = abs(dy),
        sx = sgn(dx),
        sy = sgn(dy),
        stx = absx >> 1,
        sty = absy >> 1,
        i, px = ax, py = ay;

    int *dmajor, *smajor, *sminor, *stminor, *absmajor, *absminor, *pmajor, *pminor;

    if(ay == by) {
        memset(target_array+min(ax, bx), by, absx);
    }

    if(abs(dx) > abs(dy)) {
        dmajor = &dx;
        smajor = &sx;
        sminor = &sy;
        stminor = &sty;
        absmajor = &absx;
        absminor = &absy;   
        pmajor = &px;
        pminor = &py;   
    } else {
        dmajor = &dy;
        smajor = &sy;
        sminor = &sx;
        stminor = &stx;
        absmajor = &absy;
        absminor = &absx;   
        pmajor = &py;
        pminor = &px;
    }

    target_array[ax] = ay;

    if(ax < bx) {
        for(i = 0; i <= ax; i++) {
            target_array[i] = ay;
        }

        for(i = bx; i < SCREEN_WIDTH; i++) {
            target_array[i] = by;
        }
    } else {
        for(i = 0; i <= bx; i++) {
            target_array[i] = by;
        }

        for(i = ax; i < SCREEN_WIDTH; i++) {
            target_array[i] = ay;
        }
    }

    for(i = 0; i <= abs(*dmajor); i ++) {
        *stminor += *absminor;
        if(*stminor >= *absmajor) {
            *stminor -= *absmajor;
            *pminor += *sminor;
        }
        *pmajor += *smajor;
        if(px > 0 && px < SCREEN_WIDTH) {
            target_array[px] = clamp(py, 0, SCREEN_WIDTH);
        }
    }
}

byte bayerize(int x, int y, int val) {
    return (val >> 4) + (bayer[((y % 4) * 4) + (x % 4)] <= ((byte) (val & 15)));
}

void make_gradient(int ax, int ay, int bx, int by, byte *target_array) {
    int dx = bx - ax,
        dy = (by << 4) - (ay << 4),
        absx = abs(dx),
        absy = abs(dy),
        sx = sgn(dx),
        sy = sgn(dy),
        stx = absx >> 1,
        sty = absy >> 1,
        i, px = ax, py = ay << 4;

    int *dmajor, *smajor, *sminor, *stminor, *absmajor, *absminor, *pmajor, *pminor;

    ay = ay<<4;
    by = by<<4;

    if(ay == by) {
        /*memset(target_array+min(ax, bx), by, absx);*/
        for(i = ax; i <= bx; i++) {
            /*target_array[i] = ay;*/
            target_array[i] = bayerize(i, 0, ay);
            target_array[i+SCREEN_WIDTH] = bayerize(i, 1, ay);
            target_array[i+SCREEN_WIDTH*2] = bayerize(i, 2, ay);
            target_array[i+SCREEN_WIDTH*3] = bayerize(i, 3, ay);
        }
        return;
    }

    if(abs(dx) > abs(dy)) {
        dmajor = &dx;
        smajor = &sx;
        sminor = &sy;
        stminor = &sty;
        absmajor = &absx;
        absminor = &absy;   
        pmajor = &px;
        pminor = &py;   
    } else {
        dmajor = &dy;
        smajor = &sy;
        sminor = &sx;
        stminor = &stx;
        absmajor = &absy;
        absminor = &absx;   
        pmajor = &py;
        pminor = &px;
    }

    target_array[ax] = ay;

    if(ax < bx) {
        for(i = 0; i <= bx; i++) {
            /*target_array[i] = ay;*/
            target_array[i] = bayerize(i, 0, ay);
            target_array[i+SCREEN_WIDTH] = bayerize(i, 1, ay);
            target_array[i+SCREEN_WIDTH*2] = bayerize(i, 2, ay);
            target_array[i+SCREEN_WIDTH*3] = bayerize(i, 3, ay);
        }

        for(i = ax; i < SCREEN_WIDTH; i++) {
            /*target_array[i] = by;*/
            target_array[i] = bayerize(i, 0, by);
            target_array[i+SCREEN_WIDTH] = bayerize(i, 1, by);
            target_array[i+SCREEN_WIDTH*2] = bayerize(i, 2, by);
            target_array[i+SCREEN_WIDTH*3] = bayerize(i, 3, by);
        }
    } else {
        for(i = 0; i <= ax; i++) {
            /*target_array[i] = by;*/
            target_array[i] = bayerize(i, 0, by);
            target_array[i+SCREEN_WIDTH] = bayerize(i, 1, by);
            target_array[i+SCREEN_WIDTH*2] = bayerize(i, 2, by);
            target_array[i+SCREEN_WIDTH*3] = bayerize(i, 3, by);
        }

        for(i = bx; i < SCREEN_WIDTH; i++) {
            /*target_array[i] = ay;*/
            target_array[i] = bayerize(i, 0, ay);
            target_array[i+SCREEN_WIDTH] = bayerize(i, 1, ay);
            target_array[i+SCREEN_WIDTH*2] = bayerize(i, 2, ay);
            target_array[i+SCREEN_WIDTH*3] = bayerize(i, 3, ay);
        }
    }

    for(i = 0; i <= abs(*dmajor); i ++) {
        *stminor += *absminor;
        if(*stminor >= *absmajor) {
            *stminor -= *absmajor;
            *pminor += *sminor;
        }
        *pmajor += *smajor;
        if(px > 0 && px < SCREEN_WIDTH) {
            /*target_array[i] = ay;*/
            target_array[px] = bayerize(px, 0, py);
            target_array[px+SCREEN_WIDTH] = bayerize(px, 1, py);
            target_array[px+SCREEN_WIDTH*2] = bayerize(px, 2, py);
            target_array[px+SCREEN_WIDTH*3] = bayerize(px, 3, py);
        }
    }
}

void draw_wall_screen(int ax, int ay, int bx, int by, int gradient_type, byte color) {
    int dx = bx - ax,
        dy = by - ay,
        absx = abs(dx),
        absy = abs(dy),
        sx = sgn(dx),
        sy = sgn(dy),
        stx = absx >> 1,
        sty = absy >> 1,
        i, height, px = ax, py = ay,
        lower_top = max(ay,by),
        higher_bottom = min(trape_heights[ax+1], trape_heights[bx-1]),
        higher_bottom2 = higher_bottom,
        lower_top2 = lower_top;
    byte grad_color_h, *drawn_color;

    /*if(failsBounds(ax,ay)) return;
    if(failsBounds(bx,by)) return;*/
    
    if(gradient_type == GRAD_H) {
        drawn_color = &grad_color_h;
    } else {
        drawn_color = &color;
    }

    if(lower_top < higher_bottom) {
        vert(ax, ay, lower_top - ay, color, gradient_type);
        vert(ax, higher_bottom, trape_heights[ax] - higher_bottom, color, gradient_type);

        vert(bx, by, lower_top - by, color, gradient_type);
        vert(bx, higher_bottom, trape_heights[bx] - higher_bottom, color, gradient_type);
        if(absx > absy) {
            for(i = 0; i <= absx; i ++) {
                sty += absy;
                if(sty >= absx) {
                    sty -= absx;
                    py += sy;
                }
                px += sx;
                grad_color_h = gradient_buffer[px];
                /*graphic_buffer[(py << 8) + (py << 6) + px] = color;*/
                /*vert(px, py, trape_heights[px] - py, color);*/
                vert(px, py, lower_top - py, *drawn_color, gradient_type);
                vert(px, higher_bottom, trape_heights[px] - higher_bottom, *drawn_color, gradient_type);

            }
        } else {
            for(i = 0; i <= absy; i ++) {

                stx += absx;
                if(stx >= absy) {
                    stx -= absy;
                    px += sx;
                }
                py += sy;
                grad_color_h = gradient_buffer[px];
                /*graphic_buffer[(py << 8) + (py << 6) + px] = color;*/
                vert(px, py, lower_top - py, *drawn_color, gradient_type);
                vert(px, higher_bottom, trape_heights[px] - higher_bottom, *drawn_color, gradient_type);
            }
        }
        if(gradient_type == GRAD_H) {
            grad_rect_h(ax, lower_top, absx+2, higher_bottom - lower_top);
        } else if(gradient_type == GRAD_V) {
            grad_rect_v(ax, lower_top, absx+2, higher_bottom - lower_top);
        } else {
            rect(ax, lower_top, absx+2, higher_bottom - lower_top, color);
        }
    } else {
        vert(px, py, trape_heights[px] - py, color, gradient_type);
        vert(bx, by, trape_heights[bx] - by, color, gradient_type);
        if(absx > absy) {
            for(i = 0; i <= absx; i ++) {
                sty += absy;
                if(sty >= absx) {
                    sty -= absx;
                    py += sy;
                }
                px += sx;
                vert(px, py, trape_heights[px] - py, color, gradient_type);

            }
        } else {
            for(i = 0; i <= absy; i ++) {
                stx += absx;
                if(stx >= absy) {
                    stx -= absy;
                    px += sx;
                }
                py += sy;
                vert(px, py, trape_heights[px] - py, color, gradient_type);
            }
        }
    }
}

void x_sort(point2 *a, point2 *b) {
    if(a->x > b->x) {
        point2 tmp = *b;
        *b = *a;
        *a = *b;
    }
}

/*assume we've already checked the view cone and should definitely draw this wall*/
void draw_wall(point2 pointA, point2 pointB, coord_t floor, coord_t ceiling, viewport portal, int flags, int base_color, int wallmask) {
    point3 a_top = pnt_223(pointA, ceiling);
    point3 b_top = pnt_223(pointB, ceiling);
    point3 a_bot = pnt_223(pointA, floor);
    point3 b_bot = pnt_223(pointB, floor);

    int left_shade, right_shade,i;
    coord_t left_side, right_side;

    point2 a_top_screen = localToScreen(a_top, SCREEN_WIDTH, SCREEN_HEIGHT);   
    point2 b_top_screen = localToScreen(b_top, SCREEN_WIDTH, SCREEN_HEIGHT);
    point2 a_bot_screen = localToScreen(a_bot, SCREEN_WIDTH, SCREEN_HEIGHT);   
    point2 b_bot_screen = localToScreen(b_bot, SCREEN_WIDTH, SCREEN_HEIGHT);

    if(a_top_screen.y > portal.bottom && b_top_screen.y > portal.bottom) return;
    if(a_bot_screen.y < portal.top && b_bot_screen.y < portal.top) return;


    if(a_top_screen.x < b_top_screen.x) {
        left_shade = depth_to_shade(pointA.y << 1) + base_color;
        right_shade = depth_to_shade(pointB.y << 1) + base_color;
    } else {
        left_shade = depth_to_shade(pointB.y) + base_color;
        right_shade = depth_to_shade(pointA.y) + base_color;
    }

    x_sort(&a_top_screen, &b_top_screen);
    x_sort(&a_bot_screen, &b_bot_screen);

    if(a_top_screen.x > portal.right) return;
    if(b_top_screen.x < portal.left) return;

    /* definitely on screen now... hopefully */
    left_side = a_top_screen.x;
    right_side = b_top_screen.x;
    if(wallmask & FLOOR) {
        if(!clip_line_to_viewport(&a_bot_screen, &b_bot_screen, portal)) {
            a_bot_screen.y = portal.bottom;
            b_bot_screen.y = portal.bottom;
        } else {
            /*bottom is on screen*/
            make_gradient(199, 31 + (int) (floor >> 3), 84, 15, gradient_buffer);
            for(i = 0; i < SCREEN_WIDTH; i++) {
                trape_heights[i] = (byte) portal.bottom;
            }
            draw_wall_screen(a_bot_screen.x, a_bot_screen.y, b_bot_screen.x, b_bot_screen.y, GRAD_V, 31 + (int) (floor >> 5));
        }
    }

    if(wallmask & CEILING) {
        if(!clip_line_to_viewport(&a_top_screen, &b_top_screen, portal)) {
            a_top_screen.y = portal.top;
            b_top_screen.y = portal.top;
        } else {
            /*top is on screen*/
            make_gradient(0, 31 - (ceiling >> 3), 116, 15, gradient_buffer);
            interpolate_buffer(a_top_screen.x, a_top_screen.y, b_top_screen.x, b_top_screen.y, trape_heights);
            draw_wall_screen(a_top_screen.x, portal.top, b_top_screen.x, portal.top, GRAD_V, 31 - (int) (ceiling >> 5));
        }
    }

    if(wallmask & WALL) {
        make_gradient(left_side,
            left_shade
            , right_side,
            right_shade
            , gradient_buffer);

        for(i = portal.left; i <= portal.right; i++) {
            trape_heights[i] = portal.bottom; 
        }
        interpolate_buffer(a_bot_screen.x, a_bot_screen.y, b_bot_screen.x, b_bot_screen.y, trape_heights);
        draw_wall_screen(a_top_screen.x, a_top_screen.y, b_top_screen.x, b_top_screen.y, GRAD_H, 0);
        if(left_side < a_top_screen.x && a_top_screen.y == portal.top) {
            draw_wall_screen(left_side, portal.top, a_top_screen.x, portal.top, GRAD_H, 0);
        }
        if(right_side > b_top_screen.x && b_top_screen.y == portal.top) {
            draw_wall_screen(b_top_screen.x, portal.top, right_side, portal.top, GRAD_H, 0);
        }
    }
}

void check_and_draw_wall(point2 a, point2 b, coord_t floor, coord_t ceiling, gap *neighbor_heights, byte color, point3 camerap, byte cnt, viewport portal) {
    point3 localized = worldToLocal(pnt_223(a, 0), camerap, cnt);
    point3 localized2 = worldToLocal(pnt_223(b, 0), camerap, cnt);
    /*if(localized.x > localized2.x) {
        point3 swp = localized2;
        localized2 = localized;
        localized = swp;
    }*/
    if(localized.z > 0 || localized2.z > 0) {
        if(view_cone(&localized, &localized2)) {
            if(localized.z > 0 && localized2.z > 0) {
                if(neighbor_heights == NULL) {
                    draw_wall(pnt_flat(localized), pnt_flat(localized2), floor - camerap.y, ceiling - camerap.y,  portal, 0, 144, WALL | CEILING | FLOOR);
                } else {
                    if(neighbor_heights->top < ceiling)
                        draw_wall(pnt_flat(localized), pnt_flat(localized2), neighbor_heights->top - camerap.y, ceiling - camerap.y,  portal, 0, 144, CEILING | WALL);
                    else
                        draw_wall(pnt_flat(localized), pnt_flat(localized2), floor - camerap.y, ceiling - camerap.y,  portal, 0, 144, CEILING);

                    if(neighbor_heights->bottom > floor)
                        draw_wall(pnt_flat(localized), pnt_flat(localized2), floor - camerap.y, neighbor_heights->bottom - camerap.y,  portal, 0, 144, FLOOR | WALL);
                    else
                        draw_wall(pnt_flat(localized), pnt_flat(localized2), floor - camerap.y, ceiling - camerap.y,  portal, 0, 144, FLOOR);
                }
                
            }
        }
    }
}