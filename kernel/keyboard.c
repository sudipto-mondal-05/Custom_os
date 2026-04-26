#define VIDEO_MEMORY 0xB8000
#define WHITE_ON_BLACK 0x0F

void outb(unsigned short port, unsigned char val);
unsigned char inb(unsigned short port);

extern char cmd_buffer[];
extern int col;
void shell_putchar(char c, unsigned char color);
void shell_process(char* cmd);
void shell_prompt();
int shell_get_cursor_pos();

static int cmd_len  = 0;
static int shift_held = 0;

void update_cursor(int pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// Normal scancode map (no shift)
static const char scancode_normal[] = {
    0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=',
    0x08, 0,
    'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,
    ' ','7','8','9',
    '-','4','5','6',
    '+','1','2','3',
    '0','.', 0
};

// Shifted scancode map
static const char scancode_shift[] = {
    0,   0,  '!','@','#','$','%','^','&','*','(',')','_','+',
    0x08, 0,
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','"','~', 0,'|',
    'Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,
    ' ','7','8','9',
    '-','4','5','6',
    '+','1','2','3',
    '0','.', 0
};

void keyboard_handler() {
    unsigned char* vid = (unsigned char*) VIDEO_MEMORY;
    unsigned char scancode = inb(0x60);

    // Shift press
    if (scancode == 0x2A || scancode == 0x36) { shift_held = 1; outb(0x20, 0x20); return; }
    // Shift release
    if (scancode == 0xAA || scancode == 0xB6) { shift_held = 0; outb(0x20, 0x20); return; }

    // Ignore key releases
    if (scancode & 0x80) { outb(0x20, 0x20); return; }

    if (scancode >= sizeof(scancode_normal)) { outb(0x20, 0x20); return; }

    char c = shift_held ? scancode_shift[scancode] : scancode_normal[scancode];

    if (c == 0x08) {
        if (cmd_len > 0) {
            cmd_len--;
            cmd_buffer[cmd_len] = 0;
            int pos = shell_get_cursor_pos();
            if (pos > 0) {
                pos--;
                vid[pos * 2]     = ' ';
                vid[pos * 2 + 1] = WHITE_ON_BLACK;
                col--;
            }
        }
    } else if (c == '\n') {
        cmd_buffer[cmd_len] = 0;
        shell_process(cmd_buffer);
        cmd_len = 0;
        cmd_buffer[0] = 0;
        shell_prompt();
    } else if (c != 0 && cmd_len < 255) {
        cmd_buffer[cmd_len++] = c;
        cmd_buffer[cmd_len]   = 0;
        shell_putchar(c, WHITE_ON_BLACK);
    }

    update_cursor(shell_get_cursor_pos());
    outb(0x20, 0x20);
}
