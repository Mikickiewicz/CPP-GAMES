#include <raylib.h>
#include <iostream>
#include <string>
#include "Random.h"
#include <vector>



Texture2D texture_background;
Texture2D texture_base;
Texture2D texture_pipe_green;
Texture2D birds[3];
Texture digits[10];
Sound wing;
Sound point;
Sound die;
Sound hit;


class Bird
    {
    public:
        Texture2D birds[3];
        Vector2 pos;
        float velocity;
        float gravity{0.5f};
        float jumpForce{-8.0f};
        float rotation{0};
        int frame;
        int frameCounter;
        int screenHeight;
        int floorHeight;

        //constructor
        Bird(Vector2 startPos, Texture2D b[3] , int screenH, int floorH)
        {
        pos = startPos;
        velocity = 0;
        frame = 0;
        frameCounter = 0;
        screenHeight = screenH; 
        floorHeight = floorH;
        for(int i{0}; i<3; i++) birds[i] = b[i];
        }
    
    //animate wings
    void Update() 
    {
        frameCounter++;           // Every 5 frames change type of wings  
        if (frameCounter > 5) 
        {
            frame = (frame + 1) % 3;
            frameCounter = 0;    
        }

        velocity += gravity;
        pos.y += velocity;
        

        rotation = velocity * 5;          // Rotate only when falling or rising 
        if (rotation > 45) rotation = 45;
        if (rotation < -25) rotation = -25;


        if (pos.y >  screenHeight - floorHeight - birds[1].height ) // floor lock in order no to go under base
        {
        pos.y = screenHeight - floorHeight - birds[1].height;
        velocity = 0;
        }

        if (pos.y < 0) // the same but up
        {
        pos.y = 0;
        velocity = 0;
        }
        
    }
        
    void Draw() 
    {
        
        //this shit takes only float so i need to convert everything to it

        Rectangle source     // Part of the texture we want to draw (what we cut from the image)
        { 
        0.0f,
        0.0f,
        static_cast<float>(birds[frame].width),
        static_cast<float>(birds[frame].height)
        };

        
        Rectangle dest       // Where and how big we draw the texture on screen
        {
        pos.x + static_cast<float>(birds[frame].width) / 2.0f,
        pos.y + static_cast<float>(birds[frame].height) / 2.0f,
        static_cast<float>(birds[frame].width),   
        static_cast<float>(birds[frame].height)
        };

        Vector2 origin{    // Point around which the texture rotates (pivot)
        static_cast<float>(birds[frame].width) / 2.0f,
        static_cast<float>(birds[frame].height) / 2.0f
        };
   

        
        DrawTexturePro(birds[frame] , source , dest , origin , rotation , WHITE );
    }   
    
    };
    
    
class PipePair
{
public:
    Texture2D texture_pipe_green;
    int y_bottom; // Y position (top) of the bottom pipe (coordinate from top of screen)
    int y_top;    // Y position (top) of the top pipe
    int gap{200}; // gap between the πiπes 
    int visibleMargin{80}; // to see the pipe
    int x{400};
    bool passed{false};

    // Generates a random vertical placement for the pipe pair.
    void GenerateHeight()
    {
        int H = GetScreenHeight();
        int B = 162;                             // floor height
        int P = texture_pipe_green.height;       // pipe texture height
        int G = gap;
        int V = visibleMargin;
        int minY_bottom = V + G;                 // lowest (closest to top) top of bottom pipe allowed
        int maxY_bottom = (H - B) - V;           // highest (closest to floor) top of bottom pipe allowed
     
        
        // Randomly pick bottom pipe top coordinate in the feasible range.
        y_bottom = Random::get(minY_bottom, maxY_bottom);
        // /std::cout <<  minY_bottom << "\n"<< maxY_bottom << "\n" << y_bottom << "\n";
        // Calculate top pipe top coordinate so that gap is exactly G (top is above bottom)
        y_top = y_bottom - G - P;

        // If top pipe would be too high (less than V px visible), move both pipes down
        // so the top pipe shows at least V px
        if (y_top + P < V)
        {
            int neededShift = V - (y_top + P); // positive amount to shift down
            y_top += neededShift;
            y_bottom += neededShift;

            // Clamp bottom so it doesn't go past maxY_bottom
            if (y_bottom > maxY_bottom)
            {
                // if it would, clamp and recompute top accordingly
                y_bottom = maxY_bottom;
                y_top = y_bottom - G - P;
            }
        }

        // Final safety: ensure bottom is not below floor-visible limit and top keeps at least some visibility
        if (y_bottom > maxY_bottom) y_bottom = maxY_bottom;
        if (y_bottom < minY_bottom) y_bottom = minY_bottom;
        y_top = y_bottom - G - P;
    }

