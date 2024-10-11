#include "gt/input.h"
#include "bytesine.h"
#include "geom.h"
#include "sector.h"
#include "drawing.h"

char keystates[32];
char old_keystates[32];

point3 camerap = {-20, 0, 84};
unsigned char cnt = 0x22;

int sgn(int x) {
	if(x > 0) return 1;
	if(x < 0) return -1;
	if(x == 0) return 0;
}

point2 screenshift(point2 a) {
	a.x += (SCREEN_WIDTH>>1);
    a.y = (SCREEN_HEIGHT>>1) - a.y;
    return a;
}

int sectorstack = 0;
void recursive_sector_draw(int current_sector, viewport portal, int limit) {
    int i;
    pixel(0, sectorstack) = (byte) current_sector+1;
    if(limit & (1 << current_sector)) return;
    if(portal.top == portal.bottom) return;
    if(portal.left == portal.right) return;
    for(i = 0; i < world_sector_list[current_sector].corner_count; i++) {
        point2 pntA = world_vertex_list[world_sector_list[current_sector].corners[i]],
        pntB = world_vertex_list[world_sector_list[current_sector].corners[(i+1)%(world_sector_list[current_sector].corner_count)]];
        /*line_3d(pnt_223(pntA, world_sector_list[current_sector].floor), pnt_223(pntB, world_sector_list[current_sector].floor), 10, camerap, cnt);
        line_3d(pnt_223(pntA, world_sector_list[current_sector].ceiling), pnt_223(pntB, world_sector_list[current_sector].ceiling), 10, camerap, cnt);
        line_3d(pnt_223(pntB, world_sector_list[current_sector].ceiling), pnt_223(pntB, world_sector_list[current_sector].floor), 10, camerap, cnt);

        draw_line_screen(screenshift(pnt2_shift(pnt_flat(worldToLocal(pnt_223(pntA, 0), camerap, cnt)), -2)),
            screenshift(pnt2_shift(pnt_flat(worldToLocal(pnt_223(pntB, 0), camerap, cnt)), -2)), 13);*/

        if(world_sector_list[current_sector].neighbors[i] != -1) {
            int neighbor = world_sector_list[current_sector].neighbors[i];
            viewport neighbor_portal;
            coord_t ceiling = world_sector_list[current_sector].ceiling;
            coord_t floor = world_sector_list[current_sector].floor;
            point3 a_top = worldToLocal(pnt_223(pntA, ceiling), camerap, cnt);
            point3 b_top = worldToLocal(pnt_223(pntB, ceiling), camerap, cnt);
            gap neighbor_heights;
            neighbor_heights.top = world_sector_list[neighbor].ceiling;
            neighbor_heights.bottom = world_sector_list[neighbor].floor;
            if(neighbor_heights.top < ceiling) {
                a_top.y = neighbor_heights.top - camerap.y;
                b_top.y = neighbor_heights.top - camerap.y;
            }
            if(a_top.z > 0 || b_top.z > 0) {
                if(view_cone(&a_top, &b_top)) {
                    if(a_top.z > 0 && b_top.z > 0) {
                        point2 a_top_screen = localToScreen(a_top, SCREEN_WIDTH, SCREEN_HEIGHT);   
                        point2 b_top_screen = localToScreen(b_top, SCREEN_WIDTH, SCREEN_HEIGHT);
                        if(a_top_screen.x < b_top_screen.x) {
                            
                            
                            point3 a_bot = a_top;
                            point3 b_bot = b_top;
                            point2 a_bot_screen; 
                            point2 b_bot_screen;
                            
                            a_bot.y = floor - camerap.y;
                            b_bot.y = floor - camerap.y;
                            if(neighbor_heights.bottom > floor) {
                                a_bot.y = neighbor_heights.bottom - camerap.y;
                                b_bot.y = neighbor_heights.bottom - camerap.y;
                            }
                            a_bot_screen = localToScreen(a_bot, SCREEN_WIDTH, SCREEN_HEIGHT);   
                            b_bot_screen = localToScreen(b_bot, SCREEN_WIDTH, SCREEN_HEIGHT);
                            neighbor_portal.left = a_top_screen.x;
                            neighbor_portal.right = b_top_screen.x;
                            if(clip_line_to_viewport(&a_top_screen, &b_top_screen, portal)) {
                                neighbor_portal.top = a_top_screen.y;
                                if(a_top_screen.y > b_top_screen.y) neighbor_portal.top = b_top_screen.y;
                            } else {
                                neighbor_portal.top = portal.top;
                            }
                            
                            if(clip_line_to_viewport(&a_bot_screen, &b_bot_screen, portal)) {
                                neighbor_portal.bottom = a_bot_screen.y;
                                if(a_bot_screen.y < b_bot_screen.y) neighbor_portal.bottom = b_bot_screen.y;
                            } else {
                                neighbor_portal.bottom = portal.bottom;
                            }

                            if(portal_overlap(&neighbor_portal, &portal)) {
                                highlight_portal(neighbor_portal, 15);
                                sectorstack++;
                                recursive_sector_draw(neighbor, neighbor_portal,limit | (1 << current_sector));
                                pixel(0, sectorstack) = 0;
                                sectorstack--;
                                check_and_draw_wall(pntA, pntB, world_sector_list[current_sector].floor, world_sector_list[current_sector].ceiling,
                                    &neighbor_heights, 16, camerap, cnt, portal);
                                
                            }
                        }
                    }
                }
            }
            /*recursive_sector_draw(world_sector_list[current_sector].neighbors[i], portal,limit-1);*/
        } else {
            check_and_draw_wall(pntA, pntB, world_sector_list[current_sector].floor, world_sector_list[current_sector].ceiling, NULL, 16, camerap, cnt, portal);
        }
    }
}

