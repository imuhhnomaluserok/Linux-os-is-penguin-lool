/*
 * sooome cooooool os
 * A tiny freestanding 32-bit framebuffer kernel.
 *
 * GRUB provides the framebuffer through the Multiboot2 information structure.
 * This deliberately has no libc dependency so it can boot on real x86
 * hardware, VirtualBox, and QEMU.
 */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int uintptr_t;

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289u
#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

typedef struct {
    uint32_t type;
    uint32_t size;
} multiboot_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
} multiboot_tag_framebuffer_common_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
    uint8_t red_position;
    uint8_t red_mask_size;
    uint8_t green_position;
    uint8_t green_mask_size;
    uint8_t blue_position;
    uint8_t blue_mask_size;
} multiboot_tag_framebuffer_rgb_t;

typedef struct {
    uint8_t *address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t red_position;
    uint8_t green_position;
    uint8_t blue_position;
} screen_t;

static screen_t screen;

static uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)red << screen.red_position) |
           ((uint32_t)green << screen.green_position) |
           ((uint32_t)blue << screen.blue_position);
}

static void pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= screen.width || y >= screen.height) {
        return;
    }

    uint32_t *row = (uint32_t *)(screen.address + y * screen.pitch);
    row[x] = color;
}

static void rectangle(uint32_t x, uint32_t y, uint32_t width,
                      uint32_t height, uint32_t color) {
    for (uint32_t row = 0; row < height && y + row < screen.height; row++) {
        for (uint32_t column = 0;
             column < width && x + column < screen.width;
             column++) {
            pixel(x + column, y + row, color);
        }
    }
}

static void line(uint32_t x, uint32_t y, uint32_t width, uint32_t color) {
    rectangle(x, y, width, 1, color);
}

/* A compact 5x7 font for the desktop labels. */
static const uint8_t font[][7] = {
    {0x1e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}, /* A */
    {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e}, /* B */
    {0x0f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0f}, /* C */
    {0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e}, /* D */
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f}, /* E */
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10}, /* F */
    {0x0f, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0f}, /* G */
    {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}, /* H */
    {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1f}, /* I */
    {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0e}, /* J */
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, /* K */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f}, /* L */
    {0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11}, /* M */
    {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}, /* N */
    {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}, /* O */
    {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10}, /* P */
    {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d}, /* Q */
    {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11}, /* R */
    {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e}, /* S */
    {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, /* T */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}, /* U */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04}, /* V */
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11}, /* W */
    {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11}, /* X */
    {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04}, /* Y */
    {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f}, /* Z */
};

static void character(uint32_t x, uint32_t y, char value, uint32_t color) {
    if (value < 'A' || value > 'Z') {
        return;
    }

    const uint8_t *glyph = font[value - 'A'];
    for (uint32_t row = 0; row < 7; row++) {
        for (uint32_t column = 0; column < 5; column++) {
            if (glyph[row] & (1u << (4u - column))) {
                rectangle(x + column * 2, y + row * 2, 2, 2, color);
            }
        }
    }
}

static void text(uint32_t x, uint32_t y, const char *value, uint32_t color) {
    while (*value != '\0') {
        character(x, y, *value, color);
        x += 14;
        value++;
    }
}

static void draw_cursor(uint32_t x, uint32_t y, uint32_t color) {
    for (uint32_t row = 0; row < 14; row++) {
        for (uint32_t column = 0; column <= row / 2; column++) {
            pixel(x + column, y + row, color);
        }
    }
    rectangle(x + 3, y + 8, 3, 5, color);
}

static void draw_desktop(void) {
    uint32_t navy = rgb(20, 29, 56);
    uint32_t blue = rgb(49, 104, 181);
    uint32_t panel = rgb(235, 239, 247);
    uint32_t ink = rgb(24, 31, 48);
    uint32_t white = rgb(255, 255, 255);
    uint32_t accent = rgb(98, 219, 190);
    uint32_t shadow = rgb(13, 18, 35);

    rectangle(0, 0, screen.width, screen.height, navy);
    rectangle(0, 0, screen.width, screen.height * 3 / 5, blue);

    uint32_t window_width = screen.width * 2 / 3;
    uint32_t window_height = screen.height * 2 / 3;
    uint32_t window_x = (screen.width - window_width) / 2;
    uint32_t window_y = (screen.height - window_height) / 2 - 12;

    rectangle(window_x + 8, window_y + 8, window_width, window_height, shadow);
    rectangle(window_x, window_y, window_width, window_height, panel);
    rectangle(window_x, window_y, window_width, 34, ink);
    rectangle(window_x + 12, window_y + 11, 12, 12, accent);
    rectangle(window_x + window_width - 62, window_y + 12, 10, 3, panel);
    rectangle(window_x + window_width - 40, window_y + 10, 10, 10, panel);
    rectangle(window_x + window_width - 18, window_y + 10, 10, 10, accent);
    text(window_x + 36, window_y + 11, "WELCOME", white);

    text(window_x + 28, window_y + 70, "SOOOME", ink);
    text(window_x + 28, window_y + 96, "COOOOOL", ink);
    line(window_x + 28, window_y + 123, window_width - 56, rgb(205, 211, 223));
    text(window_x + 28, window_y + 145, "YOUR DESKTOP IS READY", blue);

    uint32_t taskbar_height = 44;
    rectangle(0, screen.height - taskbar_height, screen.width, taskbar_height, ink);
    rectangle(14, screen.height - 34, 24, 24, accent);
    rectangle(20, screen.height - 28, 12, 2, ink);
    rectangle(25, screen.height - 33, 2, 12, ink);
    text(52, screen.height - 32, "START", white);
    text(screen.width - 110, screen.height - 32, "READY", accent);
    draw_cursor(screen.width / 2 + 60, screen.height / 2 + 50, white);
}

static int find_framebuffer(uint32_t info_address) {
    uint32_t total_size = *(uint32_t *)info_address;
    uint32_t offset = 8;

    while (offset < total_size) {
        multiboot_tag_t *tag = (multiboot_tag_t *)(info_address + offset);
        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            multiboot_tag_framebuffer_rgb_t *framebuffer =
                (multiboot_tag_framebuffer_rgb_t *)tag;
            if (framebuffer->framebuffer_type != 1 ||
                framebuffer->framebuffer_bpp != 32) {
                return 0;
            }

            screen.address = (uint8_t *)(uintptr_t)framebuffer->framebuffer_addr;
            screen.pitch = framebuffer->framebuffer_pitch;
            screen.width = framebuffer->framebuffer_width;
            screen.height = framebuffer->framebuffer_height;
            screen.bpp = framebuffer->framebuffer_bpp;
            screen.red_position = framebuffer->red_position;
            screen.green_position = framebuffer->green_position;
            screen.blue_position = framebuffer->blue_position;
            return 1;
        }

        offset = (offset + tag->size + 7) & ~7u;
    }

    return 0;
}

void kernel_main(uint32_t magic, uint32_t info_address) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC ||
        !find_framebuffer(info_address)) {
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    draw_desktop();
    for (;;) {
        __asm__ volatile ("hlt");
    }
}