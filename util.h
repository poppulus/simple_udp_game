#define SDL_MAIN_HANDLED

#define FPS 60
#define TICKS 1000 / FPS

#define W_WIDTH 640
#define W_HEIGHT 480

#define FONT_SIZE 16

#define EMPTY_TILE 0

#define F_WIDTH 64
#define F_HEIGHT 66
#define F_TILE 68
#define F_SPAWN_X 69
#define F_SPAWN_Y 71
#define F_BSIZE 73
#define MAP_START 74

#define STANDARD_VELOCITY 2.0F
#define SPRINT_VELOCITY 3.0F

#define JOYSTICK_DEADZONE 8000

#define AIM_RADIUS 50

#define AIM_RIGHT 315
#define AIM_DOWN  45
#define AIM_LEFT  135
#define AIM_UP    225

#define INPUT_UP   -1.570796F
#define INPUT_LEFT  3.141593F
#define INPUT_RIGHT 0
#define INPUT_DOWN  1.570796F

#define I_UP_LEFT  -2.356194F
#define I_UP_RIGHT -0.785398F
#define I_DOWN_LEFT  2.356194F
#define I_DOWN_RIGHT 0.785398F

#include "SDL_FontCache.h"
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>

enum PLAY_SOUND
{
    S_LOW,
    S_MEDIUM,
    S_HIGH,
    S_SCRATCH,
    S_BEAT
};

enum PLAY_TYPE
{
    P_TOPDOWN,
    P_TOPDOWN_SHOOT,
    P_PLATFORM
};

enum PLAY_STATE
{
    P_PLAY,
    P_SCORE
};

enum KEYBOARD_INPUTS
{
    KEY_DOWN = 1,
    KEY_UP,
    KEY_RIGHT,
    KEY_LEFT
};

enum PLAYER_STATE 
{
    STANDING, 
    FALLING,
    JUMPING
};

typedef struct Texture
{
    SDL_Texture *t;
    int w, h, a, pieces, t_h;
    Uint32 f;
} T;

typedef struct Level
{
    SDL_Rect r;

    char w_text[13];
    int t_size, t_map_h, t_bit_size;

    char h_text[14];
    int t_map_pieces;

    char t_text[9];
    unsigned char *map;
    unsigned char *collision;
    char *filename;

    char f_text[39];
} L;

typedef struct CROSSHAIR
{
    SDL_Rect r;
    bool show:1;
} X_HAIR;

typedef struct BULLET
{
    SDL_Rect r;
    float x, y, xvel, yvel;
    bool shoot;
} BUL;

typedef struct BULLET_HIT
{
    int x, y;
    bool used;
} B_HIT;

typedef struct BULLET_HITS
{
    B_HIT a[30];
    unsigned char index;
} B_HITS;

typedef struct Player
{
    T *texture;

    X_HAIR crosshair;

    int mx, my, rx, ry;

    short sprint_cdown_timer;

    float   x, y, xvel, yvel, 
            gvel, vel, pvel,
            INPUT_angle, AIM_angle,
            AIM_xvel, AIM_yvel, 
            AIM_xdir, AIM_ydir,
            AIM_radx, AIM_rady, AIM_deg,
            JOY_xdir, JOY_ydir, JOY_vel;

    SDL_Rect r, clip, *t_clips, club_r;

    bool    shoot:1, sprint:1, sprint_cdown:1, 
            swing:1, grab:1, spawned:1,
            JOY_use:1, AIM_done:1,
            m_move:1, m_hold:1;

    unsigned char *dir, input_q[4], facing,
                   a_counter, yvel_counter,
                   a_index, c_index,
                   sprint_timer, swing_timer, 
                   AIM_timer;

    enum PLAYER_STATE state;

    BUL bullets[10];
} P;

typedef struct Puck
{
    SDL_Rect r;
    bool hit:1;
    unsigned char hit_counter;
    float x, y, xvel, yvel, fvel;
} Puck;

typedef struct Play_Test
{
    Mix_Chunk **mix_chunks;

    T *gunTexture, texture;
    SDL_GameController *controller;
    L level;
    SDL_Rect gunClips[25];
    Puck puck;
    P *c_player; 

    SDL_Rect screen, camera, goal_r, sprint_hud_r, *t_clips;

    enum PLAY_TYPE style;
    enum PLAY_STATE state;

    unsigned char   GUN_delay, GUN_speed, SCORE_timer,
                    PUCK_freeze_timer,
                    t_n_size, t_bit_size, *m_buffer;

    char f_buffer[MAP_START];
    
    short channel_volume;

    bool PUCK_freeze:1, quit:1, w_focus:1;
    
    char TEXT_gun_speed[2];

    B_HITS bullet_hits;

    char TEXT_gun_delay[2]; 

    // make sure strings are separated with this byte/bit
    bool volume:1;
    
    char TEXT_channel_volume[3];
} P_TEST;

float lerp(float v0, float v1, float t);

void reverse(char s[]);
void n_itoa(int n, char s[]);

bool checkCollision(SDL_Rect a, SDL_Rect b);
bool checkPlayerPosition(int x, int y, unsigned char map[], int msize);
bool checkGoal(SDL_Rect puck, SDL_Rect goal);

SDL_Texture *loadTexture(SDL_Renderer *r, const char path[]);

bool loadMap(SDL_Renderer *r, P_TEST *pt, char str[]);

void closeSdl(SDL_Window **w, SDL_Renderer **r, Mix_Chunk *m[]);

void freeBuffers(P_TEST *pt);
void freePlayer(P p[]);
void freePlayTest(P_TEST *ptest);
void freeTexture(T *text);

bool initSdl(SDL_Window **w, SDL_Renderer **r);
bool initTexture(SDL_Renderer *r, T *texture, const char path[]);
bool initTextureMap(SDL_Renderer *r, P_TEST *pt, char *str);
void initLevel(P_TEST *pt);
void initMap(P_TEST *pt);
void initPlayer(P *p, L l, unsigned char b[]);
void initPlayerClips(SDL_Rect clips[]);
void initTiles(P_TEST *pt);

void setCamera(SDL_Rect *c, int x, int y);
void setMapDimensions(L *l, unsigned char *w, unsigned char *h, unsigned char s);

void setupPlay(P_TEST *pt, P *player);
void resetPlay(P_TEST *pt, P *player);

void resetPlayer(P *player);

void updateBulletHits(B_HITS *hits, int bx, int by);
void updateTopDownShoot(P_TEST *pt, P players[]);
void updateGame(P_TEST *p, P players[], Mix_Chunk chunks[]);
void updatePlayer(P *p, P_TEST *pt);

void renderGame(SDL_Renderer *r, FC_Font *f, P_TEST *p);

void playTopDownShooter(P_TEST *pt, SDL_Event ev);
bool playShootGun(P_TEST play, BUL bullets[], int sx, int sy, int dx, int dy);

void readInputs(SDL_Renderer *r, P_TEST *pt, SDL_Event event);

void playRender(SDL_Renderer *r, FC_Font *f, P_TEST *pt, P players[]);

void renderTiles(SDL_Renderer *r, P_TEST *pt);
void renderTexture(SDL_Renderer *r, T *t, SDL_Rect *clip, int x, int y, const double angle, const SDL_RendererFlip flip);
void renderPlayer(SDL_Renderer *r, SDL_Texture *t, P *p, int cx, int cy);
void renderPlayTiles(SDL_Renderer *r, P_TEST pt);

void animatePlayer(P *p);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);
