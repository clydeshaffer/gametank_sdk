extern char bittext_color;
extern char bittext_bg;

#define bittext_dest bittext_dest_u.p

void bittext_draw_char(char c);
void bittext_set_x(char x);
void bittext_set_y(char y);

void bittext_write_str(char* c);