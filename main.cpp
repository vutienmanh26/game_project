#include "Game.h"
#include <SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    Game game;

    if (!game.initSDL()) {
        std::cerr << "Failed to initialize SDL!" << std::endl;
        return -1;
    }
    game.loadResources();

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                game.handleMouseClick(x, y);
            }
        }

        SDL_SetRenderDrawColor(game.getRenderer(), 255, 255, 255, 255);
        SDL_RenderClear(game.getRenderer());

        game.updateGameState();

        switch (game.getGameState()) {
            case MENU:
                game.renderMenu();
                break;
            case PLAYING:
                game.renderGrid();
                game.renderScore();
                game.renderTime();
                break;
            case GAME_OVER_WIN:
                game.renderResult(true);
                break;
            case GAME_OVER_LOSE:
                game.renderResult(false);
                break;
        }


        SDL_RenderPresent(game.getRenderer());

        SDL_Delay(16);
    }

    return 0;
}
