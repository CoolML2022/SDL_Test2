//
//  main.cpp
//  SDL_Test2
//
//  Created by Vincent Krebs on 18.12.24.
//
#include <iostream>
#include <stdio.h>
#include <random>
#include <SDL2/SDL.h>
#include <time.h>
#include <unordered_map>
#include <cstring>
// Wrapper für das Fenster (automatische Auflösung, beim verlassen der Main Methode)

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int CELL_SIZE = 20;
const int ROWS = WINDOW_HEIGHT / CELL_SIZE;
const int COLUMNS = WINDOW_WIDTH / CELL_SIZE;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1,T2>& p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2; // XOR-Kombination der Hashes
    }
};

void SDL_DrawGrid(SDL_Renderer *renderer) {
    for (int i = 0; i< COLUMNS; i++) {
        SDL_RenderDrawLine(renderer, i * CELL_SIZE, 0, i * CELL_SIZE, WINDOW_HEIGHT);
    }
    for (int i = 0; i < ROWS; i++) {
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, WINDOW_WIDTH, i * CELL_SIZE);
    }
    
}
void DrawCells(SDL_Renderer *renderer, int (*cells)[ROWS] ) {
    for (int i = 0; i < COLUMNS; i++) {
        for (int j = 0; j < ROWS; j++) {
            if (cells[i][j] != 0) {
                //SDL_DrawRect(renderer, i * CELL_SIZE, j * CELL_SIZE);
                //std::cout << "rendered cell, at: " << i << j << std::endl;
                SDL_Rect rect = {i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_SetRenderDrawColor(renderer, 255, 242, 172, 100);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}
int getRandomSign() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    int randomValue = dis(gen);
    return (randomValue == 0) ? -1 : 1;
}

void UpdateCells(int (*cells)[ROWS]) {
    int nextCells[COLUMNS][ROWS] = { 0 };
    for (int i = 0; i < COLUMNS - 1; i++) {
        for (int j = 0; j < ROWS; j++) {
            if(cells[i][j] == 0) {
                continue;
            }
            if (j < ROWS - 1 && cells[i][j+1] == 0) {
                nextCells[i][j+1] = 1;
                cells[i][j] = 0;
            } else {
                bool isBelowLeftFree = (j < ROWS - 1 && i > 0) && cells[i - 1][j + 1] == 0;
                bool isBelowRightFree = (j < ROWS - 1 && i < COLUMNS - 2) && cells[i + 1][j + 1] == 0;
                
                if(isBelowLeftFree && isBelowRightFree) {
                    nextCells[i + getRandomSign()][j +1] = 1;
                    cells[i][j] = 0;
                } else if (isBelowLeftFree) {
                    nextCells[i - 1][j +1] = 1;
                    cells[i][j] = 0;
                } else if (isBelowRightFree) {
                    nextCells[i + 1][j +1] = 1;
                    cells[i][j] = 0;
                }
            }
        }
    }
    std::memcpy(cells, nextCells, sizeof(nextCells));
}


int main(int argc, const char *argv[]) {
    // SDL initialisieren
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // SDL-Fenster erstellen
    SDL_Window* window = SDL_CreateWindow("SDL2 Beispiel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // SDL-Renderer erstellen
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Setting all Cells empty
    int cells[COLUMNS][ROWS] = { 0 };
    bool mouseButtonPressed = false;
    bool quit = false;
    bool stepTriggered = false;
    SDL_Event event;
    
    while(!quit) {
        // events handeling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                mouseButtonPressed = true;
            }
            // Maustaste losgelassen
            if (event.type == SDL_MOUSEBUTTONUP) {
                mouseButtonPressed = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE && !stepTriggered) {
                    stepTriggered = true; // Nur ein Schritt pro Tastendruck
                }
                if (event.key.keysym.sym == SDLK_q) {
                    quit = true; // Programm beenden
                }
            }
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    stepTriggered = false; // Zurücksetzen, wenn die Taste losgelassen wird
                }
            }
            if ((event.type == SDL_MOUSEMOTION && event.motion.state != 0) || mouseButtonPressed) {
                int cell_x = event.motion.x / CELL_SIZE;
                int cell_y = event.motion.y / CELL_SIZE;
                int range = 1; // Dies bestimmt, wie viele Zellen um die Maus herum ausgefüllt werden
                // Schleife über die Zellen rund um die Maus
                for (int i = -range; i <= range; ++i) {
                    for (int j = -range; j <= range; ++j) {
                        // Berechne die tatsächliche Zelle, aber achte auf die Grenzen des Arrays
                        int target_x = cell_x + i;
                        int target_y = cell_y + j;
                        
                        // Überprüfe, ob die Zelle innerhalb des Arrays liegt
                        if (target_x >= 0 && target_x < COLUMNS && target_y >= 0 && target_y < ROWS) {
                            cells[target_x][target_y] = 1;  // Setze das Zellenfeld auf 1
                        }
                    }
                }
                
            }
        }
        
        UpdateCells(cells);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        DrawCells(renderer, cells);
        SDL_RenderPresent(renderer);
        SDL_Delay(32);
        
        //SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        //SDL_DrawGrid(renderer);
        
        
        // rendering
        
        // update
        
    }
    return 0;
}

