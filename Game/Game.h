#ifndef GAME_H_
#define GAME_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <iostream>
#include <stdio.h>

#include "Board.h"
#include "Paddle.h"
#include "Ball.h"

#define FPS 60
#define FPS_DELAY 500

class Game {
public:
    Game();
    ~Game();

	const int SCREEN_WIDTH = 1300;
	const int SCREEN_HEIGHT = 652;

	int live = 3;
	int n;

	bool Init();
    void Run();
	void Winner();
	void GameOver();
	void ClearScreen();
	void DisplayText(std::string text, int x, int y, int size, int fR, int fG, int fB, int bR, int bG, int bB);

	void loadMedia(std::string path, int x, int y, int w, int h);

private:
    SDL_Window* window;

	SDL_Surface* g_window = NULL;

    SDL_Renderer* renderer;

    SDL_Texture* texture;

	SDL_Texture* loadTexture(std::string path);

	bool isRunning;
    // Timing
	const int DELAY_TIME = 1000 / FPS;
	Uint32 flameStart, flametime;
    unsigned int lasttick, fpstick, fps, framecount;

    // Test
    float testx, testy;

    Board* board;
    Paddle* paddle;
    Ball* ball;
    bool paddlestick;

    void Clean();

    void Update(float delta);
    void Render(float delta);

    void NewGame();
    void ResetPaddle();
    void StickBall();


    void SetPaddleX(float x);
    void CheckBoardCollisions();
    float GetReflection(float hitx);
    void CheckPaddleCollisions();
    void CheckBrickCollisions();
    void CheckBrickCollisions2();
    void BallBrickResponse(int dirindex);
    int GetBrickCount();

	bool startGame = false;
	int posxMouse, posyMouse;

};

#endif