    // Draw a vertically flipped texture (for top pipe)
    void DrawTextureReversed(Texture2D tex, int x, int y, Color color)
    {
        DrawTexturePro(
            tex,
            {0, 0, static_cast<float>(tex.width), -static_cast<float>(tex.height)}, // flip vertically
            {static_cast<float>(x), static_cast<float>(y), static_cast<float>(tex.width), static_cast<float>(tex.height)},
            {0, 0},
            0.0f,
            color
        );
    }

    // Draw both pipes
    void Draw()
    {
        DrawTexture(texture_pipe_green, x, y_bottom, WHITE);
        DrawTextureReversed(texture_pipe_green, x, y_top, WHITE);
    }

    void move(float speed)
    {
     x -= static_cast<int>(speed);
    }
    bool IsOfScreen() const 
    {
    return x + texture_pipe_green.width < 0;

    }


};

class PipeManager
{
public:
std::vector<PipePair>pipes;
Texture2D pipeTexture;

//constructor 
PipeManager(Texture2D texture = texture_pipe_green)
{
pipeTexture = texture;
InIt();
}

void InIt()
{
pipes.clear();
for (int i{0} ; i <3 ; ++i)
{
PipePair p;
p.texture_pipe_green = pipeTexture;
p.x = 600 + i * 300;
p.GenerateHeight();
pipes.push_back(p);
}
}

void update(float speed)
{
for (int i{0} ; i < pipes.size() ; i++)
{
pipes[i].move(speed);
}

if (pipes.front().IsOfScreen())
{
PipePair newPair;
newPair.texture_pipe_green = pipeTexture;
newPair.x = pipes.back().x + 300;

newPair.GenerateHeight();

pipes.erase(pipes.begin()); 
pipes.push_back(newPair);
}

}
};


class GameManager
{
public:
    Bird player;           // bird object
    PipeManager pipeMgr;   // pipe manager object
    bool gameOver{false};
    bool diePlayed{false};
    bool hitPlayed{false};
    bool hitGround{false};
    float floorSpeed{2.0f};
    float scrollX{0};
    int screenWidth;
    int screenHeight;
    int floorHeight;
    int score{0};

    //constructor
    GameManager(Vector2 birdStartPos, Texture2D b[3], int screenW, int screenH, int floorH, Texture2D pipeTex)
        : player(birdStartPos, b, screenH, floorH), pipeMgr(pipeTex), score(0), gameOver(false),
          screenWidth(screenW), screenHeight(screenH), floorHeight(floorH)
    {
    }

    // Update game logic
    void Update()
    {
        if (!gameOver)
        {
            // Bird physics
            player.Update();

            // Move pipes
            pipeMgr.update(floorSpeed);

            // Check collisions
            if (checkCollision())
            {
                gameOver = true;

                // Play hit sound only once
                if (!hitPlayed)
                {
                    PlaySound(hit);
                    hitPlayed = true;
                }

                // If the bird hit the ground, play only "hit"
                if (hitGround)
                {
                    diePlayed = true;
                }
                else if (!diePlayed)
                {
                    // Play "die" right after "hit"
                    diePlayed = true;
                    PlaySound(die);
                }
            }


            // Update score
            updateScore();
        }
        else
        {
         if (player.pos.y < screenHeight - floorHeight - player.birds[0].height)
         {
            player.velocity += player.gravity;
            player.pos.y += player.velocity;
            player.rotation = player.velocity * 5;
            if (player.rotation > 45) player.rotation = 45;
         }
         else
         {
            player.pos.y = screenHeight - floorHeight - player.birds[0].height;
            player.velocity = 0;
         }
        }
    } 

