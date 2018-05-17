#include "Game.h"

Game::Game() {
    window = 0;
    renderer = 0;
}

Game::~Game() {

}

bool Game::Init() {

	// Initialization flag
	SDL_Init(SDL_INIT_VIDEO);

	// Create window
	// 800 640
	window = SDL_CreateWindow("Break ball",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 640, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!window) {
		std::cout << "Error creating window:" << SDL_GetError() << std::endl;
		return false;
	}

	// Create renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cout << "Error creating renderer:" << SDL_GetError() << std::endl;
		return false;
	}

	// Initialize resources
	/*SDL_Surface* surface = IMG_Load("background.bmp");
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);*/

	// Initialize timing
	lasttick = SDL_GetTicks();
	fpstick = lasttick;
	fps = 0;
	framecount = 0;

	testx = 0;
	testy = 0;

	return true;
    
}



void Game::Clean() {
    // Clean resources
    SDL_DestroyTexture(texture);

    // Clean renderer and window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Game::Run() {
    board = new Board(renderer);
    paddle = new Paddle(renderer);
    ball = new Ball(renderer);

    NewGame();
    // Main loop
    while (1) {
        // Handler events
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                break;
			}
        }
        // Calculate delta and fps
        unsigned int curtick = SDL_GetTicks();
		flameStart = SDL_GetTicks();
        float delta = (curtick - lasttick) / 1000.0f;
        if (curtick - fpstick >= FPS_DELAY) {
            fps = framecount * (1000.0f / (curtick - fpstick));
            fpstick = curtick;
            framecount = 0;
            //std::cout << "FPS: " << fps << std::endl;
            char buf[100];
            snprintf(buf, 100, "Break ball (FPS: %u)", fps);
            SDL_SetWindowTitle(window, buf);
        } else {
            framecount++;
        }
        lasttick = curtick;

		if (live == 0)
		{
			loadMedia("start-screen.png", 0, 0, 800, 640);
			if (SDL_PollEvent(&e))
			{
				if (e.type == SDL_KEYDOWN)
				{
					if (e.key.keysym.sym == SDLK_s)
					{

						NewGame();
					}
				}
			}
			break;
		}
        // Update and render the game
		Update(delta);
		
		Render(delta);
		if (live == 0)
		{

		}
		// Delay fps
		flametime = SDL_GetTicks() - flameStart;
		if (flametime < DELAY_TIME)
		{
			SDL_Delay(DELAY_TIME - flametime);
		}
    }
	
	
	delete board;
	delete paddle;
	delete ball;
    Clean();

    SDL_Quit();
}

void Game::NewGame() {
	//loadNextScreen();
    board->CreateLevel();
    ResetPaddle();
}

void Game::ResetPaddle() {
    paddlestick = true;
    StickBall();
	live--;
}

void Game::StickBall() {
    ball->x = paddle->x + paddle->width/2 - ball->width/2;
    ball->y = paddle->y - ball->height;
}

void Game::Update(float delta) {
    // Game logic

    // Input
    int mx, my;
    Uint8 mstate = SDL_GetMouseState(&mx, &my);
    SetPaddleX(mx - paddle->width/2.0f);

    if (mstate&SDL_BUTTON(1)) {
        if (paddlestick) {
            paddlestick = false;
            ball->SetDirection(1, -1);
        }
    }

    if (paddlestick) {
        StickBall();
    }

    CheckBoardCollisions();
    CheckPaddleCollisions();
    CheckBrickCollisions2();

    if (GetBrickCount() == 0) {
		Winner();
		Clean();
        //NewGame();
    }
    board->Update(delta);
    paddle->Update(delta);

    if (!paddlestick) {
        ball->Update(delta);
    }
}

void Game::SetPaddleX(float x) {
    float newx;
    if (x < board->x) {
        // Upper bound
        newx = board->x;
    } else if (x + paddle->width > board->x + board->width) {
        // Lower bound
        newx = board->x + board->width - paddle->width;
    } else {
        newx = x;
    }
    paddle->x = newx;
}

// COLLISSIONS

void Game::CheckBoardCollisions() {
    // Top and bottom collisions
    if (ball->y < board->y) {
        // Top
        // Keep the ball within the board and reflect the y-direction
        ball->y = board->y;
        ball->diry *= -1;
    } else if (ball->y + ball->height > board->y + board->height) {
        // Bottom

        // Ball lost
        ResetPaddle();
    }

    // Left and right collisions
    if (ball->x <= board->x) {
        // Left
        // Keep the ball within the board and reflect the x-direction
        ball->x = board->x;
        ball->dirx *= -1;
    } else if (ball->x + ball->width >= board->x + board->width) {
        // Right
        // Keep the ball within the board and reflect the x-direction
        ball->x = board->x + board->width - ball->width;
        ball->dirx *= -1;
    }
}

