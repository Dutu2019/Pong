#include "main.h"
#include "tinyttf.h"
#include <unistd.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 800

#define PLAYER_WIDTH 7
#define PLAYER_HEIGHT 100
#define BALL_SIZE 15
#define BALL_SPEED 700.0f

#define FRAME_RATE 300.0f
#define CONTROL_SPEED 600.0f

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL;
static SDL_Texture *texture = NULL;

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Time time;
    SDL_GetCurrentTime(&time);
    SDL_srand(time);
    window = SDL_CreateWindow("Pong", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialise SDL_ttf: %s\n", SDL_GetError());
        TTF_Quit();
    }

    font = TTF_OpenFontIO(SDL_IOFromConstMem(tiny_ttf, tiny_ttf_len), true, 18.0f);
    if (!font) {
        SDL_Log("Couldn't open font: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init players and ball
    SDL_FRect player1, player2;
    initPlayers(&player1, &player2);
    Ball ball;
    initBall(&ball);

    bool is_running = true;
    while (is_running) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                is_running = false;
            }
        }
        updatePos(&player1, &player2, &ball);
        checkCollisions(&player1, &player2, &ball);
        renderObjects(renderer, &player1, &player2, &ball);
        SDL_Delay(1000/FRAME_RATE);
    }

    return 0;
}

bool updateScore() {
    SDL_Surface *text = TTF_RenderText_Blended(font, "Hello World!", 0, (SDL_Color) {255, 255, 255, 255});
    if (text) {
        texture = SDL_CreateTextureFromSurface(renderer, text);
        SDL_DestroySurface(text);
    }
    if (!texture) {
        SDL_Log("Couldn't create text: %s\n", SDL_GetError());
        return false;
    }

    SDL_FRect *rect;
    rect->x = WINDOW_WIDTH/2;
    rect->y = 10;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, rect);
    SDL_RenderPresent(renderer);
}

void initBall(Ball *ball) {
    float angle = SDL_PI_F*SDL_roundf(SDL_randf());
    ball->rect = (SDL_FRect){WINDOW_WIDTH/2, WINDOW_HEIGHT/2, BALL_SIZE, BALL_SIZE};
    ball->xspeed = BALL_SPEED*SDL_cosf(angle);
    ball->yspeed = BALL_SPEED*SDL_sinf(angle);
}

void initPlayers(SDL_FRect *player1, SDL_FRect *player2) {
    *player1 = (SDL_FRect) {0, (WINDOW_HEIGHT-PLAYER_HEIGHT)/2, PLAYER_WIDTH, PLAYER_HEIGHT};
    *player2 = (SDL_FRect) {WINDOW_WIDTH-PLAYER_WIDTH, (WINDOW_HEIGHT-PLAYER_HEIGHT)/2, PLAYER_WIDTH, PLAYER_HEIGHT};
}

void changeBallDirection(Ball *ball, float angle) {
    ball->xspeed = BALL_SPEED*SDL_cosf(angle);
    ball->yspeed = BALL_SPEED*SDL_sinf(angle);
}

void renderObjects(SDL_Renderer *renderer, SDL_FRect *player1_rect, SDL_FRect *player2_rect, Ball *ball) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    // Render players
    SDL_RenderFillRect(renderer, player1_rect);
    SDL_RenderFillRect(renderer, player2_rect);
    // Render ball
    SDL_RenderFillRect(renderer, &ball->rect);

    // Render the frame
    SDL_RenderPresent(renderer);
}

void updatePos(SDL_FRect *player1, SDL_FRect *player2, Ball *ball) {
    const bool *key_states = SDL_GetKeyboardState(NULL);
    // Players
    if (key_states[SDL_SCANCODE_W]) player1->y = MAX(0.0f, player1->y-CONTROL_SPEED/FRAME_RATE);
    if (key_states[SDL_SCANCODE_S]) player1->y = MIN(WINDOW_HEIGHT-PLAYER_HEIGHT, player1->y+CONTROL_SPEED/FRAME_RATE);
    if (key_states[SDL_SCANCODE_UP]) player2->y = MAX(0.0f, player2->y-CONTROL_SPEED/FRAME_RATE);
    if (key_states[SDL_SCANCODE_DOWN]) player2->y = MIN(WINDOW_HEIGHT-PLAYER_HEIGHT, player2->y+CONTROL_SPEED/FRAME_RATE);
    // Ball
    ball->rect.x += ball->xspeed/FRAME_RATE;
    ball->rect.y += ball->yspeed/FRAME_RATE;
    if (ball->xspeed < 0.0f) ball->rect.x = MAX(0.0f, ball->rect.x);
    if (ball->xspeed > 0.0f) ball->rect.x = MIN((float)WINDOW_WIDTH-BALL_SIZE, ball->rect.x);
    if (ball->yspeed < 0.0f) ball->rect.y = MAX(0.0f, ball->rect.y);
    if (ball->yspeed > 0.0f) ball->rect.y = MIN((float)WINDOW_HEIGHT-BALL_SIZE, ball->rect.y);
}

void checkCollisions(SDL_FRect *player1, SDL_FRect *player2, Ball *ball) {
    // top and bottom walls
    if (ball->rect.y <= 0 || ball->rect.y >= WINDOW_HEIGHT-BALL_SIZE) ball->yspeed = -ball->yspeed;
    // left and right walls
    if (ball->rect.x <= 0 || ball->rect.x >= WINDOW_WIDTH-BALL_SIZE) {
        SDL_Delay(1000);
        initBall(ball);
        initPlayers(player1, player2);
    }

    // player1-ball
    if (SDL_HasRectIntersectionFloat(player1, &ball->rect)) {
        float diff = player1->y + PLAYER_HEIGHT/2 - ball->rect.y;
        float new_angle = -SDL_PI_F * diff/(2*PLAYER_HEIGHT);
        changeBallDirection(ball, new_angle);
    }
    // player2-ball
    if (SDL_HasRectIntersectionFloat(player2, &ball->rect)) {
        float diff = player2->y + PLAYER_HEIGHT/2 - ball->rect.y;
        float new_angle = SDL_PI_F * (diff/(2*PLAYER_HEIGHT) + 1);
        changeBallDirection(ball, new_angle);
    }
}