    // Draw everything
    void Draw()
    {   
        // Draw pipes
        for (int i{0}; i < pipeMgr.pipes.size(); i++)
        {
            pipeMgr.pipes[i].Draw();
        }

        if (!gameOver) scrollX += floorSpeed; 
        if (scrollX >= texture_base.width) scrollX = 0;

        //Magic moving base  
        // Select the part of the texture we want to draw
        Rectangle source = 
        { 
        scrollX,                   // X in the texture – horizontal offset for scrolling background
        0,                         // Y in the texture – start from the top
        static_cast<float>(screenWidth),        // Width of the fragment to draw
        static_cast<float>(floorHeight) // Height of the fragment (full texture height)
        };

        // Define where and at what size the texture fragment will be drawn on the screen
        Rectangle dest =
        { 
        0,                          // X on screen – from the left edge
        static_cast<float>(screenHeight) - static_cast<float>(floorHeight), // Y on screen – distance from the top (draws "floor" at bottom)
        static_cast<float>(screenWidth),         // Width on screen – can scale the fragment
        static_cast<float>(floorHeight)  // Height on screen – can scale the fragment
        };

        // Draw the selected fragment of the texture at the specified position and size
        DrawTexturePro
        (
        texture_base,      // Texture to draw
        source,            // Part of the texture to use
        dest,              // Where and at what size to draw it on screen
        Vector2{0 , 0 },   // Rotation/scaling origin point
        0,                 // Rotation angle in degrees 
        WHITE              // Color tint (WHITE = normal, no tint)
        );

        

        // Draw bird
        player.Draw();
        //Draw score
        DrawScore();
    }

private:
    bool checkCollision()
    {
    Rectangle birdRec;
    birdRec.x = static_cast<float>(player.pos.x);
    birdRec.y = static_cast<float>(player.pos.y);
    birdRec.width = static_cast<float>(player.birds[0].width);
    birdRec.height = static_cast<float>(player.birds[0].height);

    // Check collision with pipes
    for (int i{0}; i < pipeMgr.pipes.size(); i++)
    {
        PipePair pipe = pipeMgr.pipes[i];
        Rectangle bottomRec{
            static_cast<float>(pipe.x),
            static_cast<float>(pipe.y_bottom),
            static_cast<float>(pipe.texture_pipe_green.width),
            static_cast<float>(pipe.texture_pipe_green.height)
        };

        Rectangle topRec{
            static_cast<float>(pipe.x),
            static_cast<float>(pipe.y_top),
            static_cast<float>(pipe.texture_pipe_green.width),
            static_cast<float>(pipe.texture_pipe_green.height)
        };

        if (CheckCollisionRecs(birdRec, bottomRec) || CheckCollisionRecs(birdRec, topRec))
        {
            hitGround = false; // hit pipe
            return true;
        }
    }

    // Check collision with ground
    if (player.pos.y + player.birds[0].height >= player.screenHeight - player.floorHeight)
    {
        hitGround = true; // hit the ground
        return true;
    }

    // Check collision with top of the screen
    if (player.pos.y <= 0)
    {
        hitGround = false; // hit ceiling
        return true;
    }

    return false;
    }

