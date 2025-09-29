#ifndef MAIN_H
#define MAIN_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    Uint32 lastTime;
    int frameCount;
    float fps;
} SimpleFPS;

typedef struct {
    SDL_FRect rect;
    float xspeed, yspeed;
} Ball;

void renderObjects(SDL_Renderer *renderer, SDL_FRect *player1_rect, SDL_FRect *player2_rect, Ball *ball);
void updatePos(SDL_FRect *player1, SDL_FRect *player2, Ball *ball);
void initBall(Ball *ball);
void initPlayers(SDL_FRect *player1, SDL_FRect *player2);
void changeBallDirection(Ball *ball, float angle);
void checkCollisions(SDL_FRect *player1, SDL_FRect *player2, Ball *ball);

#endif