void overdoom_main() {

	
	unsigned int ang = 0x44;
	int i, current_sector = 0;

    viewport fullscreen = {0, SCREEN_HEIGHT-1, 0, SCREEN_WIDTH-1};

	if(!load_sectors_from_file("TEST3.JSD")) {
		return 1;
	}
	init_keyboard();
	set_mode(VGA_256_COLOR_MODE);
	setup_palette(unshaded_colors);

	gradient_buffer = malloc(sizeof(byte) * SCREEN_WIDTH * 4);
    memset(gradient_buffer, 0, sizeof(byte) * SCREEN_WIDTH * 4);

	memset(keystates, 0, 32);
	memset(old_keystates, 0, 32);


	while(!test_keybuf(keystates, KEY_ESC)) {
		point2 zero = {60, 0};
		point2 mid = {160 ,100};
		point2 topr = {260, 0};
		if(test_keybuf(keystates, KEY_RIGHT)) ang+=4;
		if(test_keybuf(keystates, KEY_LEFT)) ang-=4;
		cnt = (unsigned char) (ang >> 1) & 0xFF;
		if(test_keybuf(keystates, KEY_W)) {
			camerap.x += sine_table[cnt] >> 5;
			camerap.z += cosine_table[cnt] >> 5;
		}	
		if(test_keybuf(keystates, KEY_S)) {
			camerap.x -= sine_table[cnt] >> 5;
			camerap.z -= cosine_table[cnt] >> 5;
		}
		if(test_keybuf(keystates, KEY_A)) {
			camerap.x -= sine_table[(cnt+64) % 256] >> 5;
			camerap.z -= cosine_table[(cnt+64) % 256] >> 5;
		}	
		if(test_keybuf(keystates, KEY_D)) {
			camerap.x += sine_table[(cnt+64) % 256] >> 5;
			camerap.z += cosine_table[(cnt+64) % 256] >> 5;
		}

		if(!point_inside_sector(pnt_flat(camerap), world_sector_list[current_sector])) {
            for(i = 0; i < world_sector_list[current_sector].corner_count; i++) {
                if(world_sector_list[current_sector].neighbors[i] >= 0) {
                    if(point_inside_sector(pnt_flat(camerap), world_sector_list[world_sector_list[current_sector].neighbors[i]])) {
                        current_sector = world_sector_list[current_sector].neighbors[i];
                        /*viewer_z_pos = maxOf(viewer_z_pos, world_sector_list[current_sector].floor + viewer_height);
                        found_next_sector = true;*/
                        break;
                    }
                }
            }
        }

        recursive_sector_draw(current_sector, fullscreen, 0);

		
		/*draw_line_screen(zero, mid, 14);
		draw_line_screen(mid, topr, 14);*/
		wait_retrace();
		show_buffer();
		memset(graphic_buffer, 0, 64000L);
		memcpy(old_keystates, keystates, 32);
        /*printf("%ld, %ld, %ld\n0x%x\n", camerap, cnt);*/
        get_keys_hit(keystates);
	}

	deinit_keyboard();
	fade_out(4);
	set_mode(VGA_TEXT_MODE);
	free(gradient_buffer);
	return 0;
}