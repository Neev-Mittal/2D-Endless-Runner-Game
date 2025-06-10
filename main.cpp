#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <fstream>
#include <string>

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 400;
const int GROUND_LEVEL = 300;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 80;
const int OBSTACLE_WIDTH = 30;
const int OBSTACLE_HEIGHT = 50;
const int JUMP_VELOCITY = -16;
const float GRAVITY = 0.8f;
const int GAME_SPEED_INITIAL = 5;
const int GAME_SPEED_INCREMENT = 1;
const int SPEED_UP_SCORE = 500;
const std::string HIGH_SCORE_FILE = "highscore.dat";
const std::string FONT_FILE = "arial.ttf"; // Make sure this file exists in your project directory

// Obstacle types
enum ObstacleType {
    COFFEE_CUP,
    BRIEFCASE,
    FIRE_HYDRANT,
    TRASH_CAN,
    CAR,
    BICYCLE,
    PUDDLE,
    DOG,
    OBSTACLE_TYPE_COUNT
};

class Player {
public:
    float x, y;
    float velocity;
    bool isJumping;
    SDL_Rect hitbox;
    bool jumpScored;
    int animFrame;    // Current animation frame
    int frameCounter; // Frame counter for animation timing

    Player() : x(100), y(GROUND_LEVEL - PLAYER_HEIGHT), 
              velocity(0), isJumping(false), jumpScored(false),
              animFrame(0), frameCounter(0) {
        updateHitbox();
    }

    void update() {
        // Update animation frame counter
        frameCounter++;
        if (frameCounter >= 5) { // Change animation frame every 5 game frames
            frameCounter = 0;
            animFrame = (animFrame + 1) % 4; // 4 animation frames (0-3)
        }

        if (isJumping) {
            velocity += GRAVITY;
            y += velocity;

            // Check if player landed
            if (y >= GROUND_LEVEL - PLAYER_HEIGHT) {
                y = GROUND_LEVEL - PLAYER_HEIGHT;
                velocity = 0;
                isJumping = false;
                jumpScored = false;
            }
        }
        updateHitbox();
    }

    bool jump() {
        if (!isJumping) {
            isJumping = true;
            velocity = JUMP_VELOCITY;
            jumpScored = false;
            return true;
        }
        return false;
    }

    bool canScoreJump() {
        if (isJumping && !jumpScored && velocity > -2 && velocity < 2) {
            jumpScored = true;
            return true;
        }
        return false;
    }

    void updateHitbox() {
        hitbox = {static_cast<int>(x), static_cast<int>(y), PLAYER_WIDTH, PLAYER_HEIGHT};
    }

void render(SDL_Renderer* renderer) {
    // Load PNG image for character texture
    static SDL_Texture* characterTexture = nullptr;
    static bool textureLoadAttempted = false;
    
    if (characterTexture == nullptr && !textureLoadAttempted) {
        textureLoadAttempted = true;  // Only try to load once
        SDL_Surface* surface = IMG_Load("office_worker.png");
        if (surface) {
            characterTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }

    if (characterTexture) {
        // Draw the character PNG as the full body
        SDL_Rect destRect = {static_cast<int>(x), static_cast<int>(y), hitbox.w, hitbox.h};
        SDL_RenderCopy(renderer, characterTexture, nullptr, &destRect);
    } else {
        // Fallback rendering if texture couldn't be loaded
        // Draw the character
        SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255); // Blue suit
        SDL_Rect body = {static_cast<int>(x), static_cast<int>(y), hitbox.w, hitbox.h - 30};
        SDL_RenderFillRect(renderer, &body);
        
        // Head
        SDL_SetRenderDrawColor(renderer, 255, 213, 170, 255); // Skin tone
        SDL_Rect head = {static_cast<int>(x + 10), static_cast<int>(y), 30, 30};
        SDL_RenderFillRect(renderer, &head);
    }

    // Briefcase (if not jumping)
    if (!isJumping || animFrame % 2 == 0) {
        SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255); // Brown
        SDL_Rect briefcase = {static_cast<int>(x + 5), static_cast<int>(y + 60), 20, 15};
        SDL_RenderFillRect(renderer, &briefcase);

        // Handle
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect handle = {static_cast<int>(x + 12), static_cast<int>(y + 55), 6, 5};
        SDL_RenderFillRect(renderer, &handle);
    }

