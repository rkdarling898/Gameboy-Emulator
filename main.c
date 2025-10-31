#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "common.h"
#include "cartridge_header.h"
#include "sm83.h"

#define MEMORY_MAX 8388608
#define ROM_GB 1
#define ROM_GB_COLOR 2

#define BORDER_WIDTH 5
#define SCREEN_MULTIPLIER 4
#define RESOLUTION_WIDTH 160
#define RESOLUTION_HEIGHT 144
#define WINDOW_SIZE 1000
#define SCREEN_WIDTH RESOLUTION_WIDTH * SCREEN_MULTIPLIER
#define SCREEN_HEIGHT RESOLUTION_HEIGHT * SCREEN_MULTIPLIER
#define SCREEN_X (WINDOW_SIZE - (SCREEN_WIDTH * 1.25)) / 2
#define SCREEN_Y (WINDOW_SIZE - SCREEN_HEIGHT) / 2

uint8_t gb_rom_type (char *filePath) {
    int len = strlen(filePath);
    char *ext2 = (char *)(filePath + len - 3);
    char *ext3 = (char *)(filePath + len - 4);

    if (strcmp(ext2, ".gb") == 0) {
        return ROM_GB;
    } else if (strcmp(ext3, ".gbc") == 0) {
        return ROM_GB_COLOR;
    }

    return 0;
}

void error (const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_usage (const char *program_name) {
    // Just setting this up to potentially take some options and flags later on
    printf("%s%s%s", "Usage: ", program_name, " (file.gb / file.gbc) [-o] [-f]\n");
    exit(EXIT_SUCCESS);
}

void render_screen (SDL_Window *window, SDL_Renderer *renderer) {
    SDL_FRect screen = {
        SCREEN_X,
        SCREEN_Y,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    };
    SDL_FRect debug_window = {
        SCREEN_X + SCREEN_WIDTH,
        SCREEN_Y,
        SCREEN_WIDTH / 4,
        SCREEN_HEIGHT
    };
    SDL_FRect h_line = {
        SCREEN_X - BORDER_WIDTH + 1,
        SCREEN_Y - BORDER_WIDTH + 1,
        SCREEN_WIDTH * 1.25 + (BORDER_WIDTH * 2) - 2,
        BORDER_WIDTH
    };
    SDL_FRect v_line = {
        SCREEN_X - BORDER_WIDTH + 1,
        SCREEN_Y - BORDER_WIDTH + 1,
        BORDER_WIDTH,
        SCREEN_HEIGHT + (BORDER_WIDTH) - 2
    };

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 155, 155, 189, SDL_ALPHA_OPAQUE);

    SDL_RenderFillRect(renderer, &h_line);
    SDL_RenderFillRect(renderer, &v_line);

    h_line.y += SCREEN_HEIGHT - 1;
    v_line.x = debug_window.x + debug_window.w - 1;

    SDL_RenderFillRect(renderer, &h_line);
    SDL_RenderFillRect(renderer, &v_line);

    SDL_RenderRect(renderer, &screen);
    SDL_RenderFillRect(renderer, &debug_window);
}

int render_text (SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color, SDL_FRect *d_rect, const char *str) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = TTF_RenderText_Solid(font, str, strlen(str), *color);

    if (surface) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }

    if (!texture) {
        error("Error loading cpu state information\n");
        exit(EXIT_FAILURE);
    }

    d_rect->w = texture->w;
    d_rect->h = texture->h;

    SDL_RenderTexture(renderer, texture, NULL, d_rect);

    SDL_DestroyTexture(texture);
}