    void updateScore()
    {
        PipePair &pipe = pipeMgr.pipes[0]; 
        int bird_x = static_cast<int>(player.pos.x) + player.birds[0].width / 2;
        int pipe_x = pipe.x + pipe.texture_pipe_green.width / 2;

        if (!pipe.passed && bird_x >= pipe_x)
        {
            score += 1;
            PlaySound(point);
            pipe.passed = true;
        }
    }
    void DrawScore()
    {
    std::string scoreStr = std::to_string(score);                        // convert score to string
    float scale = 2.0f;                                                  // double size
    int digitWidth = digits[0].width * scale;                            // width of one digit after scaling
    int startX = screenWidth / 2 - (scoreStr.length() * digitWidth) / 2; // center the score
    int y{50};                                                           // Y position on the screen

    for (int i = 0; i < scoreStr.length(); i++)
    {
        int digit = scoreStr[i] - '0';                                   // convert character to integer
        DrawTextureEx(digits[digit], Vector2{static_cast<float>(startX + i * digitWidth), static_cast<float>(y)}, 0.0f, scale, WHITE);
    }
    }
};



void load_assets(int screenWidth , int screenHeight , int floorHeight)
{
    //sounds
    wing = LoadSound("assets/audio/wing.wav");
    point = LoadSound("assets/audio/point.wav");
    die = LoadSound("assets/audio/die.wav");
    hit = LoadSound("assets/audio/hit.wav");

    // background
    Image image = LoadImage("assets/graphics/background-day.png");
    ImageResize(&image, screenWidth, screenHeight); 
    texture_background = LoadTextureFromImage(image);

    //base
    Image image2 = LoadImage("assets/graphics/base.png");
    ImageResize(&image2 , screenWidth , floorHeight);
    texture_base = LoadTextureFromImage(image2);
    

    //bird 
    // array of file paths (each string points to a different bird image)
    std::string birdFiles[3] = {
        "assets/graphics/bluebird-downflap.png",
        "assets/graphics/bluebird-midflap.png",
        "assets/graphics/bluebird-upflap.png"
    };

    // load all bird textures
    for (int i = 0; i < 3; i++)
    {
        Image img = LoadImage(birdFiles[i].c_str());       // load image from file (convert std::string to const char*)
        ImageResize(&img, img.width * 2, img.height * 2);  // resize image to 2x (double size)
        birds[i] = LoadTextureFromImage(img);              // create texture from image (send to GPU)
        UnloadImage(img);                                  // free image from RAM (we only keep texture)
    }

    // number 0-9
    for (int i{0}; i <= 9; i++)
    {
        std::string path = "assets/graphics/" + std::to_string(i) + ".png";
        digits[i] = LoadTexture(path.c_str());
    }
                                            
    
    //pipe 
    Image image3 = LoadImage("assets/graphics/pipe-green.png");
    ImageResize(&image3 , image3.width * 2 , image3.height*2);
    texture_pipe_green = LoadTextureFromImage(image3);
    
    UnloadImage(image);
    UnloadImage(image2);
    UnloadImage(image3);

}

void unload_assets()
{
    UnloadSound(die);
    UnloadSound(wing);
    UnloadSound(point);
    UnloadSound(hit);

    UnloadTexture(texture_background);
    UnloadTexture(texture_base);
    UnloadTexture(texture_pipe_green);
    
    for (int i{0} ; i < 3 ; ++i)
      UnloadTexture(birds[i]);

    for (int i{0}; i <= 9; i++)
        UnloadTexture(digits[i]);
}





int main()
{
    constexpr int screenWidth{800};
    constexpr int screenHeight{1024};
    constexpr int floorHeight{162};

    // screenHeight - floorHeight
    InitWindow(screenWidth, screenHeight, "Flappy Bird");
    InitAudioDevice();
    SetTargetFPS(60);
    load_assets(screenWidth , screenHeight ,floorHeight);
    
    GameManager game(
    {screenWidth / 4.0f, screenHeight / 3.5f}, 
    birds, 
    screenWidth, 
    screenHeight, 
    floorHeight, 
    texture_pipe_green
    );

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);
        //start drawing 
        BeginDrawing();
        //background
        DrawTexture(texture_background , 0 , 0 , WHITE);
      
        if (!game.gameOver && IsKeyPressed(KEY_SPACE))
        {
        game.player.velocity = game.player.jumpForce;
        PlaySound(wing);
        }
        
        //it handles everything
        game.Update();
        game.Draw();
        
        
        EndDrawing();
    }

    
    unload_assets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