    // Legs - show running animation
    SDL_SetRenderDrawColor(renderer, 30, 30, 60, 255); // Dark pants
    if (isJumping) {
        SDL_Rect leg1 = {static_cast<int>(x + 15), static_cast<int>(y + 60), 8, 20};
        SDL_Rect leg2 = {static_cast<int>(x + 30), static_cast<int>(y + 60), 8, 15};
        SDL_RenderFillRect(renderer, &leg1);
        SDL_RenderFillRect(renderer, &leg2);
    } else {
        if (animFrame % 2 == 0) {
            SDL_Rect leg1 = {static_cast<int>(x + 15), static_cast<int>(y + 50), 8, 30};
            SDL_Rect leg2 = {static_cast<int>(x + 30), static_cast<int>(y + 60), 8, 20};
            SDL_RenderFillRect(renderer, &leg1);
            SDL_RenderFillRect(renderer, &leg2);
        } else {
            SDL_Rect leg1 = {static_cast<int>(x + 15), static_cast<int>(y + 60), 8, 20};
            SDL_Rect leg2 = {static_cast<int>(x + 30), static_cast<int>(y + 50), 8, 30};
            SDL_RenderFillRect(renderer, &leg1);
            SDL_RenderFillRect(renderer, &leg2);
        }
    }
}
};


class Obstacle {
public:
    float x;
    int width, height;
    SDL_Rect hitbox;
    ObstacleType type;

    Obstacle(float startX, int w, int h, ObstacleType t) : x(startX), width(w), height(h), type(t) {
        updateHitbox();
    }

    void update(int gameSpeed) {
        x -= gameSpeed;
        updateHitbox();
    }

    void updateHitbox() {
        hitbox = {static_cast<int>(x), GROUND_LEVEL - height, width, height};
    }

    bool isOffScreen() {
        return x + width < 0;
    }

    void render(SDL_Renderer* renderer) {
        switch (type) {
            case COFFEE_CUP:
                renderCoffeeCup(renderer);
                break;
            case BRIEFCASE:
                renderBriefcase(renderer);
                break;
            case FIRE_HYDRANT:
                renderFireHydrant(renderer);
                break;
            case TRASH_CAN:
                renderTrashCan(renderer);
                break;
            case CAR:
                renderCar(renderer);
                break;
            case BICYCLE:
                renderBicycle(renderer);
                break;
            case PUDDLE:
                renderPuddle(renderer);
                break;
            case DOG:
                renderDog(renderer);
                break;
            default:
                // Default obstacle
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                SDL_RenderFillRect(renderer, &hitbox);
                break;
        }
    }

private:
    void renderCoffeeCup(SDL_Renderer* renderer) {
        // Draw coffee cup
        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown
        SDL_RenderFillRect(renderer, &hitbox);
        
        // Cup handle
        SDL_Rect handle = {static_cast<int>(x + width), GROUND_LEVEL - height + 10, 10, 20};
        SDL_RenderFillRect(renderer, &handle);
        
        // Coffee
        SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255); // Darker brown
        SDL_Rect coffee = {static_cast<int>(x + 5), GROUND_LEVEL - height + 5, width - 10, 10};
        SDL_RenderFillRect(renderer, &coffee);
        
