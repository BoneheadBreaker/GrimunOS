// =============================================================================
// GrimunOS Kernel - under MPL 2.0 licensing
// =============================================================================

typedef unsigned char u8;
typedef unsigned short u16;
static const char* GRIMUNOS_VERSION = "0.0.5";

static inline u8 inb(u16 port) {
    u8 r;
    __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void outb(u16 port, u8 v) {
    __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(port));
}

int cursor = 0;

void clear_screen() {
    volatile char* v = (volatile char*)0xB8000;

    for (int i = 0; i < 80 * 25; i++) {
        v[i * 2] = ' ';
        v[i * 2 + 1] = 0x0F;
    }

    cursor = 0;
}

void put(char c) {
    volatile char* v = (volatile char*)0xB8000;

    if (c == '\n') {
        cursor = (cursor / 80 + 1) * 80;
        return;
    }

    if (c == '\b') {
        if (cursor > 0) {
            cursor--;
            v[cursor * 2] = ' ';
            v[cursor * 2 + 1] = 0x0F;
        }
        return;
    }

    v[cursor * 2] = c;
    v[cursor * 2 + 1] = 0x0F;
    cursor++;

    if (cursor >= 80 * 25) cursor = 0;
}

void print(const char* s) {
    while (*s) put(*s++);
}

int cmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int len(const char* s) {
    int i = 0;
    while (s[i]) i++;
    return i;
}

#define KEY_NONE 0
#define KEY_UP   1
#define KEY_DOWN 2

int getchar() {
    u8 status;

    while (1) {
        status = inb(0x64);

        if (status & 1) {
            u8 sc = inb(0x60);

            if (sc & 0x80) continue;

            switch (sc) {
                case 0x1E: return 'a';
                case 0x30: return 'b';
                case 0x2E: return 'c';
                case 0x20: return 'd';
                case 0x12: return 'e';
                case 0x21: return 'f';
                case 0x22: return 'g';
                case 0x23: return 'h';
                case 0x17: return 'i';
                case 0x24: return 'j';
                case 0x25: return 'k';
                case 0x26: return 'l';
                case 0x32: return 'm';
                case 0x31: return 'n';
                case 0x18: return 'o';
                case 0x19: return 'p';
                case 0x10: return 'q';
                case 0x13: return 'r';
                case 0x1F: return 's';
                case 0x14: return 't';
                case 0x16: return 'u';
                case 0x2F: return 'v';
                case 0x11: return 'w';
                case 0x2D: return 'x';
                case 0x15: return 'y';
                case 0x2C: return 'z';

                case 0x39: return ' ';
                case 0x1C: return '\n';
                case 0x0E: return '\b';

                case 0x48: return KEY_UP;
                case 0x50: return KEY_DOWN;

                default: return 0;
            }
        }
    }
}

#define HISTORY_SIZE 16
#define BUF_SIZE 128

char history[HISTORY_SIZE][BUF_SIZE];
int history_count = 0;
int history_index = 0;

void readline(char* buf, int max) {
    int i = 0;
    history_index = history_count;

    while (1) {
        int c = getchar();
        if (!c) continue;

        // ENTER
        if (c == '\n') {
            put('\n');

            if (i > 0) {
                int slot = history_count % HISTORY_SIZE;

                for (int k = 0; k < i; k++)
                    history[slot][k] = buf[k];

                history[slot][i] = 0;
                history_count++;
            }

            buf[i] = 0;
            return;
        }

        // BACKSPACE
        if (c == '\b') {
            if (i > 0) {
                i--;
                put('\b');
            }
            continue;
        }

        // UP ARROW
        if (c == KEY_UP) {
            if (history_count == 0) continue;

            if (history_index > 0) history_index--;
            else history_index = 0;

            while (i > 0) { put('\b'); i--; }

            char* h = history[history_index % HISTORY_SIZE];
            int j = 0;

            while (h[j] && j < max - 1) {
                buf[j] = h[j];
                put(h[j]);
                j++;
            }

            i = j;
            buf[i] = 0;
            continue;
        }

        // DOWN ARROW
        if (c == KEY_DOWN) {
            if (history_index < history_count)
                history_index++;

            while (i > 0) { put('\b'); i--; }

            if (history_index >= history_count) {
                buf[0] = 0;
                i = 0;
                continue;
            }

            char* h = history[history_index % HISTORY_SIZE];
            int j = 0;

            while (h[j] && j < max - 1) {
                buf[j] = h[j];
                put(h[j]);
                j++;
            }

            i = j;
            buf[i] = 0;
            continue;
        }

        // NORMAL CHAR
        if (i < max - 1) {
            buf[i++] = (char)c;
            put((char)c);
        }
    }
}

const char* skip_spaces(const char* s) {
    while (*s == ' ') s++;
    return s;
}

void extract_arg(const char* input, char* out) {
    input = skip_spaces(input);

    int i = 0;

    if (*input == '"') {
        input++;

        while (*input && *input != '"' && i < 127) {
            out[i++] = *input++;
        }

        out[i] = 0;
        return;
    }

    while (*input && i < 127) {
        out[i++] = *input++;
    }

    out[i] = 0;
}

struct Cmd {
    const char* name;
    void (*fn)();
};

void cmd_help() {
    print("   help      - displays this message\n");
    print("   about     - displays a message about the OS\n");
    print("   clear     - clears the terminal\n");
    print("   reboot    - reboots the operating system\n");
    print("   say <msg> - get the operating system to say something\n");
}

void cmd_clear() {
    clear_screen();
}

void cmd_about() {
    print("GrimunOS\n");
    print("Version: ");
    print(GRIMUNOS_VERSION);
    print("\n");
}

void cmd_reboot() {
    outb(0x64, 0xFE);
    while (1);
}

Cmd cmds[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"about", cmd_about},
    {"reboot", cmd_reboot}
};

int cmd_count = sizeof(cmds) / sizeof(Cmd);

void run(char* input) {
    if (!input[0]) return;

    char cmd[32];
    int i = 0;
    const char* p = input;

    while (*p && *p != ' ' && i < 31) {
        cmd[i++] = *p++;
    }

    cmd[i] = 0;

    const char* args = skip_spaces(p);

    if (cmp(cmd, "say") == 0) {
        char msg[128];
        extract_arg(args, msg);
        print(msg);
        print("\n");
        return;
    }

    for (int i = 0; i < cmd_count; i++) {
        if (cmp(cmd, cmds[i].name) == 0) {
            cmds[i].fn();
            return;
        }
    }

    print("Unknown command\n");
}

extern "C" void kernel_main() {
    clear_screen();

    print("GrimunOS booted\n");
    print("Type help\n\n");

    char buf[128];

    while (1) {
        print("grimun_os> ");
        readline(buf, 128);
        run(buf);
    }
}