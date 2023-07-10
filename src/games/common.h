extern char global_tick;
extern char game_state;
extern char field[256];

void clear_field();
void draw_field(char tile_bank);

typedef struct {
    char lsb, msb;
} twobytes;

typedef union {
    unsigned int i;
    twobytes b;
} coordinate;