        // Steam
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 150); // Light gray
        for (int i = 0; i < 3; i++) {
            SDL_Rect steam = {static_cast<int>(x + 10 + i * 7), GROUND_LEVEL - height - 5 - (i % 2) * 5, 3, 5};
            SDL_RenderFillRect(renderer, &steam);
        }
    }
    
    void renderBriefcase(SDL_Renderer* renderer) {
        // Main briefcase body
        SDL_SetRenderDrawColor(renderer, 80, 40, 20, 255); // Dark brown
        SDL_RenderFillRect(renderer, &hitbox);
        
        // Handle
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); // Black
        SDL_Rect handle = {static_cast<int>(x + width/3), GROUND_LEVEL - height - 8, width/3, 8};
        SDL_RenderFillRect(renderer, &handle);
        
        // Clasps
        SDL_SetRenderDrawColor(renderer, 200, 180, 0, 255); // Gold
        SDL_Rect clasp1 = {static_cast<int>(x + width/4), GROUND_LEVEL - height/2, 5, 5};
        SDL_Rect clasp2 = {static_cast<int>(x + width*3/4 - 5), GROUND_LEVEL - height/2, 5, 5};
        SDL_RenderFillRect(renderer, &clasp1);
        SDL_RenderFillRect(renderer, &clasp2);
    }
    
    void renderFireHydrant(SDL_Renderer* renderer) {
        // Main body
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, 255); // Red
        SDL_RenderFillRect(renderer, &hitbox);
        
        // Top cap
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark gray
        SDL_Rect cap = {static_cast<int>(x - 5), GROUND_LEVEL - height - 10, width + 10, 10};
        SDL_RenderFillRect(renderer, &cap);
        
        // Side outlets
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); // Light gray
        SDL_Rect outlet1 = {static_cast<int>(x - 8), GROUND_LEVEL - height + 15, 8, 8};
        SDL_Rect outlet2 = {static_cast<int>(x + width), GROUND_LEVEL - height + 15, 8, 8};
        SDL_RenderFillRect(renderer, &outlet1);
        SDL_RenderFillRect(renderer, &outlet2);
        
        // Chain
        SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255); // Gray
        for (int i = 0; i < 3; i++) {
            SDL_Rect chain = {static_cast<int>(x + width/2 - 2), GROUND_LEVEL - height + 5 + i*8, 4, 4};
            SDL_RenderFillRect(renderer, &chain);
        }
    }
    
    void renderTrashCan(SDL_Renderer* renderer) {
        // Main body
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255); // Gray
        SDL_RenderFillRect(renderer, &hitbox);
        
        // Lid
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); // Darker gray
        SDL_Rect lid = {static_cast<int>(x - 5), GROUND_LEVEL - height, width + 10, 10};
        SDL_RenderFillRect(renderer, &lid);
        
        // Trash pattern
        SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255); // Green
        SDL_Rect trash1 = {static_cast<int>(x + 5), GROUND_LEVEL - height + 15, 5, 10};
        SDL_RenderFillRect(renderer, &trash1);
        
        SDL_SetRenderDrawColor(renderer, 200, 200, 100, 255); // Yellow-ish
        SDL_Rect trash2 = {static_cast<int>(x + width - 10), GROUND_LEVEL - height + 20, 8, 5};
        SDL_RenderFillRect(renderer, &trash2);
    }
    
    void renderCar(SDL_Renderer* renderer) {
        // Car body
        SDL_SetRenderDrawColor(renderer, 30, 100, 180, 255); // Blue car
        SDL_RenderFillRect(renderer, &hitbox);
        
        // Windshield and windows
        SDL_SetRenderDrawColor(renderer, 200, 230, 255, 255); // Light blue
        SDL_Rect windshield = {static_cast<int>(x + width/5), GROUND_LEVEL - height + 10, width/2, height/3};
        SDL_RenderFillRect(renderer, &windshield);
        
        // Wheels
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); // Black
        SDL_Rect wheel1 = {static_cast<int>(x + width/5), GROUND_LEVEL - 15, 15, 15};
        SDL_Rect wheel2 = {static_cast<int>(x + width - width/3), GROUND_LEVEL - 15, 15, 15};
        SDL_RenderFillRect(renderer, &wheel1);
        SDL_RenderFillRect(renderer, &wheel2);
        
        // Hubcaps
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Silver
        SDL_Rect hub1 = {static_cast<int>(x + width/5 + 5), GROUND_LEVEL - 10, 5, 5};
        SDL_Rect hub2 = {static_cast<int>(x + width - width/3 + 5), GROUND_LEVEL - 10, 5, 5};
        SDL_RenderFillRect(renderer, &hub1);
        SDL_RenderFillRect(renderer, &hub2);
        
        // Headlights
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255); // Yellow-white
        SDL_Rect headlight = {static_cast<int>(x + width - 8), GROUND_LEVEL - height + height/2, 8, 8};
        SDL_RenderFillRect(renderer, &headlight);
    }
    
    void renderBicycle(SDL_Renderer* renderer) {
        // Wheels
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
        int wheelRadius = height / 2;
        
        SDL_Rect wheel1 = {static_cast<int>(x), GROUND_LEVEL - wheelRadius*2, wheelRadius*2, wheelRadius*2};
        SDL_Rect wheel2 = {static_cast<int>(x + width - wheelRadius*2), GROUND_LEVEL - wheelRadius*2, wheelRadius*2, wheelRadius*2};
        
        // Just draw wheel outlines
        SDL_RenderDrawRect(renderer, &wheel1);
        SDL_RenderDrawRect(renderer, &wheel2);
        
        // Frame
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255); // Red frame
        // Top bar
        SDL_Rect topBar = {static_cast<int>(x + wheelRadius), GROUND_LEVEL - height + 10, width - wheelRadius*2, 5};
        SDL_RenderFillRect(renderer, &topBar);
        
        // Down tube
        SDL_RenderDrawLine(renderer, 
                          static_cast<int>(x + wheelRadius), GROUND_LEVEL - height + 12,
                          static_cast<int>(x + wheelRadius*2), GROUND_LEVEL - wheelRadius);
        
        // Seat tube
        SDL_RenderDrawLine(renderer, 
                          static_cast<int>(x + width - wheelRadius*2), GROUND_LEVEL - height + 12,
                          static_cast<int>(x + width - wheelRadius*3), GROUND_LEVEL - wheelRadius);
        
        // Seat
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Dark gray
        SDL_Rect seat = {static_cast<int>(x + width - wheelRadius*2 - 5), GROUND_LEVEL - height + 5, 10, 5};
        SDL_RenderFillRect(renderer, &seat);
        
        // Handlebars
        SDL_Rect handlebar = {static_cast<int>(x + wheelRadius - 5), GROUND_LEVEL - height + 5, 10, 3};
        SDL_RenderFillRect(renderer, &handlebar);
    }
    
    void renderPuddle(SDL_Renderer* renderer) {
        // Puddle base
        SDL_SetRenderDrawColor(renderer, 50, 100, 180, 150); // Semi-transparent blue
        SDL_Rect puddle = {static_cast<int>(x), GROUND_LEVEL - 5, width, 5};
        SDL_RenderFillRect(renderer, &puddle);
        
        // Reflection
        SDL_SetRenderDrawColor(renderer, 150, 200, 255, 100); // Lighter blue
        for (int i = 0; i < 3; i++) {
            SDL_Rect reflection = {static_cast<int>(x + 5 + i*15), GROUND_LEVEL - 4, 10, 2};
            SDL_RenderFillRect(renderer, &reflection);
        }
    }
    
    void renderDog(SDL_Renderer* renderer) {
        // Body
        SDL_SetRenderDrawColor(renderer, 150, 120, 60, 255); // Brown
        SDL_RenderFillRect(renderer, &hitbox);
        
        // Head
        SDL_Rect head = {static_cast<int>(x + width - 20), GROUND_LEVEL - height - 10, 20, 20};
        SDL_RenderFillRect(renderer, &head);
        
        // Eyes
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
        SDL_Rect eye = {static_cast<int>(x + width - 12), GROUND_LEVEL - height - 5, 4, 4};
        SDL_RenderFillRect(renderer, &eye);
        
        // Ears
        SDL_SetRenderDrawColor(renderer, 120, 90, 40, 255); // Darker brown
        SDL_Rect ear = {static_cast<int>(x + width - 15), GROUND_LEVEL - height - 20, 10, 10};
        SDL_RenderFillRect(renderer, &ear);
        
        // Tail
        SDL_Rect tail = {static_cast<int>(x), GROUND_LEVEL - height - 5, 15, 5};
        SDL_RenderFillRect(renderer, &tail);
        
        // Legs
        for (int i = 0; i < 2; i++) {
            SDL_Rect leg = {static_cast<int>(x + 10 + i*(width-20)), GROUND_LEVEL - 15, 8, 15};
            SDL_RenderFillRect(renderer, &leg);
        }
    }
};

