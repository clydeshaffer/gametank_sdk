extern char global_tick;
extern char game_state;
extern char field[256];
extern char lives;
extern int score;

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