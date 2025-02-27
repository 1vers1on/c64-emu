#include <SDL2/SDL_timer.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cpu.hpp>
#include <SDL2/SDL.h>
#include <algorithm>
#include <cctype>

void write_bmp(const std::array<uint32_t, 40 * 25 * 8 * 8>& screen, const std::string& filename = "output.bmp") {
    const int width = 40 * 8;   // 320 pixels
    const int height = 25 * 8;  // 200 pixels
    const int bytesPerPixel = 4; // 32-bit
    const int rowSize = width * bytesPerPixel;
    const int imageSize = rowSize * height;
    const int fileHeaderSize = 14;
    const int infoHeaderSize = 40;
    const int fileSize = fileHeaderSize + infoHeaderSize + imageSize;

    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("failed to open file");
    }

    // bmp file header (14 bytes)
    unsigned char fileHeader[fileHeaderSize] = {
        'B', 'M',          // signature
        0, 0, 0, 0,        // file size (will fill in below)
        0, 0,              // reserved
        0, 0,              // reserved
        0, 0, 0, 0         // pixel data offset (will fill in below)
    };

    // fill file size (little-endian)
    fileHeader[2] = static_cast<unsigned char>(fileSize);
    fileHeader[3] = static_cast<unsigned char>(fileSize >> 8);
    fileHeader[4] = static_cast<unsigned char>(fileSize >> 16);
    fileHeader[5] = static_cast<unsigned char>(fileSize >> 24);

    const int pixelDataOffset = fileHeaderSize + infoHeaderSize;
    fileHeader[10] = static_cast<unsigned char>(pixelDataOffset);
    fileHeader[11] = static_cast<unsigned char>(pixelDataOffset >> 8);
    fileHeader[12] = static_cast<unsigned char>(pixelDataOffset >> 16);
    fileHeader[13] = static_cast<unsigned char>(pixelDataOffset >> 24);

    ofs.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);

    // bmp info header (BITMAPINFOHEADER - 40 bytes)
    unsigned char infoHeader[infoHeaderSize] = {0};

    // header size
    infoHeader[0] = static_cast<unsigned char>(infoHeaderSize);
    infoHeader[1] = static_cast<unsigned char>(infoHeaderSize >> 8);
    infoHeader[2] = static_cast<unsigned char>(infoHeaderSize >> 16);
    infoHeader[3] = static_cast<unsigned char>(infoHeaderSize >> 24);

    // image width
    infoHeader[4] = static_cast<unsigned char>(width);
    infoHeader[5] = static_cast<unsigned char>(width >> 8);
    infoHeader[6] = static_cast<unsigned char>(width >> 16);
    infoHeader[7] = static_cast<unsigned char>(width >> 24);

    // image height
    infoHeader[8]  = static_cast<unsigned char>(height);
    infoHeader[9]  = static_cast<unsigned char>(height >> 8);
    infoHeader[10] = static_cast<unsigned char>(height >> 16);
    infoHeader[11] = static_cast<unsigned char>(height >> 24);

    // planes (must be 1)
    infoHeader[12] = 1;
    infoHeader[13] = 0;

    // bits per pixel (32 for 32-bit)
    infoHeader[14] = 32;
    infoHeader[15] = 0;

    // compression (0 = BI_RGB, no compression)
    infoHeader[16] = infoHeader[17] = infoHeader[18] = infoHeader[19] = 0;

    // image size (can be 0 for BI_RGB, but here we set it)
    infoHeader[20] = static_cast<unsigned char>(imageSize);
    infoHeader[21] = static_cast<unsigned char>(imageSize >> 8);
    infoHeader[22] = static_cast<unsigned char>(imageSize >> 16);
    infoHeader[23] = static_cast<unsigned char>(imageSize >> 24);

    // the rest (resolution and color info) can remain 0
    ofs.write(reinterpret_cast<char*>(infoHeader), infoHeaderSize);

    // write pixel data; bmp stores rows bottom-up
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            // assuming 'screen' is stored in row-major order (top-down)
            int index = y * width + x;
            uint32_t pixel = screen[index];
            // bmp expects pixels in bgra order (little-endian)
            unsigned char color[4];
            color[0] = static_cast<unsigned char>(pixel & 0xFF);          // blue
            color[1] = static_cast<unsigned char>((pixel >> 8) & 0xFF);     // green
            color[2] = static_cast<unsigned char>((pixel >> 16) & 0xFF);    // red
            color[3] = static_cast<unsigned char>((pixel >> 24) & 0xFF);    // alpha
            ofs.write(reinterpret_cast<char*>(color), 4);
        }
    }
    ofs.close();
}

void render_screen(SDL_Texture* texture, SDL_Renderer* renderer, const std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
    SDL_UpdateTexture(texture, nullptr, screen.data(), 320 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow("Framebuffer Example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        320 * 2, 200 * 2, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, 320, 200);

    bool running = true;
    SDL_Event event;
    CPU cpu;
    render_screen(texture, renderer, cpu.bus->vic->screen);

    cpu.bus->loadC64rom("c64.kernel.bin");
    cpu.bus->loadCharacterRom("c64.chrom.bin");
    int i = 0;
    // cpu.bus->vic->setFramebufferCallback([&i](std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
    //     write_bmp(screen, "output/" + std::to_string(i % 2) + ".bmp");
    //     i++;
    // });
    cpu.powerOn();

    while (running) {
        cpu.executeOnce();
        if (cpu.bus->vic->needsRender) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN) {
                    // the key needs to be all uppercase for the input to work
                    std::string key = SDL_GetKeyName(event.key.keysym.sym);
                    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
                    cpu.bus->input->setKeyPressed(key, true);
                }
                if (event.type == SDL_KEYUP) {
                    std::string key = SDL_GetKeyName(event.key.keysym.sym);
                    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
                    cpu.bus->input->setKeyPressed(key, false);
                }
            }
            render_screen(texture, renderer, cpu.bus->vic->screen);
            cpu.bus->vic->needsRender = false;
            SDL_Delay(20); // if you dont do this your computer will explode
            // std::cout << cpu.dump() << std::endl;
        }
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
