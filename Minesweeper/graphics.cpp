#include <cstdio>
#include <string>
#include <sstream>
#include <vector>
#include "graphics.hpp"
#include "shared.hpp"

// Window we render to
SDL_Window *window = nullptr;

// The window renderer
SDL_Renderer *renderer = nullptr;

Texture::Texture()
{
    texture = nullptr;
    width = 0;
    height = 0;
}

Texture::~Texture()
{
    free();
}

bool Texture::loadFromFile(std::string path)
{
    // Get rid of preexisting texture
    free();
    SDL_Texture* newTexture = nullptr;
    
    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else
    {
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == nullptr)
        {
            printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }
        else
        {
            width = loadedSurface->w;
            height = loadedSurface->h;
        }
        
        SDL_FreeSurface(loadedSurface);
    }
    
    // Return success
    texture = newTexture;
    return texture != nullptr;
}

void Texture::free()
{
    // Free texture if it exists
    if (texture != nullptr)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0;
        height = 0;
    }
}

void Texture::render(int x, int y)
{
    // Set rendering space and render to screen
    SDL_Rect renderQuad = {x, y, width, height};
    SDL_RenderCopy(renderer, texture, nullptr, &renderQuad);
}

int Texture::getWidth()
{
    return width;
}

int Texture::getHeight()
{
    return height;
}

bool init()
{
    bool success = true;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
            printf("Warning: Linear texture filtering not enabled!");
        
        // Create window we'll be rendering to
        window = SDL_CreateWindow("Minesweeper",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SCREEN_WIDTH,
                                  SCREEN_HEIGHT,
                                  SDL_WINDOW_SHOWN);
        if (window == nullptr)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer == nullptr)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                // Initialize renderer color
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                
                // Initialize PNG loading
                int imflags = IMG_INIT_PNG;
                if (!(IMG_Init(imflags) & imflags))
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }
    
    return success;
}

bool loadMedia(TextureStruct &textures)
{
    bool success = true;
    
    std::vector<std::string> numbers = {"one", "two", "three", "four", "five", "six", "seven", "eight"};
    for (int i = 0; i < numbers.size(); i++)
    {
        auto path = std::string("images/") + numbers[i] + ".png";
        if (!textures.numbers[i].loadFromFile(path))
        {
            printf("Failed to load %s.png\n", numbers[i].c_str());
            success = false;
        }
    }
    
    for (int i = 0; i < 10; i++)
    {
        auto path = std::string("images/counter_") + std::to_string(i) + ".png";
        if (!textures.counterNumbers[i].loadFromFile(path))
        {
            printf("Failed to load counter_%d.png!\n", i);
            success = false;
        }
    }
    
    if(!textures.flag.loadFromFile("images/flag.png"))
    {
        printf("Failed to load flag.png!\n");
        success = false;
    }
    
    if(!textures.mine.loadFromFile("images/mine.png"))
    {
        printf("Failed to load mine.png!\n");
        success = false;
    }
    
    if (!textures.unrevealed.loadFromFile("images/unrevealed.png"))
    {
        printf("Failed to load unrevealed.png!\n");
        success = false;
    }
    
    if (!textures.dead.loadFromFile("images/dead_face.png"))
    {
        printf("Failed to load dead_face.png!\n");
        success = false;
    }
    
    if (!textures.happy.loadFromFile("images/happy_face.png"))
    {
        printf("Failed to load happy_face.png!\n");
        success = false;
    }
    
    if (!textures.glasses.loadFromFile("images/glasses_face.png"))
    {
        printf("Failed to load glasses_face.png!\n");
        success = false;
    }
    
    if (!textures.lightBulb.loadFromFile("images/light_bulb.png"))
    {
        printf("Failed to load light_bulb.png!\n");
        success = false;
    }
    
    return success;
}
