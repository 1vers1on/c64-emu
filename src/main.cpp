// #include <SDL2/SDL_timer.h>
// #ifndef __EMSCRIPTEN__
#if !defined(__EMSCRIPTEN__) && !defined(TEST_6502)
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/types.h>
// #include <cpu.hpp>
#include <system.hpp>
// #include <SDL2/SDL.h>
#include <cctype>

void write_bmp(const std::array<uint32_t, 40 * 25 * 8 * 8>& screen,
               const std::string& filename = "output.bmp") {
    const int width = 40 * 8;    // 320 pixels
    const int height = 25 * 8;   // 200 pixels
    const int bytesPerPixel = 4; // 32-bit
    const int rowSize = width * bytesPerPixel;
    const int imageSize = rowSize * height;
    const int fileHeaderSize = 14;
    const int infoHeaderSize = 40;
    const int fileSize = fileHeaderSize + infoHeaderSize + imageSize;

    std::ofstream ofs(filename, std::ios::binary);
    if(!ofs) {
        throw std::runtime_error("failed to open file");
    }

    // bmp file header (14 bytes)
    unsigned char fileHeader[fileHeaderSize] = {
        'B', 'M',       // signature
        0,   0,   0, 0, // file size (will fill in below)
        0,   0,         // reserved
        0,   0,         // reserved
        0,   0,   0, 0  // pixel data offset (will fill in below)
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
    infoHeader[8] = static_cast<unsigned char>(height);
    infoHeader[9] = static_cast<unsigned char>(height >> 8);
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
    for(int y = height - 1; y >= 0; --y) {
        for(int x = 0; x < width; ++x) {
            // assuming 'screen' is stored in row-major order (top-down)
            int index = y * width + x;
            uint32_t pixel = screen[index];
            // bmp expects pixels in bgra order (little-endian)
            unsigned char color[4];
            color[0] = static_cast<unsigned char>(pixel & 0xFF);         // blue
            color[1] = static_cast<unsigned char>((pixel >> 8) & 0xFF);  // green
            color[2] = static_cast<unsigned char>((pixel >> 16) & 0xFF); // red
            color[3] = static_cast<unsigned char>((pixel >> 24) & 0xFF); // alpha
            ofs.write(reinterpret_cast<char*>(color), 4);
        }
    }
    ofs.close();
}

// void render_screen(SDL_Texture* texture, SDL_Renderer* renderer, const std::array<uint32_t, 40 *
// 25 * 8 * 8>& screen) {
//     SDL_UpdateTexture(texture, nullptr, screen.data(), 320 * sizeof(uint32_t));
//     SDL_RenderClear(renderer);
//     SDL_RenderCopy(renderer, texture, nullptr, nullptr);
//     SDL_RenderPresent(renderer);
// }

#define SAMPLE_RATE 44100 // standard audio sample rate
#define BUFFER_SIZE 512   // number of samples per callback

// // audio callback function
// void audioCallback(void* userdata, Uint8* stream, int len) {
//     System* sys = static_cast<System*>(userdata);
//     int16_t* buffer = (int16_t*)stream;  // sdl expects raw pcm data
//     int samples = len / sizeof(int16_t);

//     for (int i = 0; i < samples; i++) {
//         float sample = sys->sid->tick();
//         buffer[i] = static_cast<int16_t>(sample * 32767.0f);  // convert to 16-bit signed

//         // buffer[i] = sys->sid->tick();
//         // instead test this with random
//     }
// }

// void initAudio(System& system) {
//     SDL_AudioSpec desiredSpec = {};
//     desiredSpec.freq = SAMPLE_RATE;
//     desiredSpec.format = AUDIO_S16SYS;  // 16-bit signed audio
//     desiredSpec.channels = 1;  // mono output
//     desiredSpec.callback = audioCallback;
//     desiredSpec.userdata = &system;
//     desiredSpec.callback = audioCallback;

//     if (SDL_OpenAudio(&desiredSpec, NULL) < 0) {
//         std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
//         exit(1);
//     }

//     SDL_PauseAudio(0);  // start playback
// }

int main() {
    // if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    //     std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    //     return 1;
    // }

    // SDL_Window *window = SDL_CreateWindow("Framebuffer Example",
    //     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    //     320 * 2, 200 * 2, SDL_WINDOW_SHOWN);
    // if (!window) {
    //     std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    //     SDL_Quit();
    //     return 1;
    // }

    // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
    //                                          SDL_TEXTUREACCESS_STREAMING, 320, 200);

    bool running = true;
    // SDL_Event event;
    System system;
    // initAudio(system);
    // render_screen(texture, renderer, cpu.bus->vic->screen);
    // system.loadRoms("c64.kernal.bin", "c64.chrom.bin");

    // cpu.bus->loadC64rom("c64.kernal.bin");
    // cpu.bus->loadCharacterRom("c64.chrom.bin");
    int i = 0;
    system.vic->setFramebufferCallback([&i](std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
        write_bmp(screen, "../output/" + std::to_string(i % 2) + ".bmp");
        i++;
    });

    // system.bus->loadCartridge("DEFENDER.CRT");

    system.powerOn();

    while(running) {
        system.step();

        // if (system.vic->needsRender) {
        //     while (SDL_PollEvent(&event)) {
        //         if (event.type == SDL_QUIT) {
        //             running = false;
        //         }
        //         if (event.type == SDL_KEYDOWN) {
        //             // check if its a grave key
        //             if (event.key.keysym.sym == SDLK_BACKQUOTE) {
        //                 for (int i = 0; i < 10; i++) {
        //                     system.sid->write(rand() % 0x1FFF, rand() % 0xFF);
        //                 }
        //             }
        //             // the key needs to be all uppercase for the input to work
        //             std::string key = SDL_GetKeyName(event.key.keysym.sym);
        //             std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        //             system.input->setKeyPressed(key, true);
        //         }
        //         if (event.type == SDL_KEYUP) {
        //             std::string key = SDL_GetKeyName(event.key.keysym.sym);
        //             std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        //             system.input->setKeyPressed(key, false);
        //         }
        //     }
        //     render_screen(texture, renderer, system.vic->screen);
        //     system.vic->needsRender = false;
        //     SDL_Delay(20); // if you dont do this your computer will explode
        //     // std::cout << cpu.dump() << std::endl;
        // }
    }
    // SDL_DestroyTexture(texture);
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();
    return 0;
}
#endif

// #include <iostream>
// #include <vector>
// #include <cstring>
// #include <algorithm>
// #include <cstdint>

// #include <cstdint>
// #include <cstring>
// #include <vector>
// #include <string>
// #include <stdexcept>

// namespace lz4 {

// constexpr int HASH_TABLE_SIZE = 1 << 16;
// constexpr int MIN_MATCH = 4;
// constexpr int MAX_MATCH = 0x7F + MIN_MATCH;
// constexpr int MAX_DISTANCE = 0xFFFF;

// inline uint32_t hash(const uint8_t* data) {
//     // Handle unaligned memory access more safely
//     uint32_t val;
//     std::memcpy(&val, data, sizeof(uint32_t));
//     return (val * 2654435761U) >> 16;
// }

// std::vector<uint8_t> compress(const uint8_t* input, size_t input_size) {
//     std::vector<uint8_t> output;
//     output.reserve(input_size * 1.1);

//     std::vector<uint32_t> hash_table(HASH_TABLE_SIZE, 0);

//     size_t ip = 0;

//     while (ip < input_size) {
//         size_t token_pos = output.size();
//         output.push_back(0);

//         size_t literal_start = ip;

//         size_t matched = 0;
//         while (ip + MIN_MATCH <= input_size && !matched) {
//             uint32_t h = hash(&input[ip]);
//             size_t ref = hash_table[h];
//             hash_table[h] = ip;

//             size_t distance = (ref > 0) ? ip - ref : 0;

//             if (distance > 0 && distance < MAX_DISTANCE &&
//                 memcmp(&input[ref], &input[ip], MIN_MATCH) == 0) {

//                 size_t match_len = MIN_MATCH;
//                 while (match_len < MAX_MATCH &&
//                        ip + match_len < input_size &&
//                        input[ref + match_len] == input[ip + match_len]) {
//                     match_len++;
//                 }

//                 size_t literal_len = ip - literal_start;
//                 if (literal_len < 15) {
//                     output[token_pos] = literal_len << 4;
//                 } else {
//                     output[token_pos] = 15 << 4;
//                     size_t remain = literal_len - 15;
//                     while (remain >= 255) {
//                         output.push_back(255);
//                         remain -= 255;
//                     }
//                     output.push_back(static_cast<uint8_t>(remain));
//                 }

//                 output.insert(output.end(), &input[literal_start], &input[literal_start +
//                 literal_len]);

//                 if (match_len < 15 + MIN_MATCH) {
//                     output[token_pos] |= (match_len - MIN_MATCH);
//                 } else {
//                     output[token_pos] |= 15;
//                     size_t remain = match_len - MIN_MATCH - 15;
//                     while (remain >= 255) {
//                         output.push_back(255);
//                         remain -= 255;
//                     }
//                     output.push_back(static_cast<uint8_t>(remain));
//                 }

//                 output.push_back(distance & 0xFF);
//                 output.push_back((distance >> 8) & 0xFF);

//                 ip += match_len;
//                 matched = 1;
//             } else {
//                 ip++;
//             }
//         }

//         if (!matched) {
//             size_t literal_len = input_size - literal_start;

//             if (literal_len < 15) {
//                 output[token_pos] = literal_len << 4;
//             } else {
//                 output[token_pos] = 15 << 4;
//                 size_t remain = literal_len - 15;
//                 while (remain >= 255) {
//                     output.push_back(255);
//                     remain -= 255;
//                 }
//                 output.push_back(static_cast<uint8_t>(remain));
//             }

//             output.insert(output.end(), &input[literal_start], &input[literal_start +
//             literal_len]); ip = input_size;
//         }
//     }

//     return output;
// }

// std::vector<uint8_t> decompress(const uint8_t* input, size_t input_size, size_t max_output_size)
// {
//     std::vector<uint8_t> output;
//     output.reserve(max_output_size);

//     size_t ip = 0;

//     while (ip < input_size) {
//         if (ip >= input_size) break;
//         uint8_t token = input[ip++];

//         size_t literal_len = token >> 4;
//         if (literal_len == 15) {
//             uint8_t extra_len;
//             do {
//                 if (ip >= input_size)
//                     throw std::runtime_error("Unexpected end of input");

//                 extra_len = input[ip++];
//                 literal_len += extra_len;
//             } while (extra_len == 255 && ip < input_size);
//         }

//         if (ip + literal_len > input_size) {
//             throw std::runtime_error("Literal length exceeds input buffer");
//         }

//         output.insert(output.end(), &input[ip], &input[ip + literal_len]);
//         ip += literal_len;

//         if (ip >= input_size) break;

//         if (ip + 2 > input_size) {
//             throw std::runtime_error("Unexpected end of input");
//         }

//         uint16_t offset = input[ip] | (input[ip + 1] << 8);
//         ip += 2;

//         if (offset == 0 || offset > output.size()) {
//             throw std::runtime_error("Invalid match offset: " + std::to_string(offset) +
//                                      ", output size: " + std::to_string(output.size()));
//         }

//         size_t match_len = (token & 0x0F) + MIN_MATCH;
//         if (match_len == 15 + MIN_MATCH) {
//             uint8_t extra_len;
//             do {
//                 if (ip >= input_size)
//                     throw std::runtime_error("Unexpected end of input");

//                 extra_len = input[ip++];
//                 match_len += extra_len;
//             } while (extra_len == 255 && ip < input_size);
//         }

//         size_t match_pos = output.size() - offset;
//         if (output.size() + match_len > max_output_size) {
//             throw std::runtime_error("Decompressed data exceeds max output size");
//         }

//         for (size_t i = 0; i < match_len; i++) {
//             output.push_back(output[match_pos + i]);
//         }
//     }

//     return output;
// }

// std::vector<uint8_t> compress(const std::vector<uint8_t>& input) {
//     return compress(input.data(), input.size());
// }

// std::vector<uint8_t> decompress(const std::vector<uint8_t>& input, size_t max_output_size) {
//     return decompress(input.data(), input.size(), max_output_size);
// }

// std::vector<uint8_t> compress(const std::string& input) {
//     return compress((const uint8_t*)input.data(), input.size());
// }

// std::string decompress_to_string(const std::vector<uint8_t>& input, size_t max_output_size) {
//     auto decompressed = decompress(input, max_output_size);
//     return std::string(decompressed.begin(), decompressed.end());
// }

// }

// #include <chrono>

// int main() {
//     std::string message = "This is a test message for LZ4 compression. "
//                          "It contains repeated patterns like this this this to demonstrate
//                          compression.";

//     auto compressed = lz4::compress(message);
//     std::cout << "Compressed size: " << compressed.size() << " bytes\n";

//     auto decompressed = lz4::decompress_to_string(compressed, message.size());
//     std::cout << "Decompressed message: " << decompressed << "\n";
//     std::cout << "Decompressed size: " << decompressed.size() << " bytes\n";
//     std::cout << "Original message: " << message << "\n";
//     std::cout << "Original size: " << message.size() << " bytes\n";
//     if (message == decompressed) {
//         std::cout << "Decompression successful!\n";
//     } else {
//         std::cout << "Decompression failed!\n";
//     }

//     // now test performance
//     auto start = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < 100000; i++) {
//         auto compressed = lz4::compress(message);
//         auto decompressed = lz4::decompress_to_string(compressed, message.size());
//         if (message != decompressed) {
//             std::cout << "Decompression failed!\n";
//             return 1;
//         }
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> elapsed = end - start;
//     std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";
//     std::cout << "Compression speed: " << (100000.0 / elapsed.count()) << " compressions per
//     second\n";

//     return 0;
// }