void render_cpu_state (sm83_ctx *cpu, uint8_t *memory, SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color color = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    SDL_FRect rect = {0};
    char str[40] = {0};
    int x = SCREEN_X + SCREEN_WIDTH + BORDER_WIDTH;
    int y = SCREEN_Y + BORDER_WIDTH;

    // Refactor this mess later

    rect.x = x;
    rect.y = y;

    snprintf(str, 39, "PC: 0x%04X", cpu->pc);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "SP: 0x%04X", cpu->sp);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h * 2;
    snprintf(str, 39, "OP: 0x%02X", memory[cpu->pc]);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "n8: %d", memory[cpu->pc + 1]);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "n16: %d (0x%04X)", bytes_to_u16(memory[cpu->pc + 1], memory[cpu->pc + 2]), bytes_to_u16(memory[cpu->pc + 1], memory[cpu->pc + 2]));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h * 2;
    snprintf(str, 39, "A: %d", cpu->rA);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "B: %d", cpu->rB);
    render_text(window, renderer, font, &color, &rect, str);
    
    rect.y += rect.h;
    snprintf(str, 39, "C: %d", cpu->rC);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "D: %d", cpu->rD);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "E: %d", cpu->rE);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "H: %d", cpu->rH);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "L: %d", cpu->rL);
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "AF: %d", bytes_to_u16(cpu->rF, cpu->rA));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "BC: %d", bytes_to_u16(cpu->rC, cpu->rB));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "DE: %d", bytes_to_u16(cpu->rE, cpu->rD));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "HL: %d (0x%04X)", bytes_to_u16(cpu->rL, cpu->rH), bytes_to_u16(cpu->rL, cpu->rH));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h * 2;
    snprintf(str, 39, "Z: %d", get_bit_u8(&cpu->rF, ZERO_FLAG));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "N: %d", get_bit_u8(&cpu->rF, SUBTRACTION_FLAG));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "H: %d", get_bit_u8(&cpu->rF, HALF_CARRY_FLAG));
    render_text(window, renderer, font, &color, &rect, str);

    rect.y += rect.h;
    snprintf(str, 39, "C: %d", get_bit_u8(&cpu->rF, CARRY_FLAG));
    render_text(window, renderer, font, &color, &rect, str);
}

int main (int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    TTF_Font *font;

    sm83_ctx cpu = {0};
    cartridge_header cart_h = {0};
    uint8_t rom_type = 0;
    uint8_t *memory = NULL;
    size_t rom_size = 0;
    FILE *file = NULL;

    if (argc == 1)
        print_usage(argv[0]);

    if ((rom_type = gb_rom_type(argv[1])) == 0)
        print_usage(argv[0]);

    if ((file = fopen(argv[1], "rb")) == NULL)
        error("Unable to open file provided\n");

    if (fseek(file, 0, SEEK_END) < 0 || (rom_size = ftell(file)) < 0)
        error("Error occured getting ROM size\n");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("Unable to initialize SDL\n");
    }

    if (TTF_Init() < 0) {
        error("Unable to initialize SDL_TTF\n");
    }

    SDL_CreateWindowAndRenderer(
        "GameBoy Emulator", WINDOW_SIZE, WINDOW_SIZE,
        SDL_WINDOW_MOUSE_GRABBED | SDL_WINDOW_KEYBOARD_GRABBED,
        &window, &renderer
    );

    font = TTF_OpenFont("./fonts/CourierPrime-Regular.ttf", 12);

    memory = (uint8_t *)malloc(rom_size);

    rewind(file);
    fread(memory, 1, rom_size, file);

    store_c_header_data(memory, &cart_h);

    cpu.sp = 0xFFFE;
    cpu.is_running = true;

    // Main Loop
    while (cpu.is_running) {
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    cpu.is_running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.scancode) {
                        case SDL_SCANCODE_ESCAPE:
                            cpu.is_running = false;
                            break;
                        case SDL_SCANCODE_SPACE:
                            next_instruction(&cpu, memory);
                            break;
                    }
                    break;
            }
        }

        render_screen(window, renderer);
        render_cpu_state(&cpu, memory, window, renderer, font);

        SDL_RenderPresent(renderer);
    }

    free(memory);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}