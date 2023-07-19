extern char global_tick;
extern char game_state;
extern char state_timer;
extern char field[256];
extern char lives;
extern unsigned int score;
extern char field_offset_x, field_offset_y;
extern char level_num;

char delta(char a, char b);
void clear_field();
void draw_field(char tile_bank);

typedef struct {
    char lsb, msb;
} twobytes;

typedef union {
    unsigned int i;
    twobytes b;
} coordinate;