class ScoreManager {
private:
    unsigned int currentScore;
    unsigned int highScore;
    
public:
    ScoreManager() : currentScore(0), highScore(0) {
        loadHighScore();
    }
    
    void incrementJumpScore() {
        // Increment score specifically for jumps
        currentScore += 10; // Jumps give 10 points
        
        if (currentScore > highScore) {
            highScore = currentScore;
        }
    }
    
    unsigned int getCurrentScore() const {
        return currentScore;
    }
    
    unsigned int getHighScore() const {
        return highScore;
    }
    
    void reset() {
        // Save high score before resetting
        saveHighScore();
        currentScore = 0;
    }
    
    void loadHighScore() {
        std::ifstream file(HIGH_SCORE_FILE);
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
    }
    
    void saveHighScore() {
        std::ofstream file(HIGH_SCORE_FILE);
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
    
    ~ScoreManager() {
        saveHighScore();
    }
};

class TextManager {
private:
    TTF_Font* font;
    TTF_Font* largeFont;
    SDL_Color textColor;
    
public:
    TextManager() : font(nullptr), largeFont(nullptr) {
        textColor = {0, 0, 0, 255}; // Black
    }
    
    bool initialize() {
        if (TTF_Init() == -1) {
            std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return false;
        }
        
        // If arial.ttf doesn't exist in your project, you may need to adjust the path or use a different font
        font = TTF_OpenFont(FONT_FILE.c_str(), 24);
        if (!font) {
            // Try a fallback font if arial.ttf is not found
            font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
            if (!font) {
                std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
                std::cerr << "Using default rendering method instead." << std::endl;
            }
        }
        
        largeFont = TTF_OpenFont(FONT_FILE.c_str(), 28);
        if (!largeFont) {
            // Try a fallback font if arial.ttf is not found
            largeFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 28);
            if (!largeFont) {
                std::cerr << "Failed to load large font! SDL_ttf Error: " << TTF_GetError() << std::endl;
                std::cerr << "Using default rendering method instead." << std::endl;
            }
        }
        
