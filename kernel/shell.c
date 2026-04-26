#define VIDEO_MEMORY 0xB8000
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define RED_ON_BLACK   0x04
#define CYAN_ON_BLACK  0x0B

void outb(unsigned short port, unsigned char val);

// ── Screen state ──────────────────────────────────────────
int row = 0;
int col = 0;

char cmd_buffer[256];

// ── Command table type ────────────────────────────────────
typedef struct {
    const char* name;
    const char* description;
    void (*handler)(char* args);
} command_t;

// Forward declare all command functions
void cmd_help_fn(char* args);
void cmd_clear_fn(char* args);
void cmd_echo_fn(char* args);
void cmd_about_fn(char* args);
void cmd_reboot_fn(char* args);
void cmd_shutdown_fn(char* args);

// ── Command table ─────────────────────────────────────────
static command_t commands[] = {
    { "help",     "show this message",     cmd_help_fn     },
    { "clear",    "clear the screen",      cmd_clear_fn    },
    { "echo",     "print text",            cmd_echo_fn     },
    { "about",    "about this OS",         cmd_about_fn    },
    { "reboot",   "reboot the machine",    cmd_reboot_fn   },
    { "shutdown", "power off the machine", cmd_shutdown_fn },
    { 0, 0, 0 }  // null terminator
};

// ── Screen functions ──────────────────────────────────────
void screen_clear() {
    unsigned char* vid = (unsigned char*) VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vid[i]   = ' ';
        vid[i+1] = WHITE_ON_BLACK;
    }
    row = 0;
    col = 0;
}

void shell_putchar(char c, unsigned char color) {
    unsigned char* vid = (unsigned char*) VIDEO_MEMORY;
    if (c == '\n') {
        row++;
        col = 0;
        if (row >= 25) {
            for (int i = 0; i < 24 * 80 * 2; i++)
                vid[i] = vid[i + 80 * 2];
            for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {
                vid[i]   = ' ';
                vid[i+1] = WHITE_ON_BLACK;
            }
            row = 24;
        }
        return;
    }
    if (row >= 25) {
        for (int i = 0; i < 24 * 80 * 2; i++)
            vid[i] = vid[i + 80 * 2];
        for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {
            vid[i]   = ' ';
            vid[i+1] = WHITE_ON_BLACK;
        }
        row = 24;
    }
    int pos = row * 80 + col;
    vid[pos * 2]     = c;
    vid[pos * 2 + 1] = color;
    col++;
    if (col >= 80) { col = 0; row++; }
}

void shell_print(const char* msg, unsigned char color) {
    for (int i = 0; msg[i] != 0; i++)
        shell_putchar(msg[i], color);
}

void shell_newline() {
    shell_putchar('\n', WHITE_ON_BLACK);
}

int shell_get_cursor_pos() {
    return row * 80 + col;
}

// ── String helpers ────────────────────────────────────────
int shell_strcmp(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] - b[i];
}

int shell_strlen(const char* s) {
    int i = 0;
    while (s[i]) i++;
    return i;
}

// ── Command implementations ───────────────────────────────
void cmd_help_fn(char* args) {
    shell_newline();
    shell_print("Available commands:", CYAN_ON_BLACK);
    shell_newline();
    for (int i = 0; commands[i].name != 0; i++) {
        shell_print("  ", WHITE_ON_BLACK);
        shell_print(commands[i].name, GREEN_ON_BLACK);
        shell_print(" - ", WHITE_ON_BLACK);
        shell_print(commands[i].description, WHITE_ON_BLACK);
        shell_newline();
    }
}

void cmd_clear_fn(char* args) {
    screen_clear();
}

void cmd_echo_fn(char* args) {
    shell_newline();
    if (shell_strlen(args) == 0)
        shell_print("Usage: echo <text>", RED_ON_BLACK);
    else
        shell_print(args, WHITE_ON_BLACK);
    shell_newline();
}

void cmd_about_fn(char* args) {
    shell_newline();
    shell_print("  MyOS v0.1",                           GREEN_ON_BLACK); shell_newline();
    shell_print("  Built from scratch in C + Assembly",  WHITE_ON_BLACK); shell_newline();
    shell_print("  x86 32-bit Protected Mode",           WHITE_ON_BLACK); shell_newline();
    shell_print("  Bootloader : GRUB2 + Multiboot2",     WHITE_ON_BLACK); shell_newline();
    shell_print("  Features   : IDT, PIC, Keyboard, Shell", WHITE_ON_BLACK); shell_newline();
}

void cmd_reboot_fn(char* args) {
    shell_newline();
    shell_print("Rebooting...", RED_ON_BLACK);
    outb(0x64, 0xFE);
}

void cmd_shutdown_fn(char* args) {
    shell_newline();
    shell_print("Shutting down...", RED_ON_BLACK);
    outb(0x501, 0x31);
}


// ── Shell core ────────────────────────────────────────────
void shell_prompt() {
    shell_newline();
    shell_print("MyOS> ", GREEN_ON_BLACK);
}

void shell_process(char* cmd) {
    // Skip leading spaces
    int i = 0;
    while (cmd[i] == ' ') i++;
    char* trimmed = cmd + i;

    if (shell_strlen(trimmed) == 0) return;

    // Split command name and args at first space
    char* args = trimmed;
    while (*args && *args != ' ') args++;
    if (*args == ' ') *args++ = 0;
    else args = "";

    // Search command table
    for (int j = 0; commands[j].name != 0; j++) {
        if (shell_strcmp(trimmed, commands[j].name) == 0) {
            commands[j].handler(args);
            return;
        }
    }

    // Not found
    shell_newline();
    shell_print("Unknown command: ", RED_ON_BLACK);
    shell_print(trimmed, RED_ON_BLACK);
    shell_newline();
    shell_print("Type 'help' for commands", WHITE_ON_BLACK);
    shell_newline();
}

void shell_init() {
    screen_clear();
    shell_print("  #     #  #     #   ####    ##### ", CYAN_ON_BLACK); shell_newline();
    shell_print("  ##   ##  #     #  #    #  #      ", CYAN_ON_BLACK); shell_newline();
    shell_print("  # # # #    # #    #    #   ####  ", CYAN_ON_BLACK);  shell_newline();
    shell_print("  #  #  #     #     #    #       # ", CYAN_ON_BLACK); shell_newline();
    shell_print("  #     #     #      ####   #####  ", CYAN_ON_BLACK); shell_newline();
    shell_newline();
    shell_print("  Version 0.1 | Type 'help' for commands", WHITE_ON_BLACK);
    shell_prompt();
}
