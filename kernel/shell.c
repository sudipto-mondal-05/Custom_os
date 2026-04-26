#define VIDEO_MEMORY 0xB8000
#define WHITE_ON_BLACK  0x0F
#define GREEN_ON_BLACK  0x0A
#define RED_ON_BLACK    0x04
#define CYAN_ON_BLACK   0x0B
#define YELLOW_ON_BLACK 0x0E
#define MAGENTA_ON_BLACK 0x05

void outb(unsigned short port, unsigned char val);
unsigned int timer_get_seconds();

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

// Forward declarations
void cmd_help_fn(char* args);
void cmd_clear_fn(char* args);
void cmd_echo_fn(char* args);
void cmd_about_fn(char* args);
void cmd_reboot_fn(char* args);
void cmd_shutdown_fn(char* args);
void cmd_uptime_fn(char* args);
void cmd_meminfo_fn(char* args);
void cmd_joke_fn(char* args);
void cmd_coinflip_fn(char* args);
void cmd_calc_fn(char* args);

// ── Command table ─────────────────────────────────────────
static command_t commands[] = {
    { "help",     "show this message",        cmd_help_fn      },
    { "clear",    "clear the screen",         cmd_clear_fn     },
    { "echo",     "print text",               cmd_echo_fn      },
    { "about",    "about this OS",            cmd_about_fn     },
    { "uptime",   "show uptime",              cmd_uptime_fn    },
    { "meminfo",  "show memory info",         cmd_meminfo_fn   },
    { "joke",     "tell a joke",              cmd_joke_fn      },
    { "coinflip", "flip a coin",              cmd_coinflip_fn  },
    { "calc",     "evaluate math (calc 2+3)", cmd_calc_fn      },
    { "reboot",   "reboot the machine",       cmd_reboot_fn    },
    { "shutdown", "power off the machine",    cmd_shutdown_fn  },
    { 0, 0, 0 }
};

// ── Screen functions ──────────────────────────────────────
void screen_clear() {
    unsigned char* vid = (unsigned char*) VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vid[i]   = ' ';
        vid[i+1] = WHITE_ON_BLACK;
    }
    row = 0; col = 0;
}

