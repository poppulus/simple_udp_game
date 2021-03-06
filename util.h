#define SDL_MAIN_HANDLED

#define AIM_PI 3.14159265358979323846

#define FPS 60
#define TICKS 1000 / FPS

#define W_WIDTH 640
#define W_HEIGHT 480

#define MAX_GAME_USERS 4
#define GOALS 2

#define FONT_SIZE 16

#define AUDIO_SAMPLE_COUNT 5

#define EMPTY_TILE 0

#define F_WIDTH 64
#define F_HEIGHT 66
#define F_TILE 68
#define F_SPAWN_X 69
#define F_SPAWN_Y 71
#define F_BSIZE 73
#define MAP_START 74

#define STANDARD_VELOCITY 2.5F
#define SPRINT_VELOCITY 3
#define MIN_SHOOT_VELOCITY 3
#define MAX_SHOOT_VELOCITY 5

#define JOYSTICK_DEADZONE 8000
#define JOY_ID_NULL -1

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

#include "net.h"

enum PLAY_SOUND
{
    S_LOW,
    S_MEDIUM,
    S_HIGH,
    S_SCRATCH,
    S_BEAT
};

enum PLAY_STATE
{
    P_DROP,
    P_PLAY,
    P_GOAL
};

enum GAME_STATE 
{
    G_MAIN,
    G_LOADING,
    G_PLAY,
    G_PLAY_NET,
    G_MENU,
    G_HOST,
    G_JOIN,
    G_HOSTING,
    G_JOINING,
    G_CONFIRM
};

enum PUCK_STATE
{
    P_STATE_NORMAL,
    P_STATE_HIT,
    P_STATE_GRAB
};

enum PLAYER_STATE 
{
    PLR_SKATE_NP,
    PLR_SKATE_WP,
    PLR_SHOOT,
    PLR_SHOOT_MAX,
    PLR_SWING,
    PLR_SPRINT,
    PLR_BLOCK
};

enum GOALIE_STATE
{
    GK_NORMAL, 
    GK_CLEAR_GOAL,
    GK_SHOOT,
    GK_GRAB
};

enum PLAYER_FACING
{
    FACING_DOWN, 
    FACING_UP, 
    FACING_RIGHT, 
    FACING_LEFT
};