float Game::GetReflection(float hitx) {
    // Make sure the hitx variable is within the width of the paddle
    if (hitx < 0) {
        hitx = 0;
    } else if (hitx > paddle->width) {
        hitx = paddle->width;
    }

    // Everything to the left of the center of the paddle is reflected to the left
    // while everything right of the center is reflected to the right
    hitx -= paddle->width / 2.0f;

    // Scale the reflection, making it fall in the range -2.0f to 2.0f
    return 2.0f * (hitx / (paddle->width / 2.0f));
}


void Game::CheckPaddleCollisions() {
    // Get the center x-coordinate of the ball
    float ballcenterx = ball->x + ball->width / 2.0f;

    // Check paddle collision
    if (ball->Collides(paddle)) {
        ball->y = paddle->y - ball->height;
        ball->SetDirection(GetReflection(ballcenterx - paddle->x), -1);
        //ball->SetDirection(0, -1);
    }
}

void Game::CheckBrickCollisions() {
    for (int i=0; i<BOARD_WIDTH; i++) {
        for (int j=0; j<BOARD_HEIGHT; j++) {
            Brick brick = board->bricks[i][j];

            // Check if brick is present
            if (brick.state) {
                // Brick x and y coordinates
                float brickx = board->brickoffsetx + board->x + i*BOARD_BRWIDTH;
                float bricky = board->brickoffsety + board->y + j*BOARD_BRHEIGHT;

                // Check ball-brick collision
                // Determine the collision using the half-widths of the rectangles
                // http://stackoverflow.com/questions/16198437/minkowski-sum-for-rectangle-intersection-calculation
                // http://gamedev.stackexchange.com/questions/29786/a-simple-2d-rectangle-collision-algorithm-that-also-determines-which-sides-that
                // http://gamedev.stackexchange.com/questions/24078/which-side-was-hit/24091#24091
                float w = 0.5f * (ball->width + BOARD_BRWIDTH);
                float h = 0.5f * (ball->height + BOARD_BRHEIGHT);
                float dx = (ball->x + 0.5f*ball->width) - (brickx + 0.5f*BOARD_BRWIDTH);
                float dy = (ball->y + 0.5f*ball->height) - (bricky + 0.5f*BOARD_BRHEIGHT);

                if (fabs(dx) <= w && fabs(dy) <= h) {
                    // Collision detected
                    board->bricks[i][j].state = false;

                    float wy = w * dy;
                    float hx = h * dx;

                    if (wy > hx) {
                        if (wy > -hx) {
                            // Bottom (y is flipped)
                            BallBrickResponse(3);
                        } else {
                            // Left
                            BallBrickResponse(0);
                        }
                    } else {
                        if (wy > -hx) {
                            // Right
                            BallBrickResponse(2);
                        } else {
                            // Top (y is flipped)
                            BallBrickResponse(1);
                        }
                    }
                    return;
                }
            }
        }
    }
}

void Game::CheckBrickCollisions2() {
    for (int i=0; i<BOARD_WIDTH; i++) {
        for (int j=0; j<BOARD_HEIGHT; j++) {
            Brick brick = board->bricks[i][j];

            // Check if brick is present
            if (brick.state) {
                // Brick x and y coordinates
                float brickx = board->brickoffsetx + board->x + i*BOARD_BRWIDTH;
                float bricky = board->brickoffsety + board->y + j*BOARD_BRHEIGHT;

                // Center of the ball x and y coordinates
                float ballcenterx = ball->x + 0.5f*ball->width;
                float ballcentery = ball->y + 0.5f*ball->height;

                // Center of the brick x and y coordinates
                float brickcenterx = brickx + 0.5f*BOARD_BRWIDTH;
                float brickcentery = bricky + 0.5f*BOARD_BRHEIGHT;

                if (ball->x <= brickx + BOARD_BRWIDTH && ball->x+ball->width >= brickx && ball->y <= bricky + BOARD_BRHEIGHT && ball->y + ball->height >= bricky) {
                    // Collision detected, remove the brick
                    board->bricks[i][j].state = false;

                    // Asume the ball goes slow enough to not skip through the bricks

                    // Calculate ysize
                    float ymin = 0;
                    if (bricky > ball->y) {
                        ymin = bricky;
                    } else {
                        ymin = ball->y;
                    }

                    float ymax = 0;
                    if (bricky+BOARD_BRHEIGHT < ball->y+ball->height) {
                        ymax = bricky+BOARD_BRHEIGHT;
                    } else {
                        ymax = ball->y+ball->height;
                    }

                    float ysize = ymax - ymin;

                    // Calculate xsize
                    float xmin = 0;
                    if (brickx > ball->x) {
                        xmin = brickx;
                    } else {
                        xmin = ball->x;
                    }

                    float xmax = 0;
                    if (brickx+BOARD_BRWIDTH < ball->x+ball->width) {
                        xmax = brickx+BOARD_BRWIDTH;
                    } else {
                        xmax = ball->x+ball->width;
                    }

                    float xsize = xmax - xmin;

                    // The origin is at the top-left corner of the screen!
                    // Set collision response
                    if (xsize > ysize) {
                        if (ballcentery > brickcentery) {
                            // Bottom
                            ball->y += ysize + 0.01f; // Move out of collision
                            BallBrickResponse(3);
                        } else {
                            // Top
                            ball->y -= ysize + 0.01f; // Move out of collision
                            BallBrickResponse(1);
                        }
                    } else {
                        if (ballcenterx < brickcenterx) {
                            // Left
                            ball->x -= xsize + 0.01f; // Move out of collision
                            BallBrickResponse(0);
                        } else {
                            // Right
                            ball->x += xsize + 0.01f; // Move out of collision
                            BallBrickResponse(2);
                        }
                    }

                    return;
                }
            }
        }
    }
}