        return true;
    }
    
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, bool useLargeFont = false) {
        TTF_Font* currentFont = useLargeFont ? largeFont : font;
        
        if (currentFont) {
            // Use SDL_ttf for text rendering
            SDL_Surface* textSurface = TTF_RenderText_Solid(currentFont, text.c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        } else {
            // Fallback to basic rendering if font couldn't be loaded
            renderBasicText(renderer, text, x, y);
        }
    }
    
    void renderBasicText(SDL_Renderer* renderer, const std::string& text, int x, int y) {
        // Simple fallback text rendering in case TTF font loading fails
        int charWidth = 10;
        int charHeight = 20;
        
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            if (c != ' ') {  // Don't render spaces
                SDL_Rect charRect = {x + static_cast<int>(i * charWidth), y, charWidth - 2, charHeight};
                
                // Different color for text
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &charRect);
            }
        }
    }
    
    ~TextManager() {
        if (font) {
            TTF_CloseFont(font);
            font = nullptr;
        }
        
        if (largeFont) {
            TTF_CloseFont(largeFont);
            largeFont = nullptr;
        }
        
        TTF_Quit();
    }
};

class CityBackground {
private:
    struct Building {
        int x;
        int width;
        int height;
        int color[3];  // RGB values
        std::vector<SDL_Rect> windows;
    };
    