enum KEYBOARD_INPUTS
{
    KEY_DOWN = 1,
    KEY_UP,
    KEY_RIGHT,
    KEY_LEFT
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

typedef struct PLAY_SCORE
{
    unsigned char score1;
    char score1_string[3];
    unsigned char score2;
    char score2_string[3];
} P_SCORE;

typedef struct PLAY_GOALIE
{
    SDL_Rect r;
    enum GOALIE_STATE state;
    float x, y;
    unsigned char s_timer, id;
} P_G;

typedef struct Player
{
    T *texture;
    X_HAIR crosshair;

    enum PLAYER_STATE state;
    enum PLAYER_FACING facing;

    int     id, ctrl_id, mx, my;

    short   sprint_cdown_timer;

    float   x, y, xvel, yvel, 
            gvel, vel, pvel,
            INPUT_angle, AIM_angle,
            AIM_xvel, AIM_yvel, 
            AIM_xdir, AIM_ydir,
            AIM_radx, AIM_rady, AIM_deg,
            JOY_xdir, JOY_ydir, JOY_vel;

    SDL_Rect r, clip, *t_clips, club_r;

    bool    sprint:1, sprint_cdown:1, spawned:1, bounce:1, state_wait:1,
            JOY_use:1, AIM_done:1,
            m_move:1, m_hold:1;

    unsigned char *dir, input_q[4], aim_q[4],
                   a_counter, a_index, c_index,
                   sprint_timer, swing_timer, block_timer,
                   state_timer, AIM_timer;
} P;

typedef struct Puck
{
    SDL_Rect r;
    unsigned char hit_counter, state;
    float x, y, xvel, yvel, fvel, fvelx, fvely;
} Puck;

typedef struct INPUT_FIELD
{
    char string[64];
    unsigned char str_len, str_pointer;
} I_FIELD;

typedef struct Play_Test
{
    Mix_Chunk **mix_chunks;
    T texture;
    SDL_GameController *controllers[4];
    L level;
    P_SCORE score;
    Puck puck;
    P_G *goalie;
    P *c_player, *players; 
    I_FIELD input_field;
    NET network;

    SDL_Rect screen, camera, sprint_hud_r, 
            *goal_r, *t_clips, *gk_r, *buttons, *plr_hud;

    enum PLAY_STATE p_state;
    enum GAME_STATE g_state;

    int rx, ry;

    char state_timer_string[2];

    unsigned char   JOY_select, SCORE_timer, PUCK_freeze_timer, STATE_timer, STATE_cntdwn,
                    t_n_size, t_bit_size, *m_buffer, f_buffer[MAP_START];

    char channel_volume;

    bool PUCK_freeze:1, quit:1, w_focus:1, is_net:1, state_change:1, playing:1, menu_hold:1;

} P_TEST;

float lerp(float v0, float v1, float t);

void reverse(char s[]);
void n_itoa(int n, char s[]);

bool checkCollision(SDL_Rect a, SDL_Rect b);
bool checkMousePosition(int x, int y, SDL_Rect r);
bool checkPlayerPosition(int x, int y, unsigned char map[], int msize);
bool checkGoal(SDL_Rect puck, SDL_Rect goal);
bool checkPuckCollision(float x, float y, SDL_Rect box);

bool checkIpString(const char string[]);

void checkPuckXDistance(float x1, float x2, float *f);

void checkControllers(SDL_GameController *controllers[]);
void removeController(SDL_GameController *controllers[], int id, P plrs[]);

SDL_Texture *loadTexture(SDL_Renderer *r, const char path[]);

bool loadMap(SDL_Renderer *r, P_TEST *pt, char str[]);

void closeSdl(SDL_Window **w, SDL_Renderer **r, Mix_Chunk *m[]);

void freeBuffers(P_TEST *pt);
void freePlayer(P p[]);
void freePlayTest(P_TEST *ptest);
void freeTexture(T *text);

bool initSdl(SDL_Window **w, SDL_Renderer **r);
bool initAudio(Mix_Chunk **chunks);
bool initTexture(SDL_Renderer *r, T *texture, const char path[]);
bool initTextureMap(SDL_Renderer *r, P_TEST *pt, char *str);
void initLevel(P_TEST *pt);
void initMap(P_TEST *pt);
void initPlayer(P *p, L l, unsigned char b[]);
void initPlayerClips(SDL_Rect clips[]);
void initTiles(P_TEST *pt);
void initButtons(SDL_Rect *r);

void initGoalkeeper(P_G *g);

void setCamera(SDL_Rect *c, int x, int y);
void setMapDimensions(L *l, unsigned char *w, unsigned char *h, unsigned char s);

void setPlayerFace(float deg, enum PLAYER_FACING *face);

void setupPlay(P_TEST *pt, P *player);
void setupGame(P_TEST *pt, SDL_Rect *gr, SDL_Rect *gkr, P_G *gkeep);
void setupGoals(SDL_Rect *r);
void setupGoalKeepers(SDL_Rect *r, P_G *gk);
void setupPlayerHud(SDL_Rect r[]);

void addPlayerGame(P *p, unsigned char id, int x, int y);
void removePlayerGame(P *p);

void resetPlay(P_TEST *pt, P plrs[], bool id);
void resetPlayer(P *player, int sx, int sy);
void resetPuck(Puck *p, int mx, int my);
void resetInputField(I_FIELD *input);
void resetScores(P_SCORE *scores);
void resetGoalkeeper(P_G *g, int x);

void adjustGoalieX(float *gkx, float rx, float vx);
void adjustGoalieY(float *gky, float ry, float vy);

void updateCamera(SDL_Rect *camera, SDL_Rect level, int rx);

void updateGame(P_TEST *pt, P players[]);
void updateGameMain(P_TEST *pt);
void updateGameLoading(P_TEST *pt);
void updateGamePlay(P_TEST *pt, P players[]);
void updateGamePlayNet(P_TEST *pt);
void updateGameMenu(P_TEST *pt);
void updateGameHost(P_TEST *pt);
void updateGameJoin(P_TEST *pt);
void updateGameHosting(P_TEST *pt);
void updateGameJoining(P_TEST *pt);

void updatePlayerInputs(P *p);
void updatePlayerX(P *p, L level);
void updatePlayerY(P *p, L level);

void updateKeyInputs(P *p);
void updateJoyInputs(P *p);

void addPlayerVel(float *vel, float wave);
void subPlayerVel(float *vel);

void updatePuck(P_TEST *pt, P players[]);

void updateGoalKeeperX(float *x, const int CHECKX);
void updateGoalKeepers(P_TEST *pt, P players[], bool grab);

void updateNetGame(P_TEST *pt, P plrs[]);
void updateNetPlayers(P_TEST *pt, P plrs[]);

void updateNetHostGame(P_TEST *pt, P plrs[]);
void updateNetClientGame(P_TEST *pt, P plrs[]);

void inputsGame(P_TEST *pt, SDL_Event ev);
void inputsGamePlay(P_TEST *pt, SDL_Event ev);
void inputsGameHostingJoining(P_TEST *pt, SDL_Event ev);
void inputsGameJoin(P_TEST *pt, SDL_Event ev);
void inputsGameHost(P_TEST *pt, SDL_Event ev);
void inputsGameMain(P_TEST *pt, SDL_Event ev);
void inputsGameMenu(P_TEST *pt, SDL_Event ev);
void inputsGameLoading(P_TEST *pt, SDL_Event ev);
void inputsGameConfirm(P_TEST *pt, SDL_Event ev);

void inputsGameJoyAxis(P *p, SDL_Event ev);

void startLocalGame(P_TEST *pt);

void shootPuck(Puck *puck, float vel, float x, float y, float angle);

void renderGame(SDL_Renderer *r, FC_Font *f, P_TEST *pt, P players[]);
void renderGameMain(SDL_Renderer *r, FC_Font *f, P_TEST *pt);
void renderGameLoading(SDL_Renderer *r, FC_Font *f, P_TEST *pt);
void renderGamePlay(SDL_Renderer *r, FC_Font *f, P_TEST *pt);
void renderGameMenu(SDL_Renderer *r, FC_Font *f, P_TEST *pt);
void renderGameHostJoin(SDL_Renderer *r, FC_Font *f, P_TEST *pt);
void renderGameHosting(SDL_Renderer *r, FC_Font *f, SDL_Rect btn);
void renderGameJoining(SDL_Renderer *r, FC_Font *f, SDL_Rect btn);
void renderGameConfirm(SDL_Renderer *r, FC_Font *f, SDL_Rect *btn, unsigned char select);

void renderTiles(SDL_Renderer *r, P_TEST *pt);
void renderTexture(SDL_Renderer *r, T *t, SDL_Rect *clip, int x, int y, const double angle, const SDL_RendererFlip flip);
void renderPlayer(SDL_Renderer *r, SDL_Texture *t, P *p, int cx, int cy);
void renderPlayTiles(SDL_Renderer *r, P_TEST pt);

void renderMenuButton(SDL_Renderer *r, SDL_Rect btns[], bool select);

void animatePlayer(P *p);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);