void Game::BallBrickResponse(int dirindex) {
    // dirindex 0: Left, 1: Top, 2: Right, 3: Bottom

    // Direction factors
    int mulx = 1;
    int muly = 1;

    if (ball->dirx > 0) {
        // Ball is moving in the positive x direction
        if (ball->diry > 0) {
            // Ball is moving in the positive y direction
            // +1 +1
            if (dirindex == 0 || dirindex == 3) {
                mulx = -1;
            } else {
                muly = -1;
            }
        } else if (ball->diry < 0) {
            // Ball is moving in the negative y direction
            // +1 -1
            if (dirindex == 0 || dirindex == 1) {
                mulx = -1;
            } else {
                muly = -1;
            }
        }
    } else if (ball->dirx < 0) {
        // Ball is moving in the negative x direction
        if (ball->diry > 0) {
            // Ball is moving in the positive y direction
            // -1 +1
            if (dirindex == 2 || dirindex == 3) {
                mulx = -1;
            } else {
                muly = -1;
            }
        } else if (ball->diry < 0) {
            // Ball is moving in the negative y direction
            // -1 -1
            if (dirindex == 1 || dirindex == 2) {
                mulx = -1;
            } else {
                muly = -1;
            }
        }
    }

    // Set the new direction of the ball, by multiplying the old direction
    // with the determined direction factors
    ball->SetDirection(mulx*ball->dirx, muly*ball->diry);
}

// Number of brick

int Game::GetBrickCount() {
    int brickcount = 0;
    for (int i=0; i<BOARD_WIDTH; i++) {
        for (int j=0; j<BOARD_HEIGHT; j++) {
            Brick brick = board->bricks[i][j];
            if (brick.state) {
                brickcount++;
            }
        }
    }

    return brickcount;
}

// Render Game

void Game::Render(float delta) {
    SDL_RenderClear(renderer);

    board->Render(delta);
    paddle->Render(delta);
    ball->Render(delta);

    SDL_RenderPresent(renderer);
}

// Load image

SDL_Texture* Game::loadTexture(std::string path)
{
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface != NULL)
	{
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		SDL_FreeSurface(loadedSurface);
	}

	return  newTexture;
}

void Game::loadMedia(std::string path, int x, int y, int w, int h)
{
	texture = loadTexture(path.c_str());
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = w;
	dest.h = h;
	SDL_RenderCopy(renderer, texture, NULL, &dest);
	SDL_RenderPresent(renderer);
}

void Game::Winner()
{
	std::cout << "Congratulation!! You win";

}
void Game::GameOver()
{
	//SDL_RenderClear(renderer);

	std::cout << "Game Over" << std::endl;

	SDL_DestroyTexture(texture);

	// Clean renderer and window
	SDL_DestroyRenderer(renderer);

	loadMedia("start-screen.png", 0, 0, 800, 640);
	//Clean();
	//ClearScreen();
	//DisplayText("You Lose", 350, 250, 12, 255, 255, 255, 0, 0, 0);

	//SDL_RenderPresent(renderer);
	//Clean();
}

// Display text
void Game::DisplayText(std::string text, int x, int y, int size, int fR, int fG, int fB, int bR, int bG, int bB)
{
	TTF_Font* font = TTF_OpenFont("arial.ttf", size);

	SDL_Color foreground = { fR, fG, fB }; // Text color
	SDL_Color background = { bR, bG, bB }; // Background color


	SDL_Surface* temp = TTF_RenderText_Shaded(font, text.c_str(), foreground, background);

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, temp);

	SDL_FreeSurface(temp);

	SDL_Rect srcRest;
	SDL_Rect desRect;
	TTF_SizeText(font, text.c_str(), &srcRest.w, &desRect.h);

	srcRest.x = 0;
	srcRest.y = 0;

	desRect.x = 200;
	desRect.y = 270;

	desRect.w = srcRest.w;
	desRect.h = srcRest.h;

	SDL_RenderCopy(renderer, texture, &srcRest, &desRect);

	SDL_RenderPresent(renderer);

	//TTF_CloseFont(font);
}

void Game::ClearScreen()
{
	SDL_FillRect(g_window, 0, 0);
}