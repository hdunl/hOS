typedef unsigned int size_t;

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f

#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

int get_screen_offset(int col, int row);
int get_cursor();
void set_cursor(int offset);
int print_char(char character, int col, int row, char attribute_byte);
void *memcpy(void *dest, const void *src, size_t n);
int handle_scrolling(int cursor_offset);

void clear_screen() {
    int row, col;
    for (row = 0; row < MAX_ROWS; row++) {
        for (col = 0; col < MAX_COLS; col++) {
            print_char(' ', col, row, WHITE_ON_BLACK);
        }
    }
    set_cursor(get_screen_offset(0, 0));
}

void kprint_at(char *message, int col, int row) {
    int offset;
    if (col >= 0 && row >= 0) {
        offset = get_screen_offset(col, row);
    } else {
        offset = get_cursor();
        row = offset / (2 * MAX_COLS);
        col = (offset - (row * 2 * MAX_COLS)) / 2;
    }

    int i = 0;
    while (message[i] != 0) {
        offset = print_char(message[i++], col, row, WHITE_ON_BLACK);
        row = offset / (2 * MAX_COLS);
        col = (offset - (row * 2 * MAX_COLS)) / 2;
    }
}

void kprint(char *message) {
    kprint_at(message, -1, -1);
}

void kernel_main() {
    clear_screen();
    kprint("Hello, World! Welcome to hOS!");
}

int get_screen_offset(int col, int row) {
    return 2 * (row * MAX_COLS + col);
}

int get_cursor() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2;
}

void set_cursor(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

int print_char(char character, int col, int row, char attribute_byte) {
    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;

    if (!attribute_byte) {
        attribute_byte = WHITE_ON_BLACK;
    }

    int offset;
    if (col >= 0 && row >= 0) {
        offset = get_screen_offset(col, row);
    } else {
        offset = get_cursor();
    }

    if (character == '\n') {
        int rows = offset / (2 * MAX_COLS);
        offset = get_screen_offset(79, rows);
    } else {
        vidmem[offset] = character;
        vidmem[offset + 1] = attribute_byte;
    }

    offset += 2;
    offset = handle_scrolling(offset);
    set_cursor(offset);
    return offset;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int handle_scrolling(int cursor_offset) {
    if (cursor_offset < MAX_ROWS * MAX_COLS * 2) {
        return cursor_offset;
    }

    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;

    int i;
    for (i = 1; i < MAX_ROWS; i++) {
        unsigned char *dest = vidmem + get_screen_offset(0, i);
        unsigned char *src = vidmem + get_screen_offset(0, i - 1);
        memcpy(dest, src, MAX_COLS * 2);
    }

    unsigned char *last_line = vidmem + get_screen_offset(0, MAX_ROWS - 1);
    for (i = 0; i < MAX_COLS * 2; i++) {
        last_line[i] = 0;
    }

    cursor_offset -= 2 * MAX_COLS;
    return cursor_offset;
}

unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}
