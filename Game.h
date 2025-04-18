#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <map>
#include <string>
#include <SDL_ttf.h>
#include <fstream>

using namespace std;

const int SCREEN_HEIGHT = 600;
const int GRID_ROWS = 4;
const int GRID_COLS = 4;
const int TILE_SIZE = 120;
const int GAME_DURATION = 60000;
const int SCREEN_WIDTH = GRID_COLS * TILE_SIZE;
const int FLIP_DELAY = 500;


enum GameState {
    MENU,
    PLAYING,
    GAME_OVER_WIN,
    GAME_OVER_LOSE
};

class Game {
private:

    SDL_Window* window;
    SDL_Renderer* renderer;


    map<int, SDL_Texture*> textures;
    SDL_Texture* menuTexture;
    SDL_Texture* winTexture;
    SDL_Texture* loseTexture;
    SDL_Texture* backTexture;
    SDL_Texture* startButtonTexture;
    SDL_Texture* restartButtonTexture;


    Mix_Music* backgroundMusic;


    GameState gameState;
    int flipCount;
    int score;
    int startTime;
    int flipBackTime;
    int highScore;



    vector<vector<int>> grid;
    vector<vector<bool>> revealed;
    vector<vector<bool>> matched;


    pair<int, int> firstClick;
    pair<int, int> secondClick;

public:
    Game();
    ~Game();


    bool initSDL();
    void loadResources();
    void initializeGrid();
    void closeSDL();

    void handleMouseClick(int x, int y);
    void updateGameState();
    bool checkRestartButtonClick(int x, int y);


    void renderGrid();
    void renderMenu();
    void renderResult(bool isWin);
    void renderScore();
    void renderTime();
    void renderRestartButton();
    void renderHighScore();


    void playBackgroundMusic();
    void stopBackgroundMusic();

    void loadHighScore();
    void saveHighScore();


    SDL_Texture* loadTexture(const string& path);


    GameState getGameState() const { return gameState; }
    SDL_Renderer* getRenderer() const { return renderer; }
};

#endif
