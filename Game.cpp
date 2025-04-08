#include <fstream>
#include "Game.h"
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;

Game::Game() :
    window(nullptr),
    renderer(nullptr),
    menuTexture(nullptr),
    winTexture(nullptr),
    loseTexture(nullptr),
    backTexture(nullptr),
    startButtonTexture(nullptr),
    restartButtonTexture(nullptr),
    backgroundMusic(nullptr),
    flipCount(0),
    gameState(MENU),
    score(0),
    highScore(0),
    startTime(0),
    flipBackTime(0),
    firstClick(-1, -1),
    secondClick(-1, -1)
{
    grid.resize(GRID_ROWS,vector<int>(GRID_COLS, 0));
    revealed.resize(GRID_ROWS,vector<bool>(GRID_COLS, false));
    matched.resize(GRID_ROWS,vector<bool>(GRID_COLS, false));
}

Game::~Game() {
    closeSDL();
}

bool Game::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() <<endl;
        return false;
    }

    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow("Matching Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << endl;
        return false;
    }

    return true;
}

SDL_Texture* Game::loadTexture(const string& path) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    if (!texture) {
        cerr << "Failed to load texture: " << path << " SDL_Error: " << SDL_GetError() << endl;
    }
    return texture;
}

void Game::loadResources() {
    for (int i = 1; i <= 8; ++i) {
        textures[i] = loadTexture("images" + to_string(i) + ".png");
    }
    menuTexture = loadTexture("imagesmenu.png");
    winTexture = loadTexture("imageswin.png");
    loseTexture = loadTexture("imageslose.png");
    startButtonTexture = loadTexture("imagesstart_button.png");
    backTexture = loadTexture("imagesback1.png");
    restartButtonTexture = loadTexture("imagesrestart_button.png");
    backgroundMusic = Mix_LoadMUS("background_music.mp3");
    if (!backgroundMusic) {
        cerr << "Failed to load background music! Mix_Error: " << Mix_GetError() << endl;
    }
    loadHighScore();
}

void Game::loadHighScore() {
    ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    } else {
        highScore = 0;
    }
}

void Game::saveHighScore() {
    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

void Game::initializeGrid() {
    vector<int> tiles;
    for (int i = 1; i <= 8; ++i) {
        tiles.push_back(i);
        tiles.push_back(i);
    }
    mt19937 g(SDL_GetTicks());
    shuffle(tiles.begin(), tiles.end(), g);

    int index = 0;
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            grid[i][j] = tiles[index++];
            revealed[i][j] = false;
            matched[i][j] = false;
        }
    }
}

void Game::handleMouseClick(int x, int y) {
    if (gameState == MENU) {
        if (x > 150 && x < 350 && y > 450 && y < 530) {
            gameState = PLAYING;
            startTime = SDL_GetTicks();
            score = 0;
            initializeGrid();
            playBackgroundMusic();
        }
    }
    else if (gameState == GAME_OVER_WIN || gameState == GAME_OVER_LOSE) {
        if (checkRestartButtonClick(x, y)) {
            gameState = PLAYING;
            score = 0;
            flipCount = 0;
            startTime = SDL_GetTicks();
            initializeGrid();
            playBackgroundMusic();
        }
    }
    else if (gameState == PLAYING && flipBackTime == 0) {
        int row = y / TILE_SIZE;
        int col = x / TILE_SIZE;
        if (row >= GRID_ROWS || col >= GRID_COLS || revealed[row][col] || matched[row][col]) return;

        if (firstClick.first == -1) {
            firstClick = {row, col};
            revealed[row][col] = true;
        } else {
            secondClick = {row, col};
            revealed[row][col] = true;

            if (grid[firstClick.first][firstClick.second] == grid[secondClick.first][secondClick.second]) {
                matched[firstClick.first][firstClick.second] = true;
                matched[secondClick.first][secondClick.second] = true;
                firstClick = {-1, -1};
                secondClick = {-1, -1};
                score += 15;
            } else {
                flipBackTime = SDL_GetTicks() + FLIP_DELAY;
                score = max(0, score - 5);
            }
        }
        flipCount++;
    }
}

void Game::updateGameState() {
    if (gameState != PLAYING) return;

    if (flipBackTime > 0 && SDL_GetTicks() >= flipBackTime) {
        revealed[firstClick.first][firstClick.second] = false;
        revealed[secondClick.first][secondClick.second] = false;
        firstClick = {-1, -1};
        secondClick = {-1, -1};
        flipBackTime = 0;
    }

    int elapsedTime = SDL_GetTicks() - startTime;
    if (elapsedTime >= GAME_DURATION) {
        gameState = GAME_OVER_LOSE;
        stopBackgroundMusic();
        return;
    }

    bool allMatched = true;
    for (const auto& row : matched) {
        for (bool cell : row) {
            if (!cell) {
                allMatched = false;
                break;
            }
        }
    }
    if (allMatched) {
        gameState = GAME_OVER_WIN;
        if (score > highScore) {
            highScore = score;
            saveHighScore();
        }
        stopBackgroundMusic();
    }
}

