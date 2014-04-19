#include <iostream>
#include <SDL2/SDL.h>


/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message too
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg)
{
    os << msg << " error: " << SDL_GetError() << std::endl;
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
	std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_Renderer * ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
	std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_Surface *bmp = SDL_LoadBMP("data/hello.bmp");
    if (bmp == nullptr)
    {
	std::cout << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_FreeSurface(bmp);
    if (tex == nullptr)
    {
	std::cout << "SDL_CreateTextureFromSurface Error: "
		  << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);

    SDL_Delay(2000);

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();


    return 0;
}