void shell_putchar(char c, unsigned char color) {
    unsigned char* vid = (unsigned char*) VIDEO_MEMORY;
    if (c == '\n') {
        row++; col = 0;
        if (row >= 25) {
            for (int i = 0; i < 24 * 80 * 2; i++)
                vid[i] = vid[i + 80 * 2];
            for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {
                vid[i] = ' '; vid[i+1] = WHITE_ON_BLACK;
            }
            row = 24;
        }
        return;
    }
    if (row >= 25) {
        for (int i = 0; i < 24 * 80 * 2; i++)
            vid[i] = vid[i + 80 * 2];
        for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {
            vid[i] = ' '; vid[i+1] = WHITE_ON_BLACK;
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

// ── String + Math helpers ─────────────────────────────────
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

void print_int(unsigned int n, unsigned char color) {
    if (n == 0) { shell_putchar('0', color); return; }
    char buf[12];
    int i = 0;
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    for (int j = i - 1; j >= 0; j--)
        shell_putchar(buf[j], color);
}

// ── Simple calc parser ────────────────────────────────────
// Supports: +, -, *, / with integers
// Operator precedence: * and / before + and -

int calc_parse_num(const char* s, int* i) {
    int result = 0;
    int neg = 0;
    if (s[*i] == '-') { neg = 1; (*i)++; }
    while (s[*i] >= '0' && s[*i] <= '9')
        result = result * 10 + (s[(*i)++] - '0');
    return neg ? -result : result;
}

int calc_eval(const char* expr) {
    // Two-pass: first handle * and /, then + and -
    // Store intermediate values
    int values[32];
    char ops[32];
    int vc = 0, oc = 0;
    int i = 0;

    // Skip spaces
    while (expr[i] == ' ') i++;

    values[vc++] = calc_parse_num(expr, &i);

    while (expr[i]) {
        while (expr[i] == ' ') i++;
        if (!expr[i]) break;

        char op = expr[i++];
        while (expr[i] == ' ') i++;
        int val = calc_parse_num(expr, &i);

        // Handle * and / immediately
        if (op == '*') {
            values[vc-1] *= val;
        } else if (op == '/') {
            if (val == 0) return -1;  // division by zero
            values[vc-1] /= val;
        } else {
            ops[oc++] = op;
            values[vc++] = val;
        }
    }

    // Now handle + and -
    int result = values[0];
    for (int j = 0; j < oc; j++) {
        if (ops[j] == '+') result += values[j+1];
        if (ops[j] == '-') result -= values[j+1];
    }
    return result;
}

// ── Simple random (LCG) ───────────────────────────────────
static unsigned int rand_seed = 12345;

unsigned int simple_rand() {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed >> 16) & 0x7FFF;
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
    shell_print("  MyOS v0.1",                              GREEN_ON_BLACK);  shell_newline();
    shell_print("  Built from scratch in C + Assembly",     WHITE_ON_BLACK);  shell_newline();
    shell_print("  Architecture : x86 32-bit Protected Mode", WHITE_ON_BLACK); shell_newline();
    shell_print("  Bootloader   : GRUB2 + Multiboot2",      WHITE_ON_BLACK);  shell_newline();
    shell_print("  Features     : IDT, PIC, Timer, Keyboard, Shell", WHITE_ON_BLACK); shell_newline();
    shell_print("  Author       : dark_avenger",             CYAN_ON_BLACK);   shell_newline();
}

void cmd_uptime_fn(char* args) {
    unsigned int secs = timer_get_seconds();
    unsigned int mins = secs / 60;
    unsigned int hrs  = mins / 60;
    secs %= 60;
    mins %= 60;
    shell_newline();
    shell_print("  Uptime: ", WHITE_ON_BLACK);
    print_int(hrs,  YELLOW_ON_BLACK); shell_print("h ", WHITE_ON_BLACK);
    print_int(mins, YELLOW_ON_BLACK); shell_print("m ", WHITE_ON_BLACK);
    print_int(secs, YELLOW_ON_BLACK); shell_print("s",  WHITE_ON_BLACK);
    shell_newline();
}

void cmd_meminfo_fn(char* args) {
    shell_newline();
    shell_print("  Memory Layout:", CYAN_ON_BLACK); shell_newline();
    shell_print("  0x00000000 - 0x000FFFFF  Low memory (1MB)",   WHITE_ON_BLACK); shell_newline();
    shell_print("  0x00100000 - 0x0010FFFF  MyOS Kernel",        GREEN_ON_BLACK); shell_newline();
    shell_print("  0x00090000              Stack",               WHITE_ON_BLACK); shell_newline();
    shell_print("  0x000B8000              VGA Video Memory",    YELLOW_ON_BLACK); shell_newline();
    shell_print("  0x000007C00             Bootloader (GRUB)",   WHITE_ON_BLACK); shell_newline();
    shell_newline();
    shell_print("  Note: No dynamic allocator yet (coming soon)", RED_ON_BLACK); shell_newline();
}

void cmd_joke_fn(char* args) {
    const char* jokes[] = {
        "Why do programmers prefer dark mode? Light attracts bugs.",
        "Why did the OS crash? It had too many processes to handle.",
        "A SQL query walks into a bar, walks up to two tables and asks: Can I join you?",
        "Why do Java developers wear glasses? Because they don't C#.",
        "I tried to write an OS in Python. It segfaulted in interpreted mode.",
        "There are 10 types of people: those who understand binary and those who don't.",
    };
    int count = 6;
    // Use uptime as seed for variety
    rand_seed += timer_get_seconds();
    int idx = simple_rand() % count;
    shell_newline();
    shell_print("  ", WHITE_ON_BLACK);
    shell_print(jokes[idx], YELLOW_ON_BLACK);
    shell_newline();
}

void cmd_coinflip_fn(char* args) {
    rand_seed += timer_get_seconds() + 1;
    int result = simple_rand() % 2;
    shell_newline();
    shell_print("  Flipping coin... ", WHITE_ON_BLACK);
    if (result)
        shell_print("HEADS!", CYAN_ON_BLACK);
    else
        shell_print("TAILS!", YELLOW_ON_BLACK);
    shell_newline();
}

void cmd_calc_fn(char* args) {
    shell_newline();
    if (shell_strlen(args) == 0) {
        shell_print("  Usage: calc <expression>", RED_ON_BLACK);
        shell_print("  Example: calc 2+3*4", WHITE_ON_BLACK);
        shell_newline();
        return;
    }
    int result = calc_eval(args);
    shell_print("  ", WHITE_ON_BLACK);
    shell_print(args, WHITE_ON_BLACK);
    shell_print(" = ", WHITE_ON_BLACK);
    if (result < 0) {
        shell_putchar('-', GREEN_ON_BLACK);
        print_int(-result, GREEN_ON_BLACK);
    } else {
        print_int(result, GREEN_ON_BLACK);
    }
    shell_newline();
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
    int i = 0;
    while (cmd[i] == ' ') i++;
    char* trimmed = cmd + i;

    if (shell_strlen(trimmed) == 0) return;

    char* args = trimmed;
    while (*args && *args != ' ') args++;
    if (*args == ' ') *args++ = 0;
    else args = "";

    for (int j = 0; commands[j].name != 0; j++) {
        if (shell_strcmp(trimmed, commands[j].name) == 0) {
            commands[j].handler(args);
            return;
        }
    }

    shell_newline();
    shell_print("Unknown: ", RED_ON_BLACK);
    shell_print(trimmed, RED_ON_BLACK);
    shell_newline();
    shell_print("Type 'help' for commands", WHITE_ON_BLACK);
    shell_newline();
}

void shell_init() {
    screen_clear();

    // Banner
    shell_print("  #     #  #     #   ####    ##### ", CYAN_ON_BLACK);   shell_newline();
    shell_print("  ##   ##  #     #  #    #  #      ", CYAN_ON_BLACK);   shell_newline();
    shell_print("  # # # #    # #    #    #   ####  ",  CYAN_ON_BLACK);   shell_newline();
    shell_print("  #  #  #     #     #    #       # ", CYAN_ON_BLACK);   shell_newline();
    shell_print("  #     #     #      ####   #####  ", CYAN_ON_BLACK);   shell_newline();
    shell_newline();
    shell_print("  Version 0.1                      ", YELLOW_ON_BLACK); shell_newline();
    shell_print("  32-bit x86 | GRUB2 | C + ASM     ", WHITE_ON_BLACK);  shell_newline();
    shell_print("  Type 'help' for available commands", WHITE_ON_BLACK);  shell_newline();
    shell_newline();
    shell_print("  ================================= ", GREEN_ON_BLACK);
    shell_prompt();
}
