#include <raylib.h>
#include <iostream>

Color Background_c = {10, 20, 40, 255};   // #0A1428 
Color Paddle_c     = {64, 156, 255, 255}; // #409CFF
Color Ball_c       = {255, 221, 125, 255}; // #FFDD7D
Color Accent_c     = {40, 75, 99, 255};   // #284B63 
Color Circle_c     = {180, 200, 210, 255}; // #B4C8D2 —

int score{0};
int ai_score{0};

class Ball
{
public:

float x{}, y{};
int speed_x{}, speed_y{} , radius{}; 

void Draw()
{
 DrawCircle( x , y , radius , Ball_c);
}

void Update()
{
   x += speed_x;
   y += speed_y;


   if (y >= 780 || y <= 20)
    {
      speed_y *= -1;
    }
   if (x + radius >= 1270)
   {
    ++score;
    Reset_Ball();
   }
   else if (x - radius <= 10)
   {
    ++ai_score;
    Reset_Ball();
   }
  
}

void Reset_Ball()
{
   x = GetScreenWidth()/2;
   y = GetScreenHeight()/2;

   int directions[2] = {-1 , 1};
   speed_x *= directions[GetRandomValue(0,1)];
   speed_y *= directions[GetRandomValue(0,1)];
}

};

class Paddle
{
protected:

void LimitCheck()
{
   if (y  <= 0)
   {
    y = 0;
   }
   if (y + height >= 800)
   {
    y = 800 - height;
   }


}

public:

float x{} , y{};
float width{} , height{};
int speed{};


void Draw()
{
DrawRectangleRounded(Rectangle{x, y , width , height} , 0.8 , 0 , Paddle_c);
}

void Update()
{
   if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        y -= speed;
    }
   if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        y += speed;
    }  
   LimitCheck();
}

};

class AiPaddle : public Paddle
{
 public:

 void Update(int ball_y)
 {
  if (y + height/2 > ball_y)
  {
    y -= speed;
  } 
  else
  {
    y += speed;
  }
  LimitCheck();
 }


};




Ball ball;
Paddle player;
AiPaddle player2; // it was impossible to win on speed the same as player xd 

int main()
{

   constexpr int width{1280};
   constexpr int height{800};
   SetTargetFPS(60);
   InitWindow(width , height , "Pong Game");
   
   ball.radius = 20;
   ball.x = width/2;
   ball.y = height/2;
   ball.speed_x = 7;
   ball.speed_y = 7;

   player.x = 10;
   player.y = 340;
   player.width = 25;
   player.height = 120;
   player.speed = 10;

   player2.x = 1245;
   player2.y = 340;
   player2.width = 25;
   player2.height = 120;
   player2.speed = 5;

   
   while(WindowShouldClose() == false)
   {
    
    //Drawing 
    
    BeginDrawing();
    ClearBackground(Background_c);
    DrawRectangle(width/2 , 0 , width /2 , height , Accent_c);
    DrawCircle(width/2 , height/2 , 150 , Circle_c);
    ball.Draw();
    player.Draw();
    player2.Draw();
    DrawLine(width/2 , 0 , width/2 , 800 , WHITE);
    DrawText( TextFormat("%i" , score) , 300 , 20 , 80, WHITE);
    DrawText( TextFormat("%i" , ai_score) , 900 , 20 , 80, WHITE);

    EndDrawing();
    
    //Simple Collision check 
    if (CheckCollisionCircleRec(Vector2{ball.x , ball.y} , ball.radius , Rectangle{player.x , player.y , player.width , player.height}))
    {
        ball.speed_x *= -1;
    }

    if (CheckCollisionCircleRec(Vector2{ball.x , ball.y} , ball.radius , Rectangle{player2.x , player2.y , player2.width , player2.height}))
    {
        ball.speed_x *= -1;
    }

    
    //Update
    ball.Update();
    player.Update();
    player2.Update(ball.y);  
   }
   

   CloseWindow();
   return 0;
}