    std::vector<Building> buildings;
    std::vector<SDL_Rect> clouds;
    int lastCloudTime;
    
public:
    CityBackground() : lastCloudTime(0) {
        initializeBuildings();
        initializeClouds();
    }
    
    void initializeBuildings() {
        // Create a skyline of buildings
        std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
        std::uniform_int_distribution<int> heightDist(100, 250);
        std::uniform_int_distribution<int> widthDist(60, 120);
        
        int x = 0;
        while (x < SCREEN_WIDTH * 1.5) {  // Create more buildings than needed to scroll
            Building building;
            building.x = x;
            building.width = widthDist(rng);
            building.height = heightDist(rng);
            
            // Random building color (grayish)
            building.color[0] = 100 + rand() % 80;
            building.color[1] = 100 + rand() % 80;
            building.color[2] = 100 + rand() % 80;
            
            // Add windows
            int windowSize = 12;
            int windowGap = 8;
            
            for (int row = 0; row < (building.height - 20) / (windowSize + windowGap); row++) {
                for (int col = 0; col < (building.width - 20) / (windowSize + windowGap); col++) {
                    SDL_Rect window = {
                        building.x + 10 + col * (windowSize + windowGap),
                        GROUND_LEVEL - building.height + 20 + row * (windowSize + windowGap),
                        windowSize, windowSize
                    };
                    
                    // 10% chance of light being off
                    if (rand() % 10 > 0) {
                        building.windows.push_back(window);
                    }
                }
            }
            
            buildings.push_back(building);
            x += building.width - 5;  // Slight overlap
        }
    }
    
    void initializeClouds() {
        // Create some initial clouds
        for (int i = 0; i < 3; i++) {
            SDL_Rect cloud = {
                rand() % SCREEN_WIDTH,
                20 + rand() % 60,
                40 + rand() % 60,
                15 + rand() % 15
            };
            clouds.push_back(cloud);
        }
    }
    
    void update(int gameSpeed) {
        // Move buildings
        for (auto& building : buildings) {
            building.x -= gameSpeed / 2;  // Parallax effect - buildings move slower than obstacles
            
            // Update window positions
            for (auto& window : building.windows) {
                window.x -= gameSpeed / 2;
            }
            
            // If building is off screen, move it to the right
            if (building.x + building.width < 0) {
                int maxX = 0;
                for (const auto& b : buildings) {
                    maxX = std::max(maxX, b.x + b.width);
                }
                
                int oldX = building.x;
                building.x = maxX - 5;  // Place after the rightmost building with slight overlap
                int xDiff = building.x - oldX;
                
                // Update window positions for the recycled building
                for (auto& window : building.windows) {
                    window.x += xDiff;
                }
                
                // Regenerate windows for variety
                building.windows.clear();
                int windowSize = 12;
                int windowGap = 8;
            
                for (int row = 0; row < (building.height - 20) / (windowSize + windowGap); row++) {
                    for (int col = 0; col < (building.width - 20) / (windowSize + windowGap); col++) {
                        SDL_Rect window = {
                            building.x + 10 + col * (windowSize + windowGap),
                            GROUND_LEVEL - building.height + 20 + row * (windowSize + windowGap),
                            windowSize, windowSize
                        };
                        
                        // 10% chance of light being off
                       // 10% chance of light being off
                        if (rand() % 10 > 0) {
                            building.windows.push_back(window);
                        }
                    }
                }
            }
        }
        
        // Move clouds
        for (auto& cloud : clouds) {
            cloud.x -= gameSpeed / 4;  // Clouds move even slower than buildings
            
            // If cloud is off screen, move it to the right
            if (cloud.x + cloud.w < 0) {
                cloud.x = SCREEN_WIDTH + rand() % 100;
                cloud.y = 20 + rand() % 60;
                cloud.w = 40 + rand() % 60;
                cloud.h = 15 + rand() % 15;
            }
        }
        
        // Maybe add a new cloud
        int currentTime = SDL_GetTicks();
        if (currentTime > lastCloudTime + 5000) {  // New cloud every 5 seconds
            if (rand() % 3 == 0) {  // 33% chance
                SDL_Rect cloud = {
                    SCREEN_WIDTH + rand() % 100,
                    20 + rand() % 60,
                    40 + rand() % 60,
                    15 + rand() % 15
                };
                clouds.push_back(cloud);
                lastCloudTime = currentTime;
            }
        }
    }
    
