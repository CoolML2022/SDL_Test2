//
//  main.cpp
//  SDL_Test2
//
//  Created by Vincent Krebs on 18.12.24.
//
#include <iostream>
#include <stdio.h>
#include <SDL2/SDL.h>
// Wrapper für das Fenster (automatische Auflösung, beim verlassen der Main Methode)
class SDLWindow {
public:
    SDLWindow(const char* title, int width, int height)
        : window(nullptr), renderer(nullptr) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return;
        }

        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
    }

    ~SDLWindow() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void clear() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }

    void present() {
        SDL_RenderPresent(renderer);
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};

int main(int argc, const char *argv[]) {
    SDLWindow window("Test", 900, 600);
    SDL_Event event;
    bool quit = false;
    
    while(!quit) {
        // events handeling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        // rendering
        
        // update
    }
    printf("Hello, SDL!");
    return 0;
}

