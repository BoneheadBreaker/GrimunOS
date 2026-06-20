// =============================================================================
// GrimunOS Kernel - Localized Stack Implementation (Zero Linker Dependencies)
// =============================================================================

// Read a raw byte from an x86 hardware I/O port
inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Global integer tracks the screen cursor index safely
int cursor_pointer = 0;

void clear_screen() {
    volatile char* video_memory = (volatile char*)0xB8000;
    for (int i = 0; i < 2000; i++) {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = 0x0F; // White text on Black background
    }
    cursor_pointer = 0;
}

void print_char(char c) {
    volatile char* video_memory = (volatile char*)0xB8000;
    
    if (c == '\n') {
        cursor_pointer = ((cursor_pointer / 80) + 1) * 80;
    } 
    else if (c == '\b') {
        if (cursor_pointer > 0) {
            cursor_pointer--;
            video_memory[cursor_pointer * 2] = ' ';
            video_memory[cursor_pointer * 2 + 1] = 0x0F;
        }
    } 
    else {
        video_memory[cursor_pointer * 2] = c;
        video_memory[cursor_pointer * 2 + 1] = 0x0F;
        cursor_pointer++;
    }
    
    if (cursor_pointer >= 2000) {
        clear_screen();
    }
}

void print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

// Custom strcmp localized to prevent external C-library linker lookups
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Custom helper function to calculate the length of a string buffer safely
int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Blocks execution and pulls characters using a strictly local keymap matrix array
char get_char() {
    char local_scan_map[128];
    for(int i = 0; i < 128; i++) local_scan_map[i] = 0;
    
    // Explicit inline mapping assignments
    local_scan_map[2]   = '1'; local_scan_map[3]   = '2'; local_scan_map[4]   = '3';
    local_scan_map[5]   = '4'; local_scan_map[6]   = '5'; local_scan_map[7]   = '6';
    local_scan_map[8]   = '7'; local_scan_map[9]   = '8'; local_scan_map[10]  = '9';
    local_scan_map[11]  = '0'; local_scan_map[12]  = '-'; local_scan_map[13]  = '=';
    local_scan_map[14]  = '\b'; // Backspace
    local_scan_map[16]  = 'q'; local_scan_map[17]  = 'w'; local_scan_map[18]  = 'e';
    local_scan_map[19]  = 'r'; local_scan_map[20]  = 't'; local_scan_map[21]  = 'y';
    local_scan_map[22]  = 'u'; local_scan_map[23]  = 'i'; local_scan_map[24]  = 'o';
    local_scan_map[25]  = 'p'; 
    local_scan_map[28]  = '\n'; // Enter key
    local_scan_map[30]  = 'a'; local_scan_map[31]  = 's'; local_scan_map[32]  = 'd';
    local_scan_map[33]  = 'f'; local_scan_map[34]  = 'g'; local_scan_map[35]  = 'h';
    local_scan_map[36]  = 'j'; local_scan_map[37]  = 'k'; local_scan_map[38]  = 'l';
    local_scan_map[44]  = 'z'; local_scan_map[45]  = 'x'; local_scan_map[46]  = 'c';
    local_scan_map[47]  = 'v'; local_scan_map[48]  = 'b'; local_scan_map[49]  = 'n';
    local_scan_map[50]  = 'm'; local_scan_map[57]  = ' ';  // Spacebar

    while (1) {
        if (inb(0x64) & 1) {
            unsigned char scancode = inb(0x60);
            
            // Key release filter flag
            if (scancode & 0x80) {
                for (volatile int i = 0; i < 20000; i++);
                continue;
            }
            
            if (scancode < 128) {
                char processed_char = local_scan_map[scancode];
                if (processed_char > 0) {
                    for (volatile int i = 0; i < 20000; i++);
                    return processed_char;
                }
            }
        }
    }
}

void read_line(char* buffer, int max_length) {
    int index = 0;
    while (index < max_length - 1) {
        char c = get_char();
        
        if (c == '\n') {
            print_char('\n');
            break;
        } 
        else if (c == '\b') {
            if (index > 0) {
                index--;
                print_char('\b');
            }
        } 
        else {
            buffer[index] = c;
            index++;
            print_char(c); 
        }
    }
    buffer[index] = '\0'; 
}

// =============================================================================
// MODULAR COMMAND REGISTRY INTERFACES
// =============================================================================

struct TerminalCommand {
    const char* name;
    void (*function)();
};

void command_help() {
    print_string("Available tools:\n");
    print_string("  help       - Show this menu layout\n");
    print_string("  clear      - Reset terminal window view\n");
    print_string("  say <text> - Repeat any text argument typed\n");
    print_string("  about      - Displays infomation about GrimunOS\n");
}

void about_command() {
    print_string("About:\n");
    print_string("  GrimunOS is a mini operating system\n");
    print_string("  \n");
    print_string("  A major design decision about GrimiunOS is capital letters are 'forbidden'\n");
    print_string("  however in dialogs like these they are used by GrimunOS itself\n");
}

TerminalCommand command_registry[] = {
    {"help", command_help},
    {"clear", clear_screen},
    {"about", about_command}
};

const int total_commands = sizeof(command_registry) / sizeof(TerminalCommand);

// Dynamic command parser that processes "say <text>" arguments
void handle_say_command(const char* full_input) {
    if (strlen(full_input) <= 4) {
        print_string("Usage: say <text>\n");
        return;
    }

    // Skip the first 4 characters ("say ") to extract the text argument directly
    const char* message_to_print = full_input + 4;
    print_string(message_to_print);
    print_char('\n');
}

void process_command(const char* input) {
    if (input[0] == '\0') return;

    // 1. DYNAMIC PREFIX CHECKS (For commands that take arguments)
    if (input[0] == 's' && input[1] == 'a' && input[2] == 'y' && input[3] == ' ') {
        handle_say_command(input);
        return;
    }

    // 2. STATIC STRING MATCHES (For standalone commands)
    for (int i = 0; i < total_commands; i++) {
        if (strcmp(input, command_registry[i].name) == 0) {
            command_registry[i].function();
            return;
        }
    }
    print_string("Unknown command. Write 'help'\n");
}

extern "C" void kernel_main() {
    clear_screen();
    
    print_string("Hello from GrimunOS!\n");
    print_string("Terminal processing online. Type 'help' for commands:\n\n");
    
    char command_buffer[256];
    
    while (1) {
        print_string("grimun_os> ");
        read_line(command_buffer, 256);
        process_command(command_buffer);
    }
}
