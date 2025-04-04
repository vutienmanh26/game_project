#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <map>
#include <string>
#include <SDL_ttf.h>

const int SCREEN_HEIGHT = 600;
const int GRID_ROWS = 4;
const int GRID_COLS = 4;
const int TILE_SIZE = 120;
const int GAME_DURATION = 60000;
const int SCREEN_WIDTH = GRID_COLS * TILE_SIZE;
const int FLIP_DELAY = 500;

// Định nghĩa các trạng thái của trò chơi
enum GameState {
    MENU,
    PLAYING,
    GAME_OVER_WIN,
    GAME_OVER_LOSE
};

class Game {
private:
    // Các thành phần SDL
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Các texture
    std::map<int, SDL_Texture*> textures;
    SDL_Texture* menuTexture;
    SDL_Texture* winTexture;
    SDL_Texture* loseTexture;
    SDL_Texture* backTexture;
    SDL_Texture* startButtonTexture;
    SDL_Texture* restartButtonTexture;

    // Âm thanh
    Mix_Music* backgroundMusic;

    // Trạng thái game
    GameState gameState;
    int flipCount;
    int score;
    Uint32 startTime;
    Uint32 flipBackTime;

    // Lưới game
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<bool>> revealed;
    std::vector<std::vector<bool>> matched;

    // Các ô đã click
    std::pair<int, int> firstClick;
    std::pair<int, int> secondClick;

public:
    Game();
    ~Game();

    // Khởi tạo và quản lý game
    bool initSDL();
    void loadResources();
    void initializeGrid();
    void closeSDL();

    // Xử lý game
    void handleMouseClick(int x, int y);
    void updateGameState();
    bool checkRestartButtonClick(int x, int y);

    // Render
    void renderGrid();
    void renderMenu();
    void renderResult(bool isWin);
    void renderTimer(Uint32 remainingTime);
    void renderScore();
    void renderTime();
    void renderRestartButton();

    // Âm thanh
    void playBackgroundMusic();
    void stopBackgroundMusic();

    // Tiện ích
    SDL_Texture* loadTexture(const std::string& path);

    // Getter cho gameState để kiểm tra trạng thái từ bên ngoài
    GameState getGameState() const { return gameState; }
    SDL_Renderer* getRenderer() const { return renderer; }
};

#endif // GAME_H

