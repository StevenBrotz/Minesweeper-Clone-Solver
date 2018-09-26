#ifndef graphics_hpp
#define graphics_hpp

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <string>

// Wrapper around SDL_Texture
class Texture
{
public:
    Texture();
    ~Texture();
    bool loadFromFile(std::string path);
    void free();
    void render(int x, int y);
    int getWidth();
    int getHeight();
    
private:
    SDL_Texture *texture; // The actual hardware texture
    int width;
    int height;
};

// Starts SDL and creates window
bool init();

// Struct with all textures
struct TextureStruct
{
    Texture counterNumbers[10];
    Texture numbers[8];
    Texture flag;
    Texture mine;
    Texture unrevealed;
    Texture dead;
    Texture happy;
    Texture glasses;
    Texture lightBulb;
};

// Loads media
bool loadMedia(TextureStruct &textures);

#endif /* graphics_hpp */
