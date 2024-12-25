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
#include <thread>
// Wrapper für das Fenster (automatische Auflösung, beim verlassen der Main Methode)

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int CELL_SIZE = 20;
const int ROWS = WINDOW_HEIGHT / CELL_SIZE;
const int COLUMNS = WINDOW_WIDTH / CELL_SIZE;
const SDL_Color SandColor = {255, 204, 98, 100};

struct Cell {
    bool isFilled;
    float x_velocity;
    float y_velocity;
    SDL_Color color;
};

void SDL_DrawGrid(SDL_Renderer *renderer) {
    for (int i = 0; i< COLUMNS; i++) {
        SDL_RenderDrawLine(renderer, i * CELL_SIZE, 0, i * CELL_SIZE, WINDOW_HEIGHT);
    }
    for (int i = 0; i < ROWS; i++) {
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, WINDOW_WIDTH, i * CELL_SIZE);
    }
    
}
void DrawCells(SDL_Renderer *renderer, Cell *cells) {
    std::vector<SDL_Rect> rectangles;
    for (int i = 0; i < COLUMNS; i++) {
        for (int j = 0; j < ROWS; j++) {
            int index = j * COLUMNS + i;
            if (cells[index].isFilled) {
                SDL_Rect rect = {i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                rectangles.push_back(rect);
            }
        }
    }
    SDL_SetRenderDrawColor(renderer, 255, 204, 98, 100);
    SDL_RenderFillRects(renderer, rectangles.data(), static_cast<int>(rectangles.size()));
}

const int LUT_SIZE = 2;  // Größe des Lookup Tables
const int randomSigns[LUT_SIZE] = {-1, 1};
int getRandomSign() {
    int index = std::rand() % LUT_SIZE;
    return randomSigns[index];
}
void UpdateCells(Cell* cells,Cell* nextCells, int start_i, int end_i) {
    for (int i = start_i; i < end_i; i++) {
        for (int j = 0; j < ROWS; j++) {
            // Berechnung des Indexes im eindimensionalen Array
            int index = j * COLUMNS + i;
            Cell* currentCell = &cells[index];
            
            // Skipping this cell, if there is nothing inside
            if (!cells[index].isFilled) {
                continue;
            }
            //std::cout << "cell: " << i << "," << j << " not empty!" << std::endl;
            // Checking if it is touching the floor
            const int lastRow = ROWS - 1;
            if (j == lastRow) {
                nextCells[index] = *currentCell;
                continue;
            }
            
            int downIndex = (j + 1) * COLUMNS + i;  // Index der Zelle direkt unter der aktuellen Zelle
            
            if (!cells[downIndex].isFilled) {
                nextCells[downIndex] = *currentCell;
            } else {
                int di = 0;  // Horizontaler Offset
                if (i > 0 && !cells[(j + 1) * COLUMNS + (i - 1)].isFilled) {
                    di = -1;  // Bewegung nach unten links
                    //std::cout << "Down Left i: " << i << " start: " << start_i << " end: " << end_i << std::endl;
                }
                if (i < COLUMNS - 1 && !cells[(j + 1) * COLUMNS + (i + 1)].isFilled) {
                    if (di == -1) {
                        di = getRandomSign();  // Zufällige Bewegung, wenn beide Richtungen frei sind
                    } else {
                        di = 1;  // Bewegung nach unten rechts
                    }
                }
                if (di != 0) {
                    nextCells[(j + 1) * COLUMNS + (i + di)] = *currentCell;  // Bewegung der Zelle
                } else {
                    nextCells[index] = *currentCell;  // Zelle bleibt an Ort und Stelle
                }
            }
        }
    }
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
    Cell cells[COLUMNS * ROWS] = {false, 0, 0, SDL_Color{0, 0, 0, 0} };
    Cell nextCells[COLUMNS * ROWS] = {false, 0, 0, SDL_Color{0, 0, 0, 0} };
    int numOfThreads = 4; // Anzahl der Threads
    std::vector<std::thread> threads;  // Vektor für die Threads
    bool mouseButtonPressed = false;
    bool quit = false;
    SDL_Event event;
    int frameCount = 0;
    Uint32 last_tick_time = 0;
    Uint32 delta = 0;
    Uint32 lastTime = SDL_GetTicks();
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
            if ((event.type == SDL_MOUSEMOTION && event.motion.state != 0) || mouseButtonPressed) {
                int cell_x = event.motion.x / CELL_SIZE;
                int cell_y = event.motion.y / CELL_SIZE;
                int range = 0; // Dies bestimmt, wie viele Zellen um die Maus herum ausgefüllt werden
                // Schleife über die Zellen rund um die Maus
                for (int i = -range; i <= range; ++i) {
                    for (int j = -range; j <= range; ++j) {
                        // Berechne die tatsächliche Zelle, aber achte auf die Grenzen des Arrays
                        int target_x = cell_x + i;
                        int target_y = cell_y + j;
                        
                        // Überprüfe, ob die Zelle innerhalb des Arrays liegt
                        if (target_x >= 0 && target_x < COLUMNS && target_y >= 0 && target_y < ROWS && getRandomSign() < 0) {
                            int index = target_y * COLUMNS + target_x;
                            cells[index] = {true, 0, 1, SandColor};  // Setze das Zellenfeld auf 1
                        }
                    }
                }
            }
        }
        
        // Delta Time
        Uint32 tick_time = SDL_GetTicks();
        delta = tick_time - last_tick_time;
        last_tick_time = tick_time;
        //std::cout << "Delta-time: " << delta << std::endl;
        
        //FrameRate
        frameCount++;
        if (tick_time - lastTime >= 1000) {  // Alle 1000 ms (1 Sekunde)
            float fps = frameCount / ((tick_time - lastTime) / 1000.0f);
            std::cout << "  FPS: " << fps << std::endl;
            frameCount = 0;
            lastTime = tick_time;
        }
        
        
        
        // rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        DrawCells(renderer, cells);
        SDL_RenderPresent(renderer);
        
        // update (multithreaded)
        for (int t = 0; t < delta/8; t++) {
            threads.clear();
            if (numOfThreads > 1) {
                int columnsPerThread = COLUMNS / numOfThreads;
                threads.emplace_back(UpdateCells, std::ref(cells), std::ref(nextCells), 0, columnsPerThread);
                
                for (int i = 1; i < numOfThreads; i++) {
                    threads.emplace_back(UpdateCells, std::ref(cells), std::ref(nextCells), i * (columnsPerThread), i * columnsPerThread + columnsPerThread);
                }
            } else if (numOfThreads == 1){
                threads.emplace_back(UpdateCells, std::ref(cells), std::ref(nextCells), 0, COLUMNS);
            } else {
                // Falls kein Multithreading genutzt wird
                UpdateCells(std::ref(cells), std::ref(nextCells), 0, COLUMNS);
            }
            
            
            
            for (auto& t : threads) {
                t.join();
            }
            //UpdateCells(std::ref(cells), std::ref(nextCells), 0, COLUMNS);
            std::copy(nextCells, nextCells + COLUMNS * ROWS, cells);
            std::memset(&nextCells[0], 0, sizeof(nextCells));
        }
    }
    return 0;
}