    void render(SDL_Renderer* renderer) {
        // Draw sky
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);  // Sky blue
        SDL_Rect sky = {0, 0, SCREEN_WIDTH, GROUND_LEVEL};
        SDL_RenderFillRect(renderer, &sky);
        
        // Draw clouds
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);  // White with slight transparency
        for (const auto& cloud : clouds) {
            SDL_RenderFillRect(renderer, &cloud);
            
            // Add some detail to clouds
            SDL_Rect cloudDetail = {cloud.x + cloud.w/4, cloud.y - cloud.h/2, cloud.w/2, cloud.h};
            SDL_RenderFillRect(renderer, &cloudDetail);
        }
        
        // Draw buildings
        for (const auto& building : buildings) {
            // Draw building
            SDL_SetRenderDrawColor(renderer, building.color[0], building.color[1], building.color[2], 255);
            SDL_Rect buildingRect = {building.x, GROUND_LEVEL - building.height, building.width, building.height};
            SDL_RenderFillRect(renderer, &buildingRect);
            
            // Draw windows (lights on)
            SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);  // Warm yellow light
            for (const auto& window : building.windows) {
                SDL_RenderFillRect(renderer, &window);
            }
        }
        
        // Draw ground
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Gray cement
        SDL_Rect ground = {0, GROUND_LEVEL, SCREEN_WIDTH, SCREEN_HEIGHT - GROUND_LEVEL};
        SDL_RenderFillRect(renderer, &ground);
        
        // Draw sidewalk
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);  // Light gray
        SDL_Rect sidewalk = {0, GROUND_LEVEL, SCREEN_WIDTH, 20};
        SDL_RenderFillRect(renderer, &sidewalk);
        
        // Draw road markers
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White
        for (int x = 0; x < SCREEN_WIDTH; x += 100) {
            SDL_Rect roadMarker = {x, GROUND_LEVEL + 40, 50, 10};
            SDL_RenderFillRect(renderer, &roadMarker);
        }
    }
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Player player;
    std::vector<Obstacle> obstacles;
    ScoreManager scoreManager;
    TextManager textManager;
    CityBackground background;
    bool isRunning;
    bool gameOver;
    int gameSpeed;
    unsigned int lastObstacleTime;
    int obstacleSpawnDelay;
    std::mt19937 rng;
    
