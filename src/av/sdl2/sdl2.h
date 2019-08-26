/********************************
** Tsunagari Tile Engine       **
** sdl2.h                      **
** Copyright 2019 Paul Merrill **
********************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_AV_SDL2_SDL2_H_
#define SRC_AV_SDL2_SDL2_H_

#include "util/int.h"
#include "util/noexcept.h"

extern "C" {

// SDL.h
int SDL_Init(uint32_t) noexcept;
uint32_t SDL_WasInit(uint32_t flags) noexcept;
#define SDL_INIT_AUDIO 0x00000010
#define SDL_INIT_VIDEO 0x00000020

// SDL_audio.h
#define AUDIO_S16LSB 0x8010

// SDL_blendmode.h
typedef enum {
    SDL_BLENDMODE_BLEND = 0x00000001,
} SDL_BlendMode;

// SDL_error.h
const char* SDL_GetError() noexcept;

// SDL_scancode.h
typedef enum {} SDL_Scancode;

// SDK_keycode.h
typedef int32_t SDL_Keycode;
enum {
    SDLK_ESCAPE = '\033',
    SDLK_SPACE = ' ',
    SDLK_RIGHT = (1 << 30) | 79,
    SDLK_LEFT = (1 << 30) | 80,
    SDLK_DOWN = (1 << 30) | 81,
    SDLK_UP = (1 << 30) | 82,
    SDLK_LCTRL = (1 << 30) | 224,
    SDLK_LSHIFT = (1 << 30) | 225,
    SDLK_RCTRL = (1 << 30) | 228,
    SDLK_RSHIFT = (1 << 30) | 229,
};

// SDL_keyboard.h
typedef struct {
    SDL_Scancode scancode;
    SDL_Keycode sym;
    uint16_t mod;
    uint32_t unused;
} SDL_Keysym;

// SDL_events.h
typedef enum {
    SDL_QUIT = 0x100,
    SDL_KEYDOWN = 0x300,
    SDL_KEYUP = 0x301,
} SDL_EventType;
typedef struct {
    uint32_t type, timestamp, windowID;
    uint8_t state, repeat, padding2, padding3;
    SDL_Keysym keysym;
} SDL_KeyboardEvent;
typedef union {
    uint32_t type;
    SDL_KeyboardEvent key;
    uint8_t padding[56];
} SDL_Event;
int SDL_PollEvent(SDL_Event*) noexcept;

// SDL_rect.h
typedef struct {
    int x, y, w, h;
} SDL_Rect;

// SDL_rwops.h
typedef struct SDL_RWops SDL_RWops;
SDL_RWops* SDL_RWFromMem(void*, int) noexcept;

// SDL_surface.h
typedef struct SDL_Surface SDL_Surface;
void SDL_FreeSurface(SDL_Surface*) noexcept;

// SDL_video.h
typedef struct SDL_Window SDL_Window;
typedef struct {
    uint32_t format;
    int w, h, refresh_rate;
    void* driverdata;
} SDL_DisplayMode;
SDL_Window* SDL_CreateWindow(const char*,
                             int,
                             int,
                             int,
                             int,
                             uint32_t) noexcept;
void SDL_DestroyWindow(SDL_Window*) noexcept;
int SDL_GetCurrentDisplayMode(int, SDL_DisplayMode*) noexcept;
int SDL_GetWindowDisplayIndex(SDL_Window*) noexcept;
void SDL_GetWindowSize(SDL_Window*, int*, int*) noexcept;
void SDL_SetWindowTitle(SDL_Window*, const char*) noexcept;
void SDL_ShowWindow(SDL_Window*) noexcept;
#define SDL_WINDOW_FULLSCREEN 0x00000001
#define SDL_WINDOW_HIDDEN 0x00000008
#define SDL_WINDOWPOS_UNDEFINED_MASK 0x1FFF0000u
#define SDL_WINDOWPOS_UNDEFINED_DISPLAY(X) (SDL_WINDOWPOS_UNDEFINED_MASK | (X))
#define SDL_WINDOWPOS_UNDEFINED SDL_WINDOWPOS_UNDEFINED_DISPLAY(0)

// SDL_render.h
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_RendererInfo {
    const char* name;
    uint32_t flags;
    uint32_t num_texture_formats;
    uint32_t texture_formats[16];
    int max_texture_width;
    int max_texture_height;
} SDL_RendererInfo;
SDL_Renderer* SDL_CreateRenderer(SDL_Window*, int, uint32_t) noexcept;
SDL_Texture* SDL_CreateTextureFromSurface(SDL_Renderer*, SDL_Surface*) noexcept;
void SDL_DestroyTexture(SDL_Texture*) noexcept;
int SDL_GetRendererInfo(SDL_Renderer*, SDL_RendererInfo*) noexcept;
int SDL_QueryTexture(SDL_Texture*, uint32_t*, int*, int*, int*) noexcept;
int SDL_RenderClear(SDL_Renderer*) noexcept;
int SDL_RenderCopy(SDL_Renderer*,
                   SDL_Texture*,
                   const SDL_Rect*,
                   const SDL_Rect*) noexcept;
int SDL_RenderFillRect(SDL_Renderer*, const SDL_Rect*) noexcept;
void SDL_RenderPresent(SDL_Renderer*) noexcept;
int SDL_SetRenderDrawBlendMode(SDL_Renderer*, SDL_BlendMode) noexcept;
int SDL_SetRenderDrawColor(SDL_Renderer*,
                           uint8_t,
                           uint8_t,
                           uint8_t,
                           uint8_t) noexcept;
#define SDL_RENDERER_ACCELERATED 0x00000002
#define SDL_RENDERER_PRESENTVSYNC 0x00000004

// SDL_image library
// SDL_image.h
SDL_Surface* IMG_Load_RW(SDL_RWops*, int) noexcept;

// SDL_mixer library
// SDL_mixer.h
typedef struct Mix_Chunk Mix_Chunk;
typedef struct Mix_Music Mix_Music;
int Mix_AllocateChannels(int) noexcept;
void Mix_ChannelFinished(void (*)(int));
void Mix_FreeChunk(Mix_Chunk*) noexcept;
void Mix_FreeMusic(Mix_Music*) noexcept;
int Mix_HaltChannel(int) noexcept;
int Mix_HaltMusic() noexcept;
Mix_Music* Mix_LoadMUS_RW(SDL_RWops*, int) noexcept;
Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops*, int) noexcept;
int Mix_OpenAudio(int, uint16_t, int, int) noexcept;
int Mix_PausedMusic() noexcept;
void Mix_Pause(int) noexcept;
void Mix_PauseMusic() noexcept;
int Mix_PlayingMusic() noexcept;
int Mix_PlayChannelTimed(int, Mix_Chunk*, int, int) noexcept;
int Mix_PlayMusic(Mix_Music*, int) noexcept;
void Mix_Resume(int) noexcept;
void Mix_ResumeMusic() noexcept;
int Mix_SetPosition(int, int16_t, uint8_t) noexcept;
int Mix_Volume(int, int) noexcept;
int Mix_VolumeMusic(int) noexcept;
#define MIX_DEFAULT_FORMAT AUDIO_S16LSB
#define Mix_PlayChannel(channel, chunk, loops) \
    Mix_PlayChannelTimed(channel, chunk, loops, -1)

}  // extern "C"

#endif  // SRC_AV_SDL2_SDL2_H_
