typedef struct twobytes_t {
    char lsb, msb;
} twobytes_t;

typedef union {
    twobytes_t b;
    char* p;
} pointer_t;

extern pointer_t bittext_dest_u;
extern char bittext_color;
extern char bittext_bg;

#define bittext_dest bittext_dest_u.p

void bittext_write_char(char c);

void bittext_set_cursor(char x, char y);

void bittext_erase_char();
void bittext_write_char_opaque(char c);
void bittext_write_str_opaque(char* c);