public:
    Game() : window(nullptr), renderer(nullptr), isRunning(false), gameOver(false), 
            gameSpeed(GAME_SPEED_INITIAL), lastObstacleTime(0), obstacleSpawnDelay(2000) {
        
        // Initialize random number generator
        rng.seed(static_cast<unsigned int>(time(nullptr)));
    }
    
    bool initialize() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
         if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
        
        // Create window
        window = SDL_CreateWindow("City Runner", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                 SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create renderer
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // Initialize text manager
        if (!textManager.initialize()) {
            std::cerr << "Warning: Text manager initialization failed. Using fallback text rendering." << std::endl;
            // Continue anyway, will use fallback rendering
        }
        
        isRunning = true;
        resetGame();
        
        return true;
    }
    
    void resetGame() {
        // Reset game state
        obstacles.clear();
        player = Player();
        gameOver = false;
        gameSpeed = GAME_SPEED_INITIAL;
        lastObstacleTime = SDL_GetTicks();
        scoreManager.reset();
    }
    
    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                isRunning = false;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                    case SDLK_UP:
                        if (gameOver) {
                            resetGame();
                        } else {
                            player.jump();
                        }
                        break;
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                }
            }
        }
    }
    
    void update() {
        if (gameOver) {
            return;
        }
        
        // Update player
        player.update();
        
        // Check for jump scoring
        if (player.canScoreJump()) {
            scoreManager.incrementJumpScore();
        }
        
        // Speed up game based on score
        if (scoreManager.getCurrentScore() > 0 && scoreManager.getCurrentScore() % SPEED_UP_SCORE == 0) {
            gameSpeed += GAME_SPEED_INCREMENT;
            obstacleSpawnDelay = std::max(500, obstacleSpawnDelay - 100);  // Faster spawning, min 500ms
        }
        
        // Update background
        background.update(gameSpeed);
        
        // Update obstacles
        for (auto it = obstacles.begin(); it != obstacles.end();) {
            it->update(gameSpeed);
            
            // Check collision
            if (SDL_HasIntersection(&player.hitbox, &it->hitbox)) {
                gameOver = true;
                // Don't update further - keep the obstacle that caused the collision
                break;
            }
            
            // Remove off-screen obstacles
            if (it->isOffScreen()) {
                it = obstacles.erase(it);
            } else {
                ++it;
            }
        }
        
        // Spawn new obstacles
        unsigned int currentTime = SDL_GetTicks();
        if (currentTime > lastObstacleTime + obstacleSpawnDelay) {
            std::uniform_int_distribution<int> typeDist(0, OBSTACLE_TYPE_COUNT - 1);
            ObstacleType type = static_cast<ObstacleType>(typeDist(rng));
            
            int width, height;
            switch (type) {
                case COFFEE_CUP:
                    width = 30;
                    height = 40;
                    break;
                case BRIEFCASE:
                    width = 50;
                    height = 30;
                    break;
                case FIRE_HYDRANT:
                    width = 40;
                    height = 60;
                    break;
                case TRASH_CAN:
                    width = 45;
                    height = 70;
                    break;
                case CAR:
                    width = 100;
                    height = 60;
                    break;
                case BICYCLE:
                    width = 70;
                    height = 50;
                    break;
                case PUDDLE:
                    width = 80;
                    height = 5;
                    break;
                case DOG:
                    width = 60;
                    height = 40;
                    break;
                default:
                    width = 30;
                    height = 50;
                    break;
            }
            
            // Add randomness to spawn delay
            obstacles.emplace_back(SCREEN_WIDTH, width, height, type);
            lastObstacleTime = currentTime;
            
            // Randomize next obstacle time
            std::uniform_int_distribution<int> delayDist(-500, 500);
            int nextDelay = obstacleSpawnDelay + delayDist(rng);
            if (nextDelay < 500) nextDelay = 500;  // Minimum 500ms
        }
    }
    
    void render() {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        // Render background
        background.render(renderer);
        
        // Render player
        player.render(renderer);
        
        // Render obstacles
        for (auto& obstacle : obstacles) {
            obstacle.render(renderer);
        }
        
        // Render score
        std::string scoreText = "Score: " + std::to_string(scoreManager.getCurrentScore());
        textManager.renderText(renderer, scoreText, 10, 10);
        
        std::string highScoreText = "High Score: " + std::to_string(scoreManager.getHighScore());
        textManager.renderText(renderer, highScoreText, 10, 40);
        
        // Render game over text
        if (gameOver) {
            std::string gameOverText = "GAME OVER";
            textManager.renderText(renderer, gameOverText, SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 - 20, true);
            
            std::string restartText = "Press SPACE to restart";
            textManager.renderText(renderer, restartText, SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 20);
        }
        
        // Update screen
        SDL_RenderPresent(renderer);
    }
    
    void run() {
        const int FPS = 60;
        const int frameDelay = 1000 / FPS;
        
        Uint32 frameStart;
        int frameTime;
        
        while (isRunning) {
            frameStart = SDL_GetTicks();
            
            handleEvents();
            update();
            render();
            
            frameTime = SDL_GetTicks() - frameStart;
            
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
        }
    }
    
    void clean() {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        IMG_Quit();
        SDL_Quit();
    }
    
    ~Game() {
        clean();
    }
};

int main(int argc, char* args[]) {
    Game game;
    
    if (game.initialize()) {
        game.run();
    } else {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }
    
    return 0;
}
