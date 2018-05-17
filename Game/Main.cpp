#include "main.h"

int main(int argc, char* argv[]) {
    // Create the game object
    Game* game = new Game();

    // Initialize and run the game
	//game->loadMedia("start-screen.bmp", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!game->Init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		game->loadMedia("start-screen2.png", 0, 0, 800, 640);
		// Main loop flag
		bool quit = false;

		// Event handler
		SDL_Event e;
		while (!quit)
		{
			// handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				// User requests quit
				if (e.type == SDL_KEYUP)
				{
					std::cout << "val" << std::endl;
				}

				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
				if ((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_RETURN))
				{

					
					std::cout << mouseX << "---" << mouseY << std::endl;
					game->Run();

				}
				/*if (e.type == SDL_MOUSEMOTION) {
					mouseX = e.button.x;
					mouseY = e.button.y;
				}*/
			}

			// Clear screen
			
		}
	}

    // Clean up
    delete game;
    return 0;
}