bool Game::checkRestartButtonClick(int x, int y) {
    SDL_Rect buttonRect = {SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 100, 200, 80};
    return (x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
            y >= buttonRect.y && y <= buttonRect.y + buttonRect.h);
}

void Game::renderMenu() {
    if (menuTexture) {
        SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, menuTexture, nullptr, &destRect);
    }

    SDL_Rect startButton = {150, 450, 200, 80};
    if (startButtonTexture) {
        SDL_RenderCopy(renderer, startButtonTexture, nullptr, &startButton);
    }
}

void Game::renderGrid() {
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            SDL_Rect rect = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            if (revealed[i][j] || matched[i][j]) {
                SDL_Texture* tileTexture = textures[grid[i][j]];
                if (tileTexture) {
                    SDL_RenderCopy(renderer, tileTexture, nullptr, &rect);
                }
            } else {
                if (backTexture) {
                    SDL_RenderCopy(renderer, backTexture, nullptr, &rect);
                } else {
                    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
}

void Game::renderTime() {
    TTF_Font* font = TTF_OpenFont("boldItalic.ttf", 24);
    if (!font) return;

    Uint32 elapsedTime = SDL_GetTicks() - startTime;
    Uint32 remainingTime = (GAME_DURATION - elapsedTime) / 1000;

    SDL_Color color = {0, 0, 0, 255};
    if (remainingTime <= 10) {
        color = {255, 0, 0, 255};
    }

    string timeText = "Time: " + to_string(remainingTime) + "s";
    SDL_Surface* surface = TTF_RenderText_Solid(font, timeText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {10, 50, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

void Game::renderScore() {
    TTF_Font* font = TTF_OpenFont("boldItalic.ttf", 24);
    if (!font) {
        cerr << "Failed to load font: " << TTF_GetError() << endl;
        return;
    }

    string scoreText = "Score: " + to_string(score);
    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}


void Game::renderHighScore() {
    TTF_Font* font = TTF_OpenFont("boldItalic.ttf", 24);
    if (!font) return;

    string highScoreText = "High Score: " + to_string(highScore);
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, highScoreText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {SCREEN_WIDTH / 2 - surface->w / 2,SCREEN_HEIGHT / 2 + 50,surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

void Game::renderRestartButton() {
    SDL_Rect buttonRect = {SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 100, 200, 80};

    if (restartButtonTexture) {
        SDL_RenderCopy(renderer, restartButtonTexture, nullptr, &buttonRect);
    }
}



void Game::renderResult(bool isWin) {
    SDL_Texture* resultTexture = isWin ? winTexture : loseTexture;
    if (resultTexture) {
        SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, resultTexture, nullptr, &destRect);
    }

    TTF_Font* font = TTF_OpenFont("boldItalic.ttf", 28);
    if (font) {
        SDL_Color color = {255, 255, 255, 255};

        string scoreText = "Score: " + to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect = {SCREEN_WIDTH/2 - scoreSurface->w/2, SCREEN_HEIGHT/2 - 50, scoreSurface->w, scoreSurface->h};
        SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);

        if (isWin) {
            string flipText = "Flips: " + to_string(flipCount);
            SDL_Surface* flipSurface = TTF_RenderText_Solid(font, flipText.c_str(), color);
            SDL_Texture* flipTexture = SDL_CreateTextureFromSurface(renderer, flipSurface);
            SDL_Rect flipRect = {SCREEN_WIDTH/2 - flipSurface->w/2, SCREEN_HEIGHT/2, flipSurface->w, flipSurface->h};
            SDL_RenderCopy(renderer, flipTexture, nullptr, &flipRect);

            SDL_FreeSurface(flipSurface);
            SDL_DestroyTexture(flipTexture);
        }

        if (!isWin) {
            string timeText = "Time's up!";
            SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), color);
            SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
            SDL_Rect timeRect = {SCREEN_WIDTH/2 - timeSurface->w/2, SCREEN_HEIGHT/2, timeSurface->w, timeSurface->h};
            SDL_RenderCopy(renderer, timeTexture, nullptr, &timeRect);

            SDL_FreeSurface(timeSurface);
            SDL_DestroyTexture(timeTexture);
        }

        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);
        TTF_CloseFont(font);
    }

    renderRestartButton();
}

void Game::playBackgroundMusic() {
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }
}

void Game::stopBackgroundMusic() {
    Mix_HaltMusic();
}



void Game::closeSDL() {
    for (auto& texturePair : textures) {
        SDL_DestroyTexture(texturePair.second);
    }

    if (menuTexture) SDL_DestroyTexture(menuTexture);
    if (winTexture) SDL_DestroyTexture(winTexture);
    if (loseTexture) SDL_DestroyTexture(loseTexture);
    if (startButtonTexture) SDL_DestroyTexture(startButtonTexture);

    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}
