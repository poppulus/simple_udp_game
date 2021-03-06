#include "util.h"

void closeSdl(SDL_Window **w, SDL_Renderer **r, Mix_Chunk *m[])
{
    printf("free window\n");

    if (*w != NULL)
    {
        SDL_DestroyWindow(*w);
        *w = NULL;
    }

    printf("free renderer\n");

    if (*r != NULL)
    {
        SDL_DestroyRenderer(*r);
        *r = NULL;
    }

    printf("free audio chunks\n");

    for (int i = 0; i < 2; i++)
    {
        if (m[i] != NULL)
        {
            Mix_FreeChunk(m[i]);
            m[i] = NULL;
        }
    }
    
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}


float lerp(float v0, float v1, float t)
{
    return (1 - t) * v0 + t * v1;    
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
} 

/* itoa:  convert n to characters in s */
void n_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
} 

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
    //The sides of the rectangles
    int leftA = a.x, 
        leftB = b.x, 
        rightA = a.x + a.w, 
        rightB = b.x + b.w, 
        topA = a.y, 
        topB = b.y, 
        bottomA = a.y + a.h, 
        bottomB = b.y + b.h;

    //If any of the sides from A are outside of B
    if (bottomA <= topB) return false;
    if (topA >= bottomB) return false;
    if (rightA <= leftB) return false;
    if (leftA >= rightB) return false;

    //If none of the sides from A are outside B
    return true;
}

bool checkMousePosition(int x, int y, SDL_Rect r)
{
    if ((x > r.x && x < r.x + r.w) 
    && (y > r.y && y < r.y + r.h)) return true;

    return false;
}

bool checkPlayerPosition(int x, int y, unsigned char map[], int msize)
{
    int i = (y * msize) + x;
    if (map[i]) return true;
    return false;
}

bool checkGoal(SDL_Rect puck, SDL_Rect goal)
{
    // if one of the sides are outside of the goal, it's false
    if (puck.x < goal.x) return false;
    if (puck.x + puck.w > goal.x + goal.w) return false;
    if (puck.y < goal.y) return false;
    if (puck.y + puck.h > goal.y + goal.h) return false;

    // if all of the sides are inside of the goal, score
    return true;
}

bool checkPuckCollision(float x, float y, SDL_Rect box)
{
    if (x >= (box.x + box.w)) return false;
    if (x <= box.x) return false;
    if (y >= (box.y + box.h)) return false;
    if (y <= box.y) return false;

    return true;
}

void checkPuckXDistance(float x1, float x2, float *f)
{
    if (x1 - x2 < 100) *f = 1.5f;
    else if (x1 - x2 < 150) *f = 1.25f;
    else if (x1 - x2 < 200)  *f = 1.125f;
}

bool checkIpString(const char string[])
{
    unsigned char n = 0, m = 0;
    size_t len = strlen(string);

    for (unsigned char i = 0; i < len; i++)
    {
        if (string[i] == '.') n++;
        
        if (n == 3)
        {
            m++;
            if (m > 3) break;
        }
        else if (n > 3) break;
    }

    if (n != 3 || (m > 3 || !m))
    {
        printf("NET: incorrect IP, example: %s\n", "123.123.123.123");
        return false;
    }

    return true;
}

void checkControllers(SDL_GameController *controllers[])
{
    for (int i = 0; i < SDL_NumJoysticks(); i++) 
    {
        if (i < 4)
        {
            if (SDL_IsGameController(i)) 
            {
                controllers[i] = SDL_GameControllerOpen(i);
                if (controllers[i]) 
                {
                    printf("Controller %d connected!\n", i);
                    break;
                }
                else fprintf(
                    stderr, 
                    "Could not open gamecontroller %i: %s\n", 
                    i, SDL_GetError());
            }
        }
        else break;
    }
}

void removeController(SDL_GameController *controllers[], int id, P plrs[])
{
    if (id < 4) controllers[id] = NULL;

    for (unsigned char i = 1; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].ctrl_id == id)
        {
            resetPlayer(&plrs[i], 0, 0);
            plrs[i].id = 0;
            plrs[i].ctrl_id = JOY_ID_NULL;
            plrs[i].spawned = false;
        }
    }
}

void freeTexture(T *text)
{
    if (text->t != NULL)
    {
        SDL_DestroyTexture(text->t);
        text->t = NULL;
        text->w = 0;
        text->h = 0;
        text->a = 0;
        text->f = 0;
    }
}

bool initSdl(SDL_Window **w, SDL_Renderer **r)
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER ) < 0 )
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        *w = SDL_CreateWindow(
            "UDP Test Game", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, 
            W_WIDTH, W_HEIGHT, 
            SDL_WINDOW_SHOWN);
        if( *w == NULL )
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            *r = SDL_CreateRenderer(*w, -1, SDL_RENDERER_ACCELERATED);
            if (r == NULL) 
            {
                printf("Could not create renderer! %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
                else
                {
                    //Initialize SDL_mixer
                    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 512 ) < 0 )
                    {
                        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                        success = false;
                    }
                }
            }
        }
    }

    return success;
}

bool initAudio(Mix_Chunk *chunks[])
{
    bool aok = true;

    for (int i = 0; i < AUDIO_SAMPLE_COUNT; i++) 
    {
        chunks[i] = NULL;

        switch (i)
        {
            case S_LOW:
                if (!(chunks[i] = Mix_LoadWAV("assets/sound/low.wav")))
                    aok = false;
            break;
            case S_MEDIUM:
                if (!(chunks[i] = Mix_LoadWAV("assets/sound/medium.wav")))
                    aok = false;
            break;
            case S_HIGH:
                if (!(chunks[i] = Mix_LoadWAV("assets/sound/high.wav")))
                    aok = false;
            break;
            case S_SCRATCH:
                if (!(chunks[i] = Mix_LoadWAV("assets/sound/scratch.wav")))
                    aok = false;
            break;
            case S_BEAT:
                if (!(chunks[i] = Mix_LoadWAV("assets/sound/beat.wav")))
                    aok = false;
            break;
        }

        if (!aok)
        {
            printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
            break;
        }
    }

    return aok;
}

bool initTexture(SDL_Renderer *r, T *texture, const char path[])
{
    Uint32 f;
    int w, h, a;

    if ((texture->t = loadTexture(r, path)) != NULL)
    {
        if (SDL_QueryTexture(texture->t, &f, &a, &w, &h) == 0)
        {
            texture->a = a;
            texture->f = f;
            texture->w = w;
            texture->h = h;
        }
        else 
        {
            printf("Texture invalid! %s\n", SDL_GetError());
            return false;
        }
    }
    else 
    {
        printf("Texture is NULL!\n");
        return false;
    }

    return true;
}

bool initTextureMap(SDL_Renderer *r, P_TEST *pt, char *str)
{
    bool success = true;
    Uint32 format;  // can we use this?
    int access, w, h;

    printf("init texture %s\n", str);

    pt->texture.t = loadTexture(r, str);

    printf("texture initialised\n");

    if (pt->texture.t == NULL) success = false;
    else 
    {
        if (SDL_QueryTexture(pt->texture.t, &format, &access, &w, &h) != 0) 
        {
            success = false;
            printf("Texture invalid! %s\n", SDL_GetError());
        }
        else 
        {
            pt->texture.w = w;
            pt->texture.h = h;
            pt->texture.f = format;
            pt->texture.a = access;
            pt->texture.t_h = w >> pt->t_bit_size;
            pt->texture.pieces = (w / pt->t_n_size) * (h / pt->t_n_size);
        }
        
    }
    
    return success;
}

SDL_Texture *loadTexture(SDL_Renderer *r, const char path[])
{
    SDL_Texture *newTexture = NULL;

    SDL_Surface *loadedSurface = IMG_Load(path);

    if (loadedSurface == NULL) 
        printf("could not load image! %s\n", IMG_GetError());
    else 
    {
        newTexture = SDL_CreateTextureFromSurface(r, loadedSurface);
        
        if (newTexture == NULL) 
            printf("could not load optimised surface! %s\n", IMG_GetError());

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void renderTexture(SDL_Renderer *r, T *t, SDL_Rect *clip, int x, int y, const double angle, const SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = {x, y, t->w, t->h};

    if (clip != NULL) 
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(r, t->t, clip, &renderQuad, angle, NULL, flip);
}

bool loadMap(SDL_Renderer *r, P_TEST *pt, char *str) 
{
    bool success = true;
    FILE *map = fopen(str, "rb");

    if (map == NULL) success = false;
    else 
    {
        int n = fread(pt->f_buffer, sizeof(pt->f_buffer[0]), MAP_START, map);

        if (n > 0)
        {
            char n_str[64];
            memcpy(n_str, pt->f_buffer, 64);

            pt->t_n_size = pt->f_buffer[F_TILE];

            if (initTextureMap(r, pt, n_str)) 
            {
                setMapDimensions(
                    &pt->level, 
                    &pt->f_buffer[F_WIDTH], 
                    &pt->f_buffer[F_HEIGHT], 
                    pt->f_buffer[F_TILE]);

                switch (pt->t_n_size)
                {
                    case 8: 
                        pt->t_bit_size = 3; 
                        pt->level.t_bit_size = 3; 
                    break;
                    case 16: 
                        pt->t_bit_size = 4; 
                        pt->level.t_bit_size = 4; 
                    break;
                    case 32:
                        pt->t_bit_size = 5; 
                        pt->level.t_bit_size = 5; 
                    break;
                    case 64: 
                        pt->t_bit_size = 6; 
                        pt->level.t_bit_size = 6; 
                    break;
                }

                if (pt->m_buffer != NULL)
                {
                    free(pt->m_buffer);
                    pt->m_buffer = NULL;
                }

                pt->m_buffer = calloc(MAP_START + (pt->level.t_map_pieces * 3), 1);

                rewind(map);

                n = fread(
                    pt->m_buffer, 
                    sizeof(pt->m_buffer[0]), 
                    MAP_START + (pt->level.t_map_pieces * 3), 
                    map);

                if (n <= 0) success = false;
            }
            else success = false;
        }
        else success = false;
    }

    fclose(map);

    return success;
}

void freeBuffers(P_TEST *pt)
{
    for (int i = 0; i < MAP_START; i++)
        pt->f_buffer[i] = 0;

    free(pt->m_buffer);
    pt->m_buffer = NULL;

    printf("free map buffer\n");

    free(pt->level.map);
    pt->level.map = NULL;

    printf("free map\n");

    free(pt->level.collision);
    pt->level.collision = NULL;

    printf("free collision map\n");
}

void freePlayer(P p[])
{
    for (int i = 0; i < 2; i++)
    {
        freeTexture(p[i].texture);
        p[i].t_clips = NULL;
    }

    printf("free players\n");
}

void freePlayTest(P_TEST *ptest)
{
    ptest->plr_hud = NULL;

    freeTexture(&ptest->texture);

    printf("free texture map\n");

    for (unsigned char i = 0; i < 4; i++)
    {
        SDL_GameControllerClose(ptest->controllers[i]);
        ptest->controllers[i] = NULL;
    }

    printf("free controllers\n");
}

void initLevel(P_TEST *pt)
{
    pt->level.r.x = 0; //(pt->screen.w >> 1) - (pt->level.r.w >> 1);
    pt->level.r.y = 0; //(pt->screen.h >> 1) - (pt->level.r.h >> 1);

    pt->level.t_map_h = pt->level.r.w / pt->level.t_size;
    pt->level.t_bit_size = pt->t_bit_size;

    pt->t_clips = calloc(pt->texture.pieces, sizeof(SDL_Rect));
    initTiles(pt);
}

void initMap(P_TEST *pt)
{
    int i;

    if (pt->level.map == NULL)
        pt->level.map = calloc(pt->level.t_map_pieces << 1, 1);
    else
    {
        free(pt->level.map);
        pt->level.map = NULL;
        pt->level.map = calloc(pt->level.t_map_pieces << 1, 1);
    }

    if (pt->level.collision == NULL)
        pt->level.collision = calloc(pt->level.t_map_pieces, 1);
    else
    {
        free(pt->level.collision);
        pt->level.collision = NULL;
        pt->level.collision = calloc(pt->level.t_map_pieces, 1);
    }
    
    for (i = 0; i < pt->level.t_map_pieces; i++)
    {
        // load first map values
        pt->level.map[(i * 2)] = pt->m_buffer[MAP_START + (i * 3)];
        // load second map values, if any
        pt->level.map[(i * 2) + 1] = pt->m_buffer[MAP_START + (i * 3) + 1];
        // load collision values
        pt->level.collision[i] = pt->m_buffer[MAP_START + (i * 3) + 2];
    }

    printf("map loaded from buffer\n");
}

void initPlayer(P *p, L level, unsigned char buffer[])
{
    p->dir = &p->input_q[0];

    for (int i = 0; i < 4; i++)
    {
        p->input_q[i] = 0;
        p->aim_q[i] = 0;
    }

    p->id = 0;
    p->ctrl_id = JOY_ID_NULL;
    p->state = PLR_SKATE_NP;

    p->AIM_angle = 0;
    p->AIM_deg = 0;
    p->AIM_radx = 0;
    p->AIM_rady = 0;
    p->AIM_done = false;
    p->AIM_timer = 0;

    p->crosshair.r.w = 4;
    p->crosshair.r.h = 4;
    p->crosshair.show = false;

    p->JOY_use = false;
    p->JOY_vel = 0;
    p->JOY_xdir = 0;
    p->JOY_ydir = 0;

    p->gvel = STANDARD_VELOCITY;
    p->vel = 1;
    p->xvel = 0;
    p->yvel = 0;
    p->pvel = MIN_SHOOT_VELOCITY;

    p->sprint = false;
    p->sprint_cdown = false;
    p->spawned = false;
    p->m_hold = false;
    p->m_move = false;
    p->bounce = false;
    p->state_wait = false;

    p->sprint_timer = 0;
    p->sprint_cdown_timer = 0;
    p->block_timer = 0;
    p->state_timer = 0;
    p->swing_timer = 0;
    p->a_counter = 0;

    p->c_index = 0;
    p->facing = 0;
    p->a_index = 0;

    p->club_r.w = 16;
    p->club_r.h = 16;
    p->club_r.x = p->x - 8 + p->AIM_radx;
    p->club_r.y = p->y - 8 + p->AIM_rady;

    /* testing bullets 
    for (int i = 0; i < 10; i++)
    {
        p->bullets[i].r.w = 4;
        p->bullets[i].r.h = 4;
        p->bullets[i].shoot = false;
    }
    */

    p->clip.w = 32;
    p->clip.h = 32;
    p->clip.x = 0;
    p->clip.y = 0;

    p->r.w = 20;
    p->r.h = 20;

    p->x = buffer[F_SPAWN_X] * buffer[F_SPAWN_X + 1];
    p->mx = (int)p->x >> level.t_bit_size;
    p->r.x = p->mx << level.t_bit_size;

    p->y = buffer[F_SPAWN_Y] * buffer[F_SPAWN_Y + 1];
    p->my = (int)p->y >> level.t_bit_size;
    p->r.y = p->my << level.t_bit_size;
}

void initButtons(SDL_Rect *btns)
{
    for (unsigned char i = 0; i < 4; i++)
    {
        btns[i].w = 320;
        btns[i].h = 40;
    }
    
    btns[0].x = (W_WIDTH >> 1) - 160;
    btns[0].y = (W_HEIGHT >> 1) - 160;

    btns[1].x = (W_WIDTH >> 1) - 160;
    btns[1].y = (W_HEIGHT >> 1) - 80;

    btns[2].x = (W_WIDTH >> 1) - 160;
    btns[2].y = (W_HEIGHT >> 1);

    btns[3].x = (W_WIDTH >> 1) - 160;
    btns[3].y = (W_HEIGHT >> 1) + 80;
}

void initPlayerClips(SDL_Rect clips[])
{
    int y = 0;
    for (int i = 0; i < 12; i++)
    {
        clips[i].w = 32;
        clips[i].h = 32;
        clips[i].x = (i % 4) << 5;
        clips[i].y = y << 5;

        if ((i % 4) == 3) y++;
    }
}

void initTiles(P_TEST *pt)
{
    int x = 0, y = 0, i = 0;
    for (; i < pt->texture.pieces; i++)
    {
        pt->t_clips[i].w = pt->level.t_size;
        pt->t_clips[i].h = pt->level.t_size;
        pt->t_clips[i].x = x;
        pt->t_clips[i].y = y;

        x += pt->level.t_size;

        if (x >= pt->texture.w)
        {
            x = 0;
            y += pt->level.t_size;
        }
    }
}

void initGoalkeeper(P_G *g)
{
    g->y = 112;
    g->r.w = 20;
    g->r.h = 20;
    g->s_timer = 0;
    g->state = GK_NORMAL;
}

void setCamera(SDL_Rect *c, int x, int y)
{
    c->x = x - (c->w >> 1);
    c->y = y - (c->h >> 1);
}

void setMapDimensions(L *l, unsigned char *w, unsigned char *h, unsigned char s)
{
    l->r.w = (*w) * (*(w + 1));
    l->r.h = (*h) * (*(h + 1));
    l->t_size = s;
    l->t_map_pieces = ((l->r.w / l->t_size) * (l->r.h / l->t_size));
    printf("w%d h%d ts%d tp%d\n", l->r.w, l->r.h, l->t_size, l->t_map_pieces);
}

void setPlayerFace(float deg, enum PLAYER_FACING *face)
{
    if (deg > AIM_RIGHT || deg < AIM_DOWN)
        *face = FACING_RIGHT; 
    else if (deg > AIM_DOWN && deg < AIM_LEFT)
        *face = FACING_DOWN; 
    else if (deg > AIM_LEFT && deg < AIM_UP)
        *face = FACING_LEFT; 
    else if (deg > AIM_UP && deg < AIM_RIGHT)
        *face = FACING_UP;
}

void renderGameMain(SDL_Renderer *r, FC_Font *f, P_TEST *pt)
{
    for (unsigned char i = 0; i < 4; i++)
    {
        renderMenuButton(r, &pt->buttons[i], pt->JOY_select == i);

        switch (i)
        {
            case 0:
                FC_Draw(
                    f, r, 
                    (pt->buttons[i].x + 80),
                    pt->buttons[i].y + 10, 
                    "Local Game");
            break;
            case 1:
                FC_Draw(
                    f, r, 
                    (pt->buttons[i].x + 128), 
                    pt->buttons[i].y + 10, 
                    "Host");
            break;
            case 2:
                FC_Draw(
                    f, r, 
                    (pt->buttons[i].x + 128), 
                    pt->buttons[i].y + 10, 
                    "Join");
            break;
            case 3:
                FC_Draw(
                    f, r, 
                    (pt->buttons[i].x + 128), 
                    pt->buttons[i].y + 10, 
                    "Exit");
            break;
        }
    }
}
void renderGameLoading(SDL_Renderer *r, FC_Font *f, P_TEST *pt)
{
    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(r, &pt->buttons[1]);

    FC_Draw(
        f, r, 
        pt->buttons[1].x, 
        pt->buttons[1].y + 96, 
        "Loading ...");
}
void renderGamePlay(SDL_Renderer *r, FC_Font *f, P_TEST *pt)
{
    unsigned char i = 0;

    // background 
    //SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
    //SDL_RenderFillRect(r, &pt->screen);

    // tiles
    renderPlayTiles(r, *pt);

    //sprint cooldown
    /*
    if (pt->c_player->sprint_cdown)
    {
        SDL_SetRenderDrawColor(r, 0x00, 0x00, 0xff, 0xff);
        SDL_RenderFillRect(r, &pt->sprint_hud_r);
    }
    */

    SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
    SDL_RenderDrawRects(r, pt->plr_hud, 4);

    for (; i < MAX_GAME_USERS; i++)
    {
        if (pt->players[i].spawned)
        {
            FC_DrawColor(
                f, r, 
                pt->plr_hud[i].x + 16, pt->plr_hud[i].y + 4,
                FC_MakeColor(0xff, 0xff, 0x00, 0xff),
                "P%d", i + 1);
        }
    }

    for (i = 0; i < GOALS; i++)
    {
        SDL_Rect gq = {
            pt->goal_r[i].x - pt->camera.x, 
            pt->goal_r[i].y - pt->camera.y, 
            pt->goal_r[i].w, 
            pt->goal_r[i].h
        }, gkq = {
            pt->gk_r[i].x - pt->camera.x, 
            pt->gk_r[i].y - pt->camera.y,
            pt->gk_r[i].w, 
            pt->gk_r[i].h
        }, kq = {
            pt->goalie[i].r.x - pt->camera.x, 
            pt->goalie[i].r.y - pt->camera.y,
            pt->goalie[i].r.w, 
            pt->goalie[i].r.h
        };

        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
        SDL_RenderFillRect(r, &gq);

        SDL_RenderFillRect(r, &gkq);

        SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderFillRect(r, &kq);
    }

    for (i = 0; i < MAX_GAME_USERS; i++)
    {
        if (pt->players[i].spawned)
        {
            SDL_Rect cq = {
                pt->players[i].club_r.x - pt->camera.x, 
                pt->players[i].club_r.y - pt->camera.y, 
                pt->players[i].club_r.w, 
                pt->players[i].club_r.h
            },
            plq = {
                pt->players[i].r.x - pt->camera.x, 
                pt->players[i].r.y - pt->camera.y, 
                pt->players[i].r.w, 
                pt->players[i].r.h
            };

            switch (pt->players[i].state)
            {
                case PLR_SKATE_WP:
                    // player hitbox
                    SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
                    SDL_RenderFillRect(r, &plq);
                    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
                    SDL_RenderDrawRect(r, &plq);
                break;
                case PLR_SHOOT_MAX:
                    // player hitbox
                    if (pt->players[i].state_timer < 4)
                        SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
                    else if (pt->players[i].state_timer >= 4)
                        SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);

                    SDL_RenderFillRect(r, &plq);

                    SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
                    SDL_RenderDrawRect(r, &plq);
                break;
                case PLR_SWING:
                    // player hitbox
                    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
                    SDL_RenderFillRect(r, &plq);
                    // club hitbox
                    SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
                    SDL_RenderFillRect(r, &cq);
                break;
                default:
                    // player hitbox
                    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
                    SDL_RenderFillRect(r, &plq);
                break;
            }

            // player directional crosshair
            SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderFillRect(r, &pt->players[i].crosshair.r);
            
            // player texture
            if (checkCollision(pt->players[i].r, pt->camera))
            {
                FC_DrawColor(
                    f, r, 
                    pt->players[i].r.x - pt->camera.x, 
                    pt->players[i].r.y - 32 - pt->camera.y, 
                    FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                    "P%d", i + 1);

                renderPlayer(
                    r, 
                    pt->players[i].texture->t,
                    &pt->players[i], 
                    pt->camera.x, 
                    pt->camera.y);
            }
            else 
            {
                int nx = pt->players[i].r.x - pt->camera.x, 
                    ny = pt->players[i].r.y - 32 - pt->camera.y;

                if (pt->players[i].r.x + pt->players[i].r.w < pt->camera.x)
                {
                    if (ny < 0) ny = 10;
                    else if (ny > W_HEIGHT - 32) ny = W_HEIGHT - 32;

                    FC_DrawColor(
                        f, r, 
                        10, ny, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
                else if (pt->players[i].r.x > pt->camera.x + pt->camera.w)
                {
                    if (ny < 0) ny = 10;
                    else if (ny > W_HEIGHT - 32) ny = W_HEIGHT - 32;

                    FC_DrawColor(
                        f, r, 
                        W_WIDTH - 32, ny, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
                else if (pt->players[i].r.y + pt->players[i].r.h < pt->camera.y)
                {
                    if (nx < 0) nx = 10;
                    else if (nx > W_WIDTH - 32) nx = W_WIDTH - 32;

                    FC_DrawColor(
                        f, r, 
                        nx, 10, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
                else if (pt->players[i].r.y > pt->camera.y + pt->camera.h)
                {
                    if (nx < 0) nx = 10;
                    else if (nx > W_WIDTH - 32) nx = W_WIDTH - 32;

                    FC_DrawColor(
                        f, r, 
                        nx, W_HEIGHT - 26, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
            }
        }
    }

    if (pt->puck.state != P_STATE_GRAB)
    {
        SDL_Rect pq = {
            pt->puck.r.x - pt->camera.x, 
            pt->puck.r.y - pt->camera.y, 
            pt->puck.r.w, pt->puck.r.h
        };

        // puck hitbox
        SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderFillRect(r, &pq);
    }

    switch (pt->p_state) 
    {
        default: break;
        case P_DROP:
            FC_DrawColor(
                f, r, 
                304, 20, 
                FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                pt->state_timer_string);
        break;
        case P_PLAY:
        break;
        case P_GOAL:
            FC_DrawColor(
                f, r, 
                320 - 56, 20, 
                FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                "SCORE!");
        break;
    }

    // draw scores

    FC_DrawColor(
        f, r, 
        (W_WIDTH >> 1) - 128, 20, 
        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
        pt->score.score1_string);

    FC_DrawColor(
        f, r, 
        (W_WIDTH >> 1) + 112, 20, 
        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
        pt->score.score2_string);
}
void renderGameMenu(SDL_Renderer *r, FC_Font *f, P_TEST *pt)
{
    for (unsigned char i = 2; i < 4; i++)
        renderMenuButton(r, &pt->buttons[i], i == pt->JOY_select);

    if (pt->is_net)
    {
        FC_Draw(
            f, r, 
            (pt->buttons[2].x + 88), 
            pt->buttons[2].y + 10, 
            "Disconnect"); 

        FC_Draw(
            f, r, 
            (pt->buttons[3].x + 128), 
            pt->buttons[3].y + 10, 
            "Exit");
    }
    else 
    {
        FC_Draw(
            f, r, 
            (pt->buttons[2].x + 88), 
            pt->buttons[2].y + 10, 
            "Main Menu"); 

        FC_Draw(
            f, r, 
            (pt->buttons[3].x + 128), 
            pt->buttons[3].y + 10, 
            "Exit");
    }
    
}
void renderGameHostJoin(SDL_Renderer *r, FC_Font *f, P_TEST *pt)
{
    renderMenuButton(r, &pt->buttons[1], false);

    FC_Draw(
        f, r, 
        pt->buttons[1].x, 
        pt->buttons[1].y + 10, 
        pt->input_field.string);

    SDL_RenderDrawLine(
        r, 
        pt->buttons[1].x + (pt->input_field.str_pointer << 4), 
        pt->buttons[1].y, 
        pt->buttons[1].x + (pt->input_field.str_pointer << 4),
        pt->buttons[1].y + pt->buttons[1].h);
}
void renderGameHosting(SDL_Renderer *r, FC_Font *f, SDL_Rect btn)
{
    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(r, &btn);

    FC_Draw(
        f, r, 
        btn.x, 
        btn.y + 10, 
        "Hosting game ...");
}
void renderGameJoining(SDL_Renderer *r, FC_Font *f, SDL_Rect btn)
{
    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(r, &btn);

    FC_Draw(
        f, r, 
        btn.x, 
        btn.y + 10, 
        "Joining game ...");
}

void renderGameConfirm(SDL_Renderer *r, FC_Font *f, SDL_Rect btns[], unsigned char select)
{
    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(r, &btns[0]);

    renderMenuButton(r, &btns[2], select == 2);
    renderMenuButton(r, &btns[3], select == 3);

    FC_Draw(
        f, r, 
        btns[0].x, 
        btns[0].y + 10, 
        "Are you sure?");

    FC_Draw(
        f, r, 
        btns[2].x, 
        btns[2].y + 10, 
        "Yes");

    FC_Draw(
        f, r, 
        btns[3].x, 
        btns[3].y + 10, 
        "No");
}

void renderGame(SDL_Renderer *r, FC_Font *f, P_TEST *pt, P players[])
{
    SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(r);

    switch (pt->g_state)
    {
        default:                                                                    break;
        case G_MAIN:        renderGameMain(r, f, pt);                               break;
        case G_LOADING:     renderGameLoading(r, f, pt);                            break;
        case G_PLAY:
        case G_PLAY_NET:    renderGamePlay(r, f, pt);                               break;
        case G_MENU:        renderGameMenu(r, f, pt);                               break;
        case G_HOST:
        case G_JOIN:        renderGameHostJoin(r, f, pt);                           break;
        case G_HOSTING:     renderGameHosting(r, f, pt->buttons[1]);                break;
        case G_JOINING:     renderGameJoining(r, f, pt->buttons[1]);                break;
        case G_CONFIRM:     renderGameConfirm(r, f, pt->buttons, pt->JOY_select);   break;
    }

    SDL_RenderPresent(r);
}

void renderTiles(SDL_Renderer *renderer, P_TEST *pt)
{
    SDL_Rect r = {
        .w = pt->level.t_size, 
        .h = pt->level.t_size, 
        .x = pt->level.r.x, 
        .y = pt->level.r.y
        };

    for (int i = 0; i < (pt->level.t_map_pieces << 1); i+=2) 
    {
        if (checkCollision(r, pt->camera))
        {
            SDL_RenderCopy(
                renderer, 
                pt->texture.t, 
                &pt->t_clips[pt->level.map[i] + pt->level.map[i + 1]],
                &r);
        }

        r.x += r.w;

        if (r.x >= pt->level.r.x + pt->level.r.w)
        {
            r.x = pt->level.r.x;
            r.y += r.h;
        }
    }
}

void animatePlayer(P *p)
{
    if (*p->dir || p->JOY_vel)
    {
        if (p->a_counter++ % 8 == 0) 
        {
            p->a_index++;
            p->a_index = (p->a_index % 2) + 2;
        }
    }
    else 
    {
        if (p->a_counter++ % 64 == 0)
        {
            p->a_index++;
            p->a_index %= 2;
        }
    }

    if (p->facing == FACING_LEFT) p->c_index = 8 + p->a_index;
    else p->c_index = (p->facing * 4) + p->a_index;
}

void inputsGame(P_TEST *pt, SDL_Event ev)
{
    switch (pt->g_state)
    {
        default:                                                break;
        case G_MAIN:        inputsGameMain(pt, ev);             break;
        case G_LOADING:     inputsGameLoading(pt, ev);          break;
        case G_PLAY:
        case G_PLAY_NET:    inputsGamePlay(pt, ev);             break;
        case G_MENU:        inputsGameMenu(pt, ev);             break;
        case G_HOST:        inputsGameHost(pt, ev);             break;
        case G_JOIN:        inputsGameJoin(pt, ev);             break;
        case G_HOSTING:
        case G_JOINING:     inputsGameHostingJoining(pt, ev);   break;
        case G_CONFIRM:     inputsGameConfirm(pt, ev);          break;
    }
}

void inputsGameHostingJoining(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        if (ev.type == SDL_QUIT) pt->quit = true;
        else if (ev.type == SDL_KEYDOWN
        && ev.key.keysym.sym == SDLK_ESCAPE)
        {
            closeNet(&pt->network);
            pt->g_state = G_MAIN;
        }
    }
}

void inputsGameJoin(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT: pt->quit = true;
            break;
            case SDL_TEXTINPUT:
                switch (ev.text.text[0])
                {
                    case ':':
                    case '.':
                    case SDLK_0 ... SDLK_9:
                        if (pt->input_field.str_len < 64)
                        {
                            for (unsigned char i = pt->input_field.str_len; 
                            i > pt->input_field.str_pointer; 
                            i--)
                            {
                                pt->input_field.string[i] = pt->input_field.string[i - 1];
                            }
                            
                            pt->input_field.string[pt->input_field.str_pointer] = ev.text.text[0];
                            pt->input_field.str_pointer++;
                            pt->input_field.str_len = strlen(pt->input_field.string);
                        }
                    break;
                }
            break;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym)
                {
                    case SDLK_ESCAPE: 
                        resetInputField(&pt->input_field);
                        pt->g_state = G_MAIN;
                    break;
                    case SDLK_BACKSPACE:
                        if (pt->input_field.str_pointer > 0 && pt->input_field.str_len > 0)
                        {
                            pt->input_field.string[pt->input_field.str_pointer - 1] = 0;
                            pt->input_field.str_pointer--;

                            for (unsigned char i = pt->input_field.str_pointer; 
                            i < pt->input_field.str_len - 1; 
                            i++)
                            {
                                pt->input_field.string[i] = pt->input_field.string[i + 1];
                            }

                            pt->input_field.string[pt->input_field.str_len - 1] = 0;
                            pt->input_field.str_len = strlen(pt->input_field.string);
                        }
                    break;
                    case SDLK_LEFT:
                        if (--pt->input_field.str_pointer < 0) 
                            pt->input_field.str_pointer = 0;
                    break;
                    case SDLK_RIGHT:
                        if (++pt->input_field.str_pointer > pt->input_field.str_len) 
                            pt->input_field.str_pointer = pt->input_field.str_len;
                    break;
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    case SDLK_RETURN2:
                        if (!ev.key.repeat)
                        {
                            if (pt->input_field.str_len >= 7)
                            {
                                if (checkIpString(pt->input_field.string))
                                {
                                    if (startNetClient(&pt->network, pt->input_field.string))
                                    {
                                        pt->g_state = G_JOINING;
                                    }
                                    else 
                                    {
                                        closeNet(&pt->network);
                                        pt->g_state = G_MAIN;
                                    }
                                    resetInputField(&pt->input_field);
                                }
                            }
                        }
                    break;
                }
            break;
        }
    }
}

void inputsGameHost(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT: pt->quit = true;
            break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_ESCAPE)
                {
                    closeNet(&pt->network);
                    pt->g_state = G_MAIN;
                }
            break;
            /*
            case SDL_TEXTINPUT:
                if (pt->input_field.str_len < 5)
                {
                    switch (ev.text.text[0])
                    {
                        case SDLK_0 ... SDLK_9:
                            for (unsigned char i = pt->input_field.str_len; 
                            i > pt->input_field.str_pointer; 
                            i--)
                            {
                                pt->input_field.string[i] = pt->input_field.string[i - 1];
                            }
                            
                            pt->input_field.string[pt->input_field.str_pointer] = ev.text.text[0];
                            pt->input_field.str_pointer++;
                            pt->input_field.str_len = strlen(pt->input_field.string);
                        break;
                    }
                }
            break;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym)
                {
                    case SDLK_ESCAPE: 
                        resetInputField(&pt->input_field);
                        pt->g_state = G_MAIN;
                    break;
                    case SDLK_BACKSPACE:
                        if (pt->input_field.str_pointer > 0 && pt->input_field.str_len > 0)
                        {
                            pt->input_field.string[pt->input_field.str_pointer - 1] = 0;
                            pt->input_field.str_pointer--;

                            for (unsigned char i = pt->input_field.str_pointer; 
                            i < pt->input_field.str_len - 1; 
                            i++)
                            {
                                pt->input_field.string[i] = pt->input_field.string[i + 1];
                            }

                            pt->input_field.string[pt->input_field.str_len - 1] = 0;
                            pt->input_field.str_len = strlen(pt->input_field.string);
                        }
                    break;
                    case SDLK_LEFT:
                        if (--pt->input_field.str_pointer < 0) 
                            pt->input_field.str_pointer = 0;
                    break;
                    case SDLK_RIGHT:
                        if (++pt->input_field.str_pointer > pt->input_field.str_len) 
                            pt->input_field.str_pointer = pt->input_field.str_len;
                    break;
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    case SDLK_RETURN2:
                        if (!ev.key.repeat)
                        {
                            if (pt->input_field.str_len > 0)
                            {
                                // setup host connection
                                if (startNetHost(&pt->network))
                                {
                                    pt->players[0].id = HOST_ID;
                                    pt->players[0].spawned = true;
                                    pt->g_state = G_HOSTING;
                                }
                                else 
                                {
                                    closeNet(&pt->network);
                                    pt->g_state = G_MAIN;
                                }
                            }
                            resetInputField(&pt->input_field);
                        }
                    break;
                }
            */
            break;
        }
    }
}

void inputsGameMain(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT: pt->quit = true; break;
            case SDL_MOUSEMOTION:
                for (unsigned char i = 0; i < 4; i++)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[i]))
                    {
                        pt->JOY_select = i;
                    }
                }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[pt->JOY_select]))
                    {
                        switch (pt->JOY_select)
                        {
                            case 0: pt->g_state = G_LOADING;    break;
                            case 1: pt->g_state = G_HOST;       break;
                            case 2: pt->g_state = G_JOIN;       break;
                            case 3: pt->quit = true;            break;
                        }
                    }
                }
            break;
            case SDL_JOYAXISMOTION:
                if (ev.jaxis.which == 0)
                {
                    if (ev.jaxis.axis == 1)
                    {
                        if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                        {
                            if (!pt->menu_hold && pt->JOY_select > 0) pt->JOY_select--;
                            pt->menu_hold = true;
                        }
                        else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                        {
                            if (!pt->menu_hold && pt->JOY_select < 3) pt->JOY_select++;
                            pt->menu_hold = true;
                        }
                        else pt->menu_hold = false;
                    }
                }
                
            break;
            case SDL_JOYBUTTONDOWN:
                if (ev.jaxis.which == 0)
                {
                    if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_A)
                    {
                        switch (pt->JOY_select)
                        {
                            case 0: pt->g_state = G_LOADING;    break;
                            case 1: pt->g_state = G_HOST;       break;
                            case 2: pt->g_state = G_JOIN;       break;
                            case 3: pt->quit = true;            break;
                        }
                    }
                }
            break;
            case SDL_JOYDEVICEADDED:
                checkControllers(pt->controllers);
            break;
            case SDL_JOYDEVICEREMOVED:
                printf("Controller %d disconnected!\n", ev.jdevice.which);
                removeController(pt->controllers, ev.jdevice.which, pt->players);
            break;
        }
    }
}

void inputsGameLoading(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        if (ev.type == SDL_QUIT) pt->quit = true;
    }
}

void inputsGameMenu(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT: pt->quit = true; 
            break;
            case SDL_KEYDOWN:
                if (!ev.key.repeat && ev.key.keysym.sym == SDLK_ESCAPE)
                {
                    if (pt->is_net) pt->g_state = G_PLAY_NET;
                    else pt->g_state = G_PLAY;
                }
            break;
            case SDL_MOUSEMOTION:
                for (unsigned char i = 2; i < 4; i++)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[i]))
                    {
                        pt->JOY_select = i;
                    }
                }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[pt->JOY_select]))
                    {
                        if (pt->is_net)
                        {
                            if (pt->JOY_select == 2)
                            {
                                // send disconnect signal
                                if (pt->network.type == NET_IS_CLIENT)
                                {
                                    pt->network.localuser.status = N_DISCONNECT;

                                    unsigned long timer = SDL_GetTicks64();

                                    while (!pt->network.lost)
                                    {
                                        if ((SDL_GetTicks64() - timer) >= 1000) 
                                            pt->network.lost = true;
                                    }
                                }

                                closeNet(&pt->network);
                                pt->is_net = false;
                                resetPlay(pt, pt->players, true);
                                pt->JOY_select = 0;
                                pt->g_state = G_MAIN;
                            }
                            else if (pt->JOY_select == 3) pt->g_state = G_CONFIRM;
                        }
                        else 
                        {
                            if (pt->JOY_select == 2) 
                            {
                                resetPlay(pt, pt->players, true);
                                pt->JOY_select = 0;
                                pt->g_state = G_MAIN;
                            }
                            else if (pt->JOY_select == 3) pt->g_state = G_CONFIRM;
                        }
                    }
                }
            break;
            case SDL_JOYAXISMOTION:
                if (ev.jaxis.which == 0)
                {
                    if (ev.jaxis.axis == 1)
                    {
                        if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                        {
                            if (!pt->menu_hold) pt->JOY_select = 2;
                            pt->menu_hold = true;
                        }
                        else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                        {
                            if (!pt->menu_hold) pt->JOY_select = 3;
                            pt->menu_hold = true;
                        }
                        else pt->menu_hold = false;
                    }
                }
                
            break;
            case SDL_JOYBUTTONDOWN:
                if (ev.jaxis.which == 0)
                {
                    if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_A)
                    {
                        if (pt->is_net)
                        {
                            if (pt->JOY_select == 2)
                            {
                                // send disconnect signal
                                if (pt->network.type == NET_IS_CLIENT)
                                {
                                    pt->network.localuser.status = N_DISCONNECT;

                                    unsigned long timer = SDL_GetTicks64();

                                    while (!pt->network.lost)
                                    {
                                        if ((SDL_GetTicks64() - timer) >= 1000) 
                                            pt->network.lost = true;
                                    }
                                }

                                closeNet(&pt->network);
                                pt->is_net = false;
                                resetPlay(pt, pt->players, true);
                                pt->JOY_select = 0;
                                pt->g_state = G_MAIN;
                            }
                            else if (pt->JOY_select == 3) pt->g_state = G_CONFIRM;
                        }
                        else 
                        {
                            if (pt->JOY_select == 2) 
                            {
                                resetPlay(pt, pt->players, true);
                                pt->JOY_select = 0;
                                pt->g_state = G_MAIN;
                            }
                            else if (pt->JOY_select == 3) pt->g_state = G_CONFIRM;
                        }
                    }
                }
            break;
            case SDL_JOYDEVICEADDED:
                checkControllers(pt->controllers);
            break;
            case SDL_JOYDEVICEREMOVED:
                printf("Controller %d disconnected!\n", ev.jdevice.which);
                removeController(pt->controllers, ev.jdevice.which, pt->players);
            break;
        }
    }
}

void inputsGamePlay(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT: pt->quit = true; 
            break;
            case SDL_KEYDOWN:
                if (!ev.key.repeat)
                {
                    pt->c_player->JOY_use = false;

                    switch (ev.key.keysym.sym)
                    {
                        case SDLK_ESCAPE: 
                            pt->JOY_select = 2;
                            pt->g_state = G_MENU; 
                        break;
                        case SDLK_a: 
                            enqueue(pt->c_player->input_q, KEY_LEFT);
                        break;
                        case SDLK_w: 
                            enqueue(pt->c_player->input_q, KEY_UP);
                        break;
                        case SDLK_d: 
                            enqueue(pt->c_player->input_q, KEY_RIGHT);
                        break;
                        case SDLK_s: 
                            enqueue(pt->c_player->input_q, KEY_DOWN); 
                        break;
                        case SDLK_KP_4:
                            enqueue(pt->c_player->aim_q, KEY_LEFT);
                        break;
                        case SDLK_KP_8:
                            enqueue(pt->c_player->aim_q, KEY_UP);
                        break;
                        case SDLK_KP_6:
                            enqueue(pt->c_player->aim_q, KEY_RIGHT);
                        break;
                        case SDLK_KP_5:
                            enqueue(pt->c_player->aim_q, KEY_DOWN);
                        break;
                        case SDLK_SPACE:
                        case SDLK_KP_SPACE:
                            if (pt->c_player->state == PLR_SKATE_WP)
                                pt->c_player->m_hold = true;
                            else if (pt->c_player->state == PLR_SKATE_NP)
                            {
                                pt->c_player->gvel -= STANDARD_VELOCITY * 0.25f;
                                pt->c_player->state = PLR_SWING;
                            }
                        break;
                        case SDLK_LSHIFT:
                            if (pt->c_player->state == PLR_SKATE_NP)
                            {
                                if (!pt->c_player->state_wait)
                                {
                                    pt->c_player->gvel = SPRINT_VELOCITY;
                                    pt->c_player->state = PLR_SPRINT;
                                }
                            }
                        break;
                        case SDLK_RSHIFT:
                            if (pt->c_player->state == PLR_SKATE_NP && pt->c_player->JOY_vel)
                            {
                                pt->c_player->gvel += STANDARD_VELOCITY * 0.375f;
                                pt->c_player->state = PLR_BLOCK;
                            }
                        break;
                    }
                }
            break;
            case SDL_KEYUP:
                pt->c_player->JOY_use = false;

                switch (ev.key.keysym.sym)
                {
                    case SDLK_a: 
                        dequeue(pt->c_player->input_q, KEY_LEFT);
                    break;
                    case SDLK_w: 
                        dequeue(pt->c_player->input_q, KEY_UP);
                    break;
                    case SDLK_d: 
                        dequeue(pt->c_player->input_q, KEY_RIGHT);
                    break;
                    case SDLK_s: 
                        dequeue(pt->c_player->input_q, KEY_DOWN);
                    break;
                    case SDLK_KP_4:
                        dequeue(pt->c_player->aim_q, KEY_LEFT);
                    break;
                    case SDLK_KP_8:
                        dequeue(pt->c_player->aim_q, KEY_UP);
                    break;
                    case SDLK_KP_6:
                        dequeue(pt->c_player->aim_q, KEY_RIGHT);
                    break;
                    case SDLK_KP_5:
                        dequeue(pt->c_player->aim_q, KEY_DOWN);
                    break;
                    case SDLK_SPACE:
                    case SDLK_KP_SPACE:
                        if (pt->c_player->state == PLR_SKATE_WP 
                        || pt->c_player->state == PLR_SHOOT_MAX)
                        {
                            pt->c_player->swing_timer = 0;
                            pt->c_player->m_hold = false;
                            pt->c_player->state = PLR_SHOOT;
                        }
                    break;
                }
            break;
            case SDL_MOUSEMOTION:
            break;
            case SDL_MOUSEBUTTONDOWN:
            break;
            case SDL_JOYDEVICEADDED:
                checkControllers(pt->controllers);
            break;
            case SDL_JOYDEVICEREMOVED:
                printf("Controller %d disconnected!\n", ev.jdevice.which);
                removeController(pt->controllers, ev.jdevice.which, pt->players);
            break;
            case SDL_JOYAXISMOTION:
                if (!pt->is_net)
                {
                    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
                    {
                        if (ev.jaxis.which == pt->players[i].ctrl_id) 
                        {
                            pt->players[i].JOY_use = true;
                            inputsGameJoyAxis(&pt->players[i], ev);
                        }
                    }
                }
                else 
                {
                    if (ev.jaxis.which == 0) 
                    {
                        pt->c_player->JOY_use = true;
                        inputsGameJoyAxis(pt->c_player, ev);
                    }
                }
            break;
            case SDL_JOYBUTTONDOWN:
                if (!pt->is_net)
                {
                    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
                    {
                        if (ev.jbutton.which == pt->players[i].ctrl_id)
                        {
                            pt->players[i].JOY_use = true;
                        
                            if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_A)
                            {
                                if (!pt->players[i].sprint_cdown 
                                && pt->players[i].state == PLR_SKATE_NP) 
                                {
                                    pt->players[i].gvel = SPRINT_VELOCITY;
                                    pt->players[i].state = PLR_SPRINT;
                                }
                            }
                            else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_B)
                            {
                                if (pt->players[i].state == PLR_SKATE_NP && pt->players[i].JOY_vel)
                                {
                                    pt->players[i].gvel += STANDARD_VELOCITY * 0.375f;
                                    pt->players[i].state = PLR_BLOCK;
                                }
                            }
                            else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_START)
                            {
                                if (&pt->players[i] == pt->c_player)
                                {
                                    pt->JOY_select = 2;
                                    pt->g_state = G_MENU;
                                }
                            }
                            else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_BACK)
                            {
                                if (&pt->players[i] != pt->c_player)
                                    removeController(pt->controllers, ev.jdevice.which, pt->players);
                            }
                        }
                    }

                    if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_START)
                    {
                        if (!pt->is_net)
                        {
                            bool has = false;

                            for (unsigned char j = 1; j < MAX_GAME_USERS; j++)
                            {
                                if (pt->players[j].ctrl_id == ev.jbutton.which)
                                {
                                    has = true;
                                    break;
                                }
                            }

                            if (!has) 
                            {
                                for (unsigned char j = 1; j < MAX_GAME_USERS; j++)
                                {
                                    if (pt->players[j].ctrl_id == JOY_ID_NULL)
                                    {
                                        resetPlayer(
                                            &pt->players[j], 
                                            pt->level.r.w >> 1, 
                                            pt->level.r.h >> 1);
                                            
                                        pt->players[j].ctrl_id = ev.jbutton.which;
                                        pt->players[j].spawned = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else 
                {
                    if (ev.jbutton.which == 0) 
                    {
                        pt->c_player->JOY_use = true;

                        if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_A)
                        {
                            if (!pt->c_player->sprint_cdown 
                            && pt->c_player->state == PLR_SKATE_NP) 
                            {
                                pt->c_player->gvel = SPRINT_VELOCITY;
                                pt->c_player->state = PLR_SPRINT;
                            }
                        }
                        else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_B)
                        {
                            if (pt->c_player->state == PLR_SKATE_NP && pt->c_player->JOY_vel)
                            {
                                pt->c_player->gvel += STANDARD_VELOCITY * 0.375f;
                                pt->c_player->state = PLR_BLOCK;
                            }
                        }
                        else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_START)
                        {
                            pt->JOY_select = 2;
                            pt->g_state = G_MENU;
                        }
                    }
                }
            break;
        }
    }
}

void inputsGameConfirm(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT: pt->quit = true; 
            break;
            case SDL_KEYDOWN:
                if (!ev.key.repeat && ev.key.keysym.sym == SDLK_ESCAPE)
                {
                    pt->g_state = G_MENU;
                }
            break;
            case SDL_MOUSEMOTION:
                for (unsigned char i = 2; i < 4; i++)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[i]))
                    {
                        pt->JOY_select = i;
                    }
                }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[pt->JOY_select]))
                    {
                        if (pt->JOY_select == 2)
                        {
                            pt->quit = true;
                        }
                        else if (pt->JOY_select == 3)
                        {
                            pt->g_state = G_MENU;
                        }
                    }
                }
            break;
            case SDL_JOYAXISMOTION:
                if (ev.jaxis.which == 0)
                {
                    if (ev.jaxis.axis == 1)
                    {
                        if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                        {
                            if (!pt->menu_hold) pt->JOY_select = 2;
                            pt->menu_hold = true;
                        }
                        else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                        {
                            if (!pt->menu_hold) pt->JOY_select = 3;
                            pt->menu_hold = true;
                        }
                        else pt->menu_hold = false;
                    }
                }
                
            break;
            case SDL_JOYBUTTONDOWN:
                if (ev.jaxis.which == 0)
                {
                    if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_A)
                    {
                        if (pt->JOY_select == 2)
                        {
                            pt->quit = true;
                        }
                        else if (pt->JOY_select == 3)
                        {
                            pt->g_state = G_MENU;
                        }
                    }
                }
            break;
        }
    }
}

void inputsGameJoyAxis(P *p, SDL_Event ev)
{
    switch (ev.jaxis.axis)
    {
        default: break;
        case 0: // left stick l/r
        case 1: // left stick u/d
            if (ev.jaxis.axis == 0)
            {
                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                    p->JOY_xdir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                    p->JOY_xdir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                else 
                    p->JOY_xdir = 0;
            }
            else if (ev.jaxis.axis == 1)
            {
                if (ev.jaxis.value < -JOYSTICK_DEADZONE) 
                    p->JOY_ydir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                    p->JOY_ydir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                else 
                    p->JOY_ydir = 0;
            }

            p->JOY_vel = p->JOY_xdir || p->JOY_ydir ? 1 : 0;
        break;
        case 2: // right stick l/r
        case 3: // right stick u/p
            if (ev.jaxis.axis == 2)
            {
                if (ev.jaxis.value < -JOYSTICK_DEADZONE) 
                    p->AIM_xdir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                    p->AIM_xdir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                else 
                    p->AIM_xdir = 0;
            }
            else if (ev.jaxis.axis == 3)
            {
                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                    p->AIM_ydir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                    p->AIM_ydir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                else 
                    p->AIM_ydir = 0;
            }
        break;
        case 5:
            switch (p->state)
            {
                default: break;
                case PLR_SKATE_NP:
                    if (ev.jaxis.value > -16000)
                    {
                        if (!p->m_hold) 
                        {
                            p->gvel -= STANDARD_VELOCITY * 0.25f;
                            p->state = PLR_SWING;
                            //Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                        }
                        p->m_hold = true;
                    } 
                    else p->m_hold = false;
                break;
                case PLR_SKATE_WP:
                case PLR_SHOOT_MAX:
                    if (ev.jaxis.value > -16000) p->m_hold = true;
                    else 
                    {
                        if (p->m_hold) 
                        {
                            p->swing_timer = 0;
                            p->state = PLR_SHOOT;
                        }
                        p->m_hold = false;
                    }
                break;
            }
        break;
    }
}

void startLocalGame(P_TEST *pt)
{
    pt->is_net = false;
    pt->state_change = false;
    pt->playing = false;

    pt->SCORE_timer = 0;
    pt->STATE_timer = 0;
    pt->STATE_cntdwn = 3;

    pt->PUCK_freeze = false;
    pt->PUCK_freeze_timer = 0;

    pt->score.score1 = 0;
    pt->score.score2 = 0;

    resetGoalkeeper(&pt->goalie[0], 154);
    resetGoalkeeper(&pt->goalie[1], 742);

    resetPuck(&pt->puck, pt->level.r.w >> 1, pt->level.r.h >> 1);

    pt->camera.x = pt->puck.x - (pt->camera.w >> 1);
    pt->camera.y = pt->puck.y - (pt->camera.h >> 1);

    pt->c_player = NULL;

    for (unsigned i = 0; i < MAX_GAME_USERS; i++)
    {
        resetPlayer(
            &pt->players[i],
            pt->level.r.w >> 1, 
            pt->level.r.h >> 1);

        pt->players[i].id = 0;
        pt->players[i].ctrl_id = JOY_ID_NULL;
        pt->players[i].spawned = false;
    }

    pt->c_player = &pt->players[0];
    
    pt->c_player->club_r.x = pt->c_player->x + pt->c_player->AIM_radx;
    pt->c_player->club_r.y = pt->c_player->y + pt->c_player->AIM_rady;

    pt->c_player->r.x = pt->c_player->x - pt->camera.x;
    pt->c_player->r.y = pt->c_player->y - pt->camera.y;

    pt->c_player->spawned = true;

    pt->p_state = P_DROP;
}

void shootPuck(Puck *puck, float vel, float x, float y, float angle)
{
    puck->x = x;
    puck->y = y;
    puck->xvel = vel * SDL_cos(angle);
    puck->yvel = vel * SDL_sin(angle);
    puck->state = P_STATE_HIT;
}

void enqueue(unsigned char *q, unsigned char val)
{
    for (int i = 4 - 1; i > 0; i--)
        q[i] = q[i - 1];
    
    q[0] = val;
}

void dequeue(unsigned char *q, unsigned char val)
{
    bool found = false;

    for (int i = 0; i < 4; i++)
    {
        if (q[i] == val) 
            found = true;

        if (found && ((i + 1) < 4)) 
            q[i] = q[i + 1];
    }

    q[4 - 1] = 0;
}

void adjustGoalieX(float *gkx, float rx, float vx)
{

}

void adjustGoalieY(float *gky, float ry, float vy)
{
    if (ry < *gky)
    {
        if ((*gky -= vy) < ry) *gky = ry;
    }
    else if (ry > *gky)
    {
        if ((*gky += vy) > ry) *gky = ry;
    }

    if (*gky < 112) *gky = 112;
    else if (*gky > 160) *gky = 160;
}

void updateGameMain(P_TEST *pt)
{

}
void updateGameLoading(P_TEST *pt)
{
    startLocalGame(pt);
    pt->g_state = G_PLAY;
}
void updateGameMenu(P_TEST *pt)
{

}
void updateGameHost(P_TEST *pt)
{
    // setup host connection
    if (startNetHost(&pt->network))
    {
        pt->players[0].id = HOST_ID;
        pt->players[0].spawned = true;
        pt->g_state = G_HOSTING;
    }
    else 
    {
        closeNet(&pt->network);
        pt->g_state = G_MAIN;
    }
}
void updateGameJoin(P_TEST *pt)
{

}
void updateGameHosting(P_TEST *pt)
{
    if (pt->network.ok)
    {
        pt->network.type = NET_IS_HOST;
        resetPuck(&pt->puck, pt->level.r.w >> 1, pt->level.r.h >> 1);
        resetPlay(pt, pt->players, false);
        pt->is_net = true;
        pt->g_state = G_PLAY_NET;
    }
}
void updateGameJoining(P_TEST *pt)
{
    if (pt->network.ok)
    {
        //resetPuck(&pt->puck, 0, 0);
        //resetPlay(pt, pt->players, false);
        pt->network.type = NET_IS_CLIENT;
        pt->is_net = true;
        pt->g_state = G_PLAY_NET;
    }
    else if (pt->network.lost)
    {
        closeNet(&pt->network);
        resetPlay(pt, pt->players, true);
        pt->g_state = G_MAIN;
    }
}

void updateCamera(SDL_Rect *camera, SDL_Rect level, int rx)
{
    // set camera, duh
    setCamera(camera, 
        lerp(camera->x + (camera->w >> 1), rx, 0.08f), 
        (level.h >> 1)
    );
    /* not used on camera y position
    lerp(pt->camera.y + (pt->camera.h >> 1), pt->ry, 0.08f)
    */

    if (camera->x < 0) camera->x = 0;
    else if ((camera->x + W_WIDTH) > level.w) 
        camera->x = level.w - W_WIDTH;

    if (camera->y > 0) camera->y = 0;
    else if ((camera->y + W_HEIGHT) < level.h) 
        camera->y = level.h - W_HEIGHT;
}

void updateGame(P_TEST *pt, P players[])
{
    switch (pt->g_state)
    {
        default:                                            break;
        case G_MAIN:        updateGameMain(pt);             break;
        case G_LOADING:     updateGameLoading(pt);          break;
        case G_PLAY:        updateGamePlay(pt, players);    break;
        case G_PLAY_NET:    updateGamePlayNet(pt);          break;
        case G_MENU:        updateGameMenu(pt);             break;
        case G_HOST:        updateGameHost(pt);             break;
        case G_JOIN:        updateGameJoin(pt);             break;
        case G_HOSTING:     updateGameHosting(pt);          break;
        case G_JOINING:     updateGameJoining(pt);          break;
    }
}

void updateGamePlay(P_TEST *pt, P plrs[])
{
    switch (pt->p_state)
    {
        case P_DROP:
            if (!pt->state_change)
            {
                resetPuck(
                    &pt->puck, 
                    pt->level.r.w >> 1, 
                    pt->level.r.h >> 1);

                for (unsigned char p = 0; p < MAX_GAME_USERS; p++)
                {
                    if (plrs[p].spawned)
                    {
                        resetPlayer(
                            &plrs[p], 
                            pt->level.r.w >> 1, 
                            (pt->level.r.h >> 1) + 40);
                    }
                }

                n_itoa(pt->STATE_cntdwn, pt->state_timer_string);

                pt->state_change = true;
            }
            
            if (++pt->STATE_timer > 180)
            {
                pt->STATE_cntdwn = 3;
                pt->STATE_timer = 0;
                pt->state_change = false;
                pt->p_state = P_PLAY;
            }
            else 
            {
                for (unsigned char p = 0; p < MAX_GAME_USERS; p++)
                {
                    if (plrs[p].spawned)
                    {
                        plrs[p].x = pt->level.r.w >> 1;
                        plrs[p].y = (pt->level.r.h >> 1) + 40;
                    }
                }

                if ((pt->STATE_timer % 60) == 0) 
                {
                    pt->STATE_cntdwn--;
                    n_itoa(pt->STATE_cntdwn, pt->state_timer_string);
                }
            }
        break;
        case P_PLAY:
            for (unsigned char i = 0; i < GOALS; i++)
            {
                if (checkGoal(pt->puck.r, pt->goal_r[i]))
                {
                    if (i == 0) pt->score.score1++;
                    else if (i == 1) pt->score.score2++;
                    // convert scores to string
                    n_itoa(pt->score.score1, pt->score.score1_string);
                    n_itoa(pt->score.score2, pt->score.score2_string);
                    pt->p_state = P_GOAL;
                }
            }
        break;
        case P_GOAL:
            if (++pt->SCORE_timer > 180)
            {
                pt->SCORE_timer = 0;
                pt->p_state = P_DROP;
            }
        break;
    }

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            switch (plrs[i].state)
            {
                default: break;
                case PLR_SPRINT:
                    if ((++plrs[i].sprint_cdown_timer) > 60)
                    {
                        plrs[i].sprint_cdown_timer = 0;
                        plrs[i].gvel = STANDARD_VELOCITY;
                        plrs[i].state = PLR_SKATE_NP;
                    }
                case PLR_SKATE_NP:
                    if (plrs[i].state_wait)
                    {
                        if (++plrs[i].state_timer > 10)
                        {
                            plrs[i].state_timer = 0;
                            plrs[i].state_wait = false;
                        }
                    }

                    if (pt->puck.state == P_STATE_NORMAL && pt->p_state == P_PLAY)
                    {
                        if (checkCollision(plrs[i].r, pt->puck.r))
                        {
                            printf("PLAYER: %d pick up puck\n", plrs[i].id);
                            pt->puck.state = P_STATE_GRAB;
                            pt->puck.xvel = 0;
                            pt->puck.yvel = 0;

                            plrs[i].sprint_cdown_timer = 0;
                            plrs[i].gvel = STANDARD_VELOCITY;
                            plrs[i].state = PLR_SKATE_WP;
                        }
                    }

                    plrs[i].r.x = plrs[i].x - 10;
                    plrs[i].r.y = plrs[i].y;
                break;
                case PLR_SKATE_WP:
                    pt->puck.x = plrs[i].x;
                    pt->puck.y = plrs[i].y;

                    plrs[i].r.x = plrs[i].x - 10;
                    plrs[i].r.y = plrs[i].y;
                break;
                case PLR_SHOOT:
                    if (pt->puck.state == P_STATE_GRAB)
                    {
                        printf("PLAYER: %d shoot puck\n", plrs[i].id);
                    
                        shootPuck(
                            &pt->puck, 
                            plrs[i].pvel, 
                            plrs[i].x - 4, plrs[i].y + 10, 
                            plrs[i].AIM_angle
                        );

                        pt->PUCK_freeze = true; 
                        
                        plrs[i].gvel -= plrs[i].gvel * 0.25f;
                        plrs[i].pvel = MIN_SHOOT_VELOCITY;
                        plrs[i].state_timer = 0;
                        plrs[i].state = PLR_SWING;

                        //Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                    }

                    plrs[i].r.x = plrs[i].x - 10;
                    plrs[i].r.y = plrs[i].y;
                break;
                case PLR_SHOOT_MAX: // make sure player has the highest shoot velocity
                    plrs[i].pvel = MAX_SHOOT_VELOCITY;

                    if (++plrs[i].state_timer > 7) 
                        plrs[i].state_timer = 0;

                    plrs[i].r.x = plrs[i].x - 10;
                    plrs[i].r.y = plrs[i].y;
                break;
                case PLR_SWING:
                    if (pt->puck.state == P_STATE_GRAB)
                    {
                        for (unsigned char j = 0; j < MAX_GAME_USERS; j++)
                        {
                            if ((plrs[j].state == PLR_SKATE_WP 
                            || plrs[j].state == PLR_SHOOT_MAX)
                            && checkCollision(plrs[i].club_r, plrs[j].r))
                            {
                                printf(
                                    "PLAYER: p: %d hit the puck off p: %d\n", 
                                    plrs[i].id, plrs[j].id);

                                // puck goes wild
                                shootPuck(
                                    &pt->puck, 1.5f, 
                                    plrs[j].x - 4, plrs[j].y, 
                                    plrs[i].AIM_angle);

                                plrs[j].state = PLR_SKATE_NP;

                                pt->PUCK_freeze = true;

                                //Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                                break;
                            }
                        }
                    }

                    if ((++plrs[i].swing_timer) > 10)
                    {
                        plrs[i].swing_timer = 0;
                        plrs[i].gvel = STANDARD_VELOCITY;
                        plrs[i].state = PLR_SKATE_NP;
                    }

                    plrs[i].r.x = plrs[i].x - 10;
                    plrs[i].r.y = plrs[i].y;
                break;
                case PLR_BLOCK:
                    plrs[i].JOY_vel = 1;

                    if ((plrs[i].gvel -= 0.05f) < 0) plrs[i].gvel = 0;

                    if ((++plrs[i].block_timer) > 60)
                    {
                        plrs[i].block_timer = 0;
                        plrs[i].bounce = false;
                        plrs[i].gvel = STANDARD_VELOCITY;
                        plrs[i].r.w = 20;
                        plrs[i].r.h = 20;
                        plrs[i].state = PLR_SKATE_NP;
                    }
                    else 
                    {
                        switch (plrs[i].facing)
                        {
                            case FACING_DOWN:
                            case FACING_UP:
                                plrs[i].r.w = 16;
                                plrs[i].r.h = 30;
                            break;
                            case FACING_RIGHT:
                            case FACING_LEFT:
                                plrs[i].r.w = 30;
                                plrs[i].r.h = 16;
                            break;
                        }
                    }

                    switch (plrs[i].facing)
                    {
                        case FACING_DOWN:
                            plrs[i].r.x = plrs[i].x - 8;
                            plrs[i].r.y = plrs[i].y;
                        break;
                        case FACING_UP:
                            plrs[i].r.x = plrs[i].x - 8;
                            plrs[i].r.y = plrs[i].y - 10;
                        break;
                        case FACING_RIGHT:
                            plrs[i].r.x = plrs[i].x - 10;
                            plrs[i].r.y = plrs[i].y + 5;
                        break;
                        case FACING_LEFT:
                            plrs[i].r.x = plrs[i].x - 20;
                            plrs[i].r.y = plrs[i].y + 5;
                        break;
                    }
                break;
            }

            if (plrs[i].state != PLR_BLOCK) updatePlayerInputs(&plrs[i]);
            
            if (plrs[i].JOY_vel && plrs[i].state != PLR_SHOOT_MAX)
            {
                float   cos = (
                    SDL_cos(plrs[i].INPUT_angle) * plrs[i].gvel),
                        sin = (
                    SDL_sin(plrs[i].INPUT_angle) * plrs[i].gvel);

                addPlayerVel(&plrs[i].xvel, cos);
                addPlayerVel(&plrs[i].yvel, sin);
            }
            else
            {
                if (plrs[i].state != PLR_SPRINT)
                {
                    subPlayerVel(&plrs[i].xvel);
                    subPlayerVel(&plrs[i].yvel);
                }
            }

            if (plrs[i].xvel) updatePlayerX(&plrs[i], pt->level);

            if (plrs[i].yvel) updatePlayerY(&plrs[i], pt->level);

            if (plrs[i].m_hold)
            {
                if (++plrs[i].swing_timer > 30)
                {
                    plrs[i].swing_timer = 31;
                    plrs[i].state = PLR_SHOOT_MAX;
                }
            }

            plrs[i].AIM_deg = (plrs[i].AIM_angle * 180) / AIM_PI;

            if (plrs[i].AIM_deg < 0) plrs[i].AIM_deg += 360;

            setPlayerFace(plrs[i].AIM_deg, &plrs[i].facing);

            plrs[i].AIM_radx = 10 * SDL_cos(plrs[i].AIM_angle);
            plrs[i].AIM_rady = 10 * SDL_sin(plrs[i].AIM_angle);

            plrs[i].club_r.x = plrs[i].r.x + plrs[i].AIM_radx;
            plrs[i].club_r.y = plrs[i].r.y + plrs[i].AIM_rady;

            plrs[i].crosshair.r.x = plrs[i].club_r.x + 10 - pt->camera.x;
            plrs[i].crosshair.r.y = plrs[i].club_r.y + 10 - pt->camera.y;

            animatePlayer(&plrs[i]);
        }
    }

    updatePuck(pt, pt->players);
    updateGoalKeepers(pt, pt->players, pt->puck.state == P_STATE_GRAB ? 1 : 0);

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;

    pt->rx = pt->puck.x;
    pt->ry = pt->puck.y;

    updateCamera(&pt->camera, pt->level.r, pt->rx);
}

void updateGamePlayNet(P_TEST *pt)
{
    updateNetGame(pt, pt->players);

    if (pt->network.join)
    {
        printf("NET: player join\n");
        pt->network.join = false;
        for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
        {
            if (!pt->players[i].id && pt->network.players_net[i].id)
            {
                addPlayerGame(
                    &pt->players[i], 
                    pt->network.players_net[i].id, 
                    pt->network.players_net[i].x, 
                    pt->network.players_net[i].y);

                printf("PLAYER: %d joined\n", pt->players[i].id);
            }

            if (pt->players[i].id == pt->network.localplayer->id)
                pt->c_player = &pt->players[i];
        }
    }

    if (pt->network.left)
    {
        printf("NET: player left\n");
        pt->network.left = false;
        for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
        {
            if (pt->players[i].id && !pt->network.players_net[i].id)
            {
                printf("PLAYER: %d left\n", pt->players[i].id);

                if (pt->network.puck.id == pt->players[i].id)
                {
                    resetPuck(&pt->puck, pt->players[i].x, pt->players[i].y);
                    pt->network.puck.x = pt->puck.x;
                    pt->network.puck.y = pt->puck.y;
                    pt->network.puck.id = 0;
                }

                removePlayerGame(&pt->players[i]);
            }
        }
    }
    
    if (pt->network.lost)
    {
        // disconnect from net here
        closeNet(&pt->network);
        pt->is_net = false;
        resetPlay(pt, pt->players, true);
        resetScores(&pt->score);
        // set local player to first
        pt->c_player = &pt->players[0];
        pt->c_player->spawned = true;
    }

    // convert score to string
    n_itoa(pt->score.score1, pt->score.score1_string);
    n_itoa(pt->score.score2, pt->score.score2_string);

    updateCamera(&pt->camera, pt->level.r, pt->rx);
}

void updateKeyInputs(P *p)
{
    switch (*p->dir)
    {
        case KEY_LEFT:
            if (p->input_q[1] == KEY_UP) 
                p->INPUT_angle = I_UP_LEFT;
            else if (p->input_q[1] == KEY_DOWN) 
                p->INPUT_angle = I_DOWN_LEFT;
            else 
                p->INPUT_angle = INPUT_LEFT;

            p->JOY_vel = 1;
        break;
        case KEY_RIGHT:
            if (p->input_q[1] == KEY_UP) 
                p->INPUT_angle = I_UP_RIGHT;
            else if (p->input_q[1] == KEY_DOWN) 
                p->INPUT_angle = I_DOWN_RIGHT;
            else 
                p->INPUT_angle = INPUT_RIGHT;

            p->JOY_vel = 1;
        break;
        case KEY_UP:
            if (p->input_q[1] == KEY_LEFT) 
                p->INPUT_angle = I_UP_LEFT;
            else if (p->input_q[1] == KEY_RIGHT) 
                p->INPUT_angle = I_UP_RIGHT;
            else 
                p->INPUT_angle = INPUT_UP;

            p->JOY_vel = 1;
        break;
        case KEY_DOWN:
            if (p->input_q[1] == KEY_LEFT) 
                p->INPUT_angle = I_DOWN_LEFT;
            else if (p->input_q[1] == KEY_RIGHT) 
                p->INPUT_angle = I_DOWN_RIGHT;
            else 
                p->INPUT_angle = INPUT_DOWN;

            p->JOY_vel = 1;
        break;
        default:
            p->JOY_vel = 0;
        break;
    }

    switch (*p->aim_q)
    {
        case KEY_LEFT:
            if (p->aim_q[1] == KEY_UP) 
                p->AIM_angle = I_UP_LEFT;
            else if (p->aim_q[1] == KEY_DOWN) 
                p->AIM_angle = I_DOWN_LEFT;
            else 
                p->AIM_angle = INPUT_LEFT;
        break;
        case KEY_RIGHT:
            if (p->aim_q[1] == KEY_UP) 
                p->AIM_angle = I_UP_RIGHT;
            else if (p->aim_q[1] == KEY_DOWN) 
                p->AIM_angle = I_DOWN_RIGHT;
            else 
                p->AIM_angle = INPUT_RIGHT;
        break;
        case KEY_UP:
            if (p->aim_q[1] == KEY_LEFT) 
                p->AIM_angle = I_UP_LEFT;
            else if (p->aim_q[1] == KEY_RIGHT) 
                p->AIM_angle = I_UP_RIGHT;
            else 
                p->AIM_angle = INPUT_UP;
        break;
        case KEY_DOWN:
            if (p->aim_q[1] == KEY_LEFT) 
                p->AIM_angle = I_DOWN_LEFT;
            else if (p->aim_q[1] == KEY_RIGHT) 
                p->AIM_angle = I_DOWN_RIGHT;
            else 
                p->AIM_angle = INPUT_DOWN;
        break;
        default:
            if (p->JOY_vel) p->AIM_angle = p->INPUT_angle;
        break;
    }
}
void updateJoyInputs(P *p)
{
    if (p->JOY_ydir || p->JOY_xdir)
        p->INPUT_angle = SDL_atan2(p->JOY_ydir, p->JOY_xdir);

    if (p->AIM_ydir || p->AIM_xdir) 
        p->AIM_angle = SDL_atan2(p->AIM_ydir, p->AIM_xdir);
    else 
    {
        p->AIM_angle = p->INPUT_angle;
    }
}

void updatePlayerInputs(P *p)
{
    if (p->JOY_use) updateJoyInputs(p);
    else updateKeyInputs(p);
}

void updatePlayerX(P *p, L level)
{
    float x = p->x + p->xvel;

    if (!checkPlayerPosition(
        (int)x >> level.t_bit_size, 
        (int)p->y >> level.t_bit_size, 
        level.collision, 
        level.t_map_h)
    )
    {
        if (!(x < 0 || x > level.r.w)) p->x = x;
        else p->xvel = 0;
    }
    else p->xvel = 0;
}

void updatePlayerY(P *p, L level)
{
    float y = p->y + p->yvel;

    if (!checkPlayerPosition(
        (int)p->x >> level.t_bit_size, 
        (int)y >> level.t_bit_size, 
        level.collision, 
        level.t_map_h)
    )
    {
        if (!(y < 0 || y > level.r.h)) p->y = y;
        else p->yvel = 0;
    }
    else p->yvel = 0;
}

void addPlayerVel(float *vel, float wave)
{
    if (*vel > wave)
    {
        if ((*vel -= 0.25f) < wave) *vel = wave;
    }
    else if (*vel < wave)
    {
        if ((*vel += 0.25f) > wave) *vel = wave;
    }
}
void subPlayerVel(float *vel)
{
    if (*vel > 0)
    {
        if ((*vel -= 0.075f) < 0) *vel = 0;
    }
    else if (*vel < 0)
    {
        if ((*vel += 0.075f) > 0) *vel = 0;
    }
}

void updatePuck(P_TEST *pt, P players[])
{
    if (pt->PUCK_freeze && (++pt->PUCK_freeze_timer > 4))
    {
        pt->PUCK_freeze = false;
        pt->PUCK_freeze_timer = 0;
    }

    if (pt->puck.state == P_STATE_HIT 
    && (++pt->puck.hit_counter > 15)) 
    {
        pt->puck.state = P_STATE_NORMAL;
        pt->puck.hit_counter = 0;
    }

    if (pt->puck.xvel > 0)
    {
        pt->puck.xvel -= pt->puck.fvelx;
        if (pt->puck.xvel < 0) 
            pt->puck.xvel = 0;
    }
    else if (pt->puck.xvel < 0)
    {
        pt->puck.xvel += pt->puck.fvelx;
        if (pt->puck.xvel > 0) 
            pt->puck.xvel = 0;
    }
    
    if (pt->puck.yvel > 0)
    {
        pt->puck.yvel -= pt->puck.fvely;
        if (pt->puck.yvel < 0) 
            pt->puck.yvel = 0;
    }
    else if (pt->puck.yvel < 0)
    {
        pt->puck.yvel += pt->puck.fvely;
        if (pt->puck.yvel > 0) 
            pt->puck.yvel = 0;
    }

    if (pt->puck.xvel)
    {
        float px = pt->puck.x + pt->puck.xvel;
        bool check = false;

        if (pt->goalie[0].state == GK_NORMAL)
        {
            if (checkPuckCollision(px, pt->puck.y, pt->goalie[0].r))
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvelx += 0.5f;
                pt->puck.fvely += 0.01f;
            }
        }
        if (pt->goalie[1].state == GK_NORMAL && !check)
        {
            if (checkPuckCollision(px, pt->puck.y, pt->goalie[1].r))
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvelx += 0.5f;
                pt->puck.fvely += 0.01f;
            }
        }

        if (!check)
        {
            if (checkPlayerPosition(
                (int)px >> pt->level.t_bit_size, 
                (int)pt->puck.y >> pt->level.t_bit_size, 
                pt->level.collision, pt->level.t_map_h)
            )
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvelx += 0.5f;
                pt->puck.fvely += 0.01f;
            }
            else if (px < 0 || px > pt->level.r.w) 
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvelx += 0.5f;
                pt->puck.fvely += 0.01f;
            }
            else pt->puck.x = px;
        }
    }

    if (pt->puck.yvel)
    {
        float py = pt->puck.y + pt->puck.yvel;
        bool check = false;

        if (pt->goalie[0].state == GK_NORMAL)
        {
            if (checkPuckCollision(pt->puck.x, py, pt->goalie[0].r))
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvelx += 0.01f;
                pt->puck.fvely += 0.5f;
                check = true;
            }
        }
        if (pt->goalie[1].state == GK_NORMAL && !check)
        {
            if (checkPuckCollision(pt->puck.x, py, pt->goalie[1].r))
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvelx += 0.01f;
                pt->puck.fvely += 0.5f;
                check = true;
            }
        }

        if (!check)
        {
            if (checkPlayerPosition(
                (int)pt->puck.x >> pt->level.t_bit_size, 
                (int)py >> pt->level.t_bit_size, 
                pt->level.collision, pt->level.t_map_h)
            )
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvelx += 0.01f;
                pt->puck.fvely += 0.5f;
            }
            else if (py < 0 || py > pt->level.r.h) 
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvelx += 0.01f;
                pt->puck.fvely += 0.5f;
            }
            else pt->puck.y = py;
        }
    }

    if (!pt->puck.xvel && !pt->puck.yvel)
    {
        pt->puck.fvelx = 0.01f;
        pt->puck.fvely = 0.01f;
    }

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;
}

void updateGoalKeeperX(float *x, const int CHECKX)
{
    if (*x > CHECKX) 
    {
        if ((*x -= 1.125f) < CHECKX) *x = CHECKX;
    }
    else if (*x < CHECKX) 
    {
        if ((*x += 1.125f) > CHECKX) *x = CHECKX;
    }
}

void updateGoalKeepers(P_TEST *pt, P players[], bool grab)
{
    if (!pt->PUCK_freeze)
    {
        for (unsigned char g = 0; g < GOALS; g++)
        {
            switch (pt->goalie[g].state)
            {
                case GK_NORMAL:
                    if (!pt->puck.xvel && !pt->puck.yvel && !grab 
                    && pt->puck.state == P_STATE_NORMAL)
                    {
                        if (checkCollision(pt->puck.r, pt->gk_r[g]))
                        {
                            pt->goalie[g].state = GK_CLEAR_GOAL;
                            break;
                        }
                    }

                    float f = 0.85f;

                    if (g == 0)
                    {
                        updateGoalKeeperX(&pt->goalie[g].x, 154);
                        checkPuckXDistance(pt->puck.r.x, pt->goalie[g].x, &f);
                    }
                    else 
                    {
                        updateGoalKeeperX(&pt->goalie[g].x, 742);
                        checkPuckXDistance(pt->goalie[g].x, pt->puck.r.x, &f);
                    }

                    if (pt->puck.r.y < 112)
                    {
                        if (pt->goalie[g].y < 112) pt->goalie[g].y += f;
                        else adjustGoalieY(&pt->goalie[g].y, pt->puck.r.y, f);
                    }
                    else if (pt->puck.r.y > 160)
                    {
                        if (pt->goalie[g].y > 160) pt->goalie[g].y -= f;
                        else adjustGoalieY(&pt->goalie[g].y, pt->puck.r.y, f);
                    }
                    else adjustGoalieY(&pt->goalie[g].y, pt->puck.r.y, f);
                break;
                case GK_CLEAR_GOAL:
                    if (!grab)
                    {
                        if (checkCollision(pt->puck.r, pt->goalie[g].r))
                        {
                            pt->puck.x = pt->goalie[g].x;
                            pt->puck.y = pt->goalie[g].y;
                            pt->puck.state = P_STATE_GRAB;

                            pt->goalie[g].state = GK_GRAB;
                        }

                        if (g == 0)
                        {
                            if (pt->goalie[g].x < pt->puck.x)
                            {
                                if ((pt->goalie[g].x += 1.125f) > pt->puck.x)
                                    pt->goalie[g].x = pt->puck.x;
                            }
                        }
                        else 
                        {
                            if (pt->goalie[g].x > pt->puck.x)
                            {
                                if ((pt->goalie[g].x -= 1.125f) < pt->puck.x)
                                    pt->goalie[g].x = pt->puck.x;
                            }
                        }
                    }
                    else pt->goalie[g].state = GK_NORMAL;

                    if (pt->puck.y > pt->goalie[g].y)
                        pt->goalie[g].y += 1.125f;
                    else if (pt->puck.y < pt->goalie[g].y)
                        pt->goalie[g].y -= 1.125f;
                break;
                case GK_SHOOT:
                    if (++pt->goalie[g].s_timer > 10)
                    {
                        pt->goalie[g].s_timer = 0;
                        pt->goalie[g].state = GK_NORMAL;
                    }
                break;
                case GK_GRAB:
                    // swing/shoot?
                    // puck goes to center
                    if (++pt->goalie[g].s_timer > 10)
                    {
                        pt->puck.state = P_STATE_HIT;
                        pt->puck.xvel = g == 0 ? 2.5f : -2.5f;
                        pt->puck.yvel = 0;

                        pt->PUCK_freeze = true;

                        pt->goalie[g].s_timer = 0;
                        pt->goalie[g].state = GK_SHOOT;

                        //Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                    }
                break;
            }
            pt->goalie[g].r.x = pt->goalie[g].x - 10;
            pt->goalie[g].r.y = pt->goalie[g].y - 10;
        }
    }
}

void updateNetGoalKeepers(P_TEST *pt, bool grab)
{
    for (unsigned char g = 0; g < GOALS; g++)
    {
        switch (pt->goalie[g].state)
        {
            case GK_NORMAL:
                float f = 0.85f;

                if (g == 0)
                {
                    updateGoalKeeperX(&pt->goalie[g].x, 154);
                    checkPuckXDistance(pt->puck.r.x, pt->goalie[g].x, &f);
                }
                else 
                {
                    updateGoalKeeperX(&pt->goalie[g].x, 742);
                    checkPuckXDistance(pt->goalie[g].x, pt->puck.r.x, &f);
                }

                if (pt->puck.r.y < 112)
                {
                    if (pt->goalie[g].y < 112) pt->goalie[g].y += f;
                    else adjustGoalieY(&pt->goalie[g].y, pt->puck.r.y, f);
                }
                else if (pt->puck.r.y > 160)
                {
                    if (pt->goalie[g].y > 160) pt->goalie[g].y -= f;
                    else adjustGoalieY(&pt->goalie[g].y, pt->puck.r.y, f);
                }
                else adjustGoalieY(&pt->goalie[g].y, pt->puck.r.y, f);
            break;
            case GK_CLEAR_GOAL:
                if (!grab)
                {
                    if (g == 0)
                    {
                        if (pt->goalie[g].x < pt->puck.x)
                        {
                            if ((pt->goalie[g].x += 1.125f) > pt->puck.x)
                                pt->goalie[g].x = pt->puck.x;
                        }
                    }
                    else 
                    {
                        if (pt->goalie[g].x > pt->puck.x)
                        {
                            if ((pt->goalie[g].x -= 1.125f) < pt->puck.x)
                                pt->goalie[g].x = pt->puck.x;
                        }
                    }
                }

                if (pt->puck.y > pt->goalie[g].y)
                    pt->goalie[g].y += 1.125f;
                else if (pt->puck.y < pt->goalie[g].y)
                    pt->goalie[g].y -= 1.125f;
            break;
            case GK_SHOOT:
            break;
            case GK_GRAB:
                // maybe check if position is off?
            break;
        }
        pt->goalie[g].r.x = pt->goalie[g].x - 10;
        pt->goalie[g].r.y = pt->goalie[g].y - 10;
    }
}

void updateNetGame(P_TEST *pt, P plrs[])
{
    if (pt->network.localplayer != NULL)
    {
        if (pt->network.localplayer->id == HOST_ID)
            updateNetHostGame(pt, plrs);
        else 
            updateNetClientGame(pt, plrs);

        pt->network.localplayer->state = pt->c_player->state;
        pt->network.localplayer->x = pt->c_player->x;
        pt->network.localplayer->y = pt->c_player->y;
        pt->network.localplayer->angle = pt->c_player->AIM_angle;
    }

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;

    pt->rx = pt->puck.x;
    pt->ry = pt->puck.y;

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            plrs[i].AIM_deg = (plrs[i].AIM_angle * 180) / AIM_PI;

            if (plrs[i].AIM_deg < 0) plrs[i].AIM_deg += 360;

            setPlayerFace(plrs[i].AIM_deg, &plrs[i].facing);

            if (plrs[i].state == PLR_BLOCK)
            {
                switch (plrs[i].facing)
                {
                    case FACING_DOWN:
                        plrs[i].r.x = plrs[i].x - 8;
                        plrs[i].r.y = plrs[i].y;
                    break;
                    case FACING_UP:
                        plrs[i].r.x = plrs[i].x - 8;
                        plrs[i].r.y = plrs[i].y - 10;
                    break;
                    case FACING_RIGHT:
                        plrs[i].r.x = plrs[i].x - 10;
                        plrs[i].r.y = plrs[i].y + 5;
                    break;
                    case FACING_LEFT:
                        plrs[i].r.x = plrs[i].x - 20;
                        plrs[i].r.y = plrs[i].y + 5;
                    break;
                }
            }
            else 
            {
                plrs[i].r.x = plrs[i].x - 10;
                plrs[i].r.y = plrs[i].y;
            }

            if (plrs[i].state == PLR_SKATE_WP)
            {
                pt->rx = plrs[i].r.x;
                pt->ry = plrs[i].r.y;
            }

            plrs[i].AIM_radx = 10 * SDL_cos(plrs[i].AIM_angle);
            plrs[i].AIM_rady = 10 * SDL_sin(plrs[i].AIM_angle);

            plrs[i].club_r.x = plrs[i].r.x + plrs[i].AIM_radx;
            plrs[i].club_r.y = plrs[i].r.y + plrs[i].AIM_rady;

            plrs[i].crosshair.r.x = plrs[i].club_r.x + 10 - pt->camera.x;
            plrs[i].crosshair.r.y = plrs[i].club_r.y + 10 - pt->camera.y;

            animatePlayer(&plrs[i]);
        }
    }
}

void updateNetPlayers(P_TEST *pt, P plrs[])
{
    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            if (plrs[i].id == pt->network.players_net[i].id)
            {
                if (pt->network.players_net[i].id != pt->c_player->id)
                {
                    if (pt->network.players_net[i].x != plrs[i].x || 
                    pt->network.players_net[i].y != plrs[i].y)
                    {
                        plrs[i].input_q[0] = 1;
                    }
                    else plrs[i].input_q[0] = 0;

                    plrs[i].x = pt->network.players_net[i].x;
                    plrs[i].y = pt->network.players_net[i].y;
                    plrs[i].AIM_angle = pt->network.players_net[i].angle;
                    plrs[i].state = pt->network.players_net[i].state;
                }
                else if (pt->network.players_net[i].id == pt->c_player->id)
                {
                    if (pt->p_state == P_DROP)
                    {
                        pt->c_player->x = pt->network.players_net[i].x;
                        pt->c_player->y = pt->network.players_net[i].y;
                    }
                    else 
                    {
                        if (pt->network.players_net[i].state == PLR_SHOOT)
                        {
                            printf("PLAYER: %d shoot from host\n", pt->c_player->id);
                            plrs[i].gvel -= pt->c_player->gvel * 0.25f;
                            plrs[i].state = PLR_SWING;
                            //pt->puck.state = P_STATE_HIT;
                        }
                    }
                }

                if (plrs[i].id == pt->network.puck.id)
                {
                    if (plrs[i].state == PLR_SKATE_NP 
                    || plrs[i].state == PLR_SPRINT) 
                    {
                        plrs[i].state = PLR_SKATE_WP;
                        plrs[i].sprint_cdown_timer = 0;
                        plrs[i].gvel = STANDARD_VELOCITY;
                    }

                    pt->puck.state = P_STATE_GRAB;
                }
                else if (!pt->network.puck.id)
                {
                    if (plrs[i].state == PLR_SKATE_WP)
                        plrs[i].state = PLR_SKATE_NP;
                    else if (plrs[i].state == PLR_SHOOT 
                    || plrs[i].state == PLR_SHOOT_MAX)
                    {
                        plrs[i].m_hold = false;
                        plrs[i].swing_timer = 0;
                        plrs[i].state = PLR_SWING; 
                    }

                    pt->puck.state = P_STATE_NORMAL;
                }
            }
        }
    }
}

void updateNetHostGame(P_TEST *pt, P plrs[])
{
    unsigned char i = 0;

    for (; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            if (plrs[i].id == pt->network.players_net[i].id)
            {
                if (pt->network.players_net[i].id != pt->c_player->id)
                {
                    if (pt->p_state != P_DROP)
                    {
                        if (!(pt->network.players_net[i].state == PLR_SKATE_WP 
                        && pt->puck.state == P_STATE_NORMAL))
                            plrs[i].state = pt->network.players_net[i].state;
                        else 
                            plrs[i].state = PLR_SKATE_NP;

                        if (pt->network.players_net[i].x != plrs[i].x 
                        || pt->network.players_net[i].y != plrs[i].y)
                        {
                            plrs[i].input_q[0] = 1;
                        }
                        else plrs[i].input_q[0] = 0;
                    
                        plrs[i].x = pt->network.players_net[i].x;
                        plrs[i].y = pt->network.players_net[i].y;
                    }

                    plrs[i].AIM_angle = pt->network.players_net[i].angle;
                }
            }
        }
    }

    switch (pt->p_state)
    {
        case P_DROP:
            if (!pt->state_change)
            {
                pt->network.puck.id = 0;

                resetPuck(
                    &pt->puck, 
                    pt->level.r.w >> 1, 
                    pt->level.r.h >> 1);

                for (i = 0; i < MAX_GAME_USERS; i++)
                {
                    if (plrs[i].spawned)
                    {
                        resetPlayer(
                            &plrs[i], 
                            pt->level.r.w >> 1, 
                            (pt->level.r.h >> 1) + 40);
                    }
                }

                pt->state_change = true;
            }
            
            if (++pt->STATE_timer > 180)
            {
                pt->STATE_timer = 0;
                pt->state_change = false;
                pt->p_state = P_PLAY;
            }
            else 
            {
                for (i = 0; i < MAX_GAME_USERS; i++)
                {
                    if (plrs[i].spawned)
                    {
                        plrs[i].x = pt->level.r.w >> 1;
                        plrs[i].y = (pt->level.r.h >> 1) + 40;
                    }
                }
            }
        break;
        case P_PLAY:
            for (i = 0; i < GOALS; i++)
            {
                if (checkGoal(pt->puck.r, pt->goal_r[i]))
                {
                    if (i == 0) pt->score.score1++;
                    else if (i == 1) pt->score.score2++;
                    pt->p_state = P_GOAL;
                }
            }
        break;
        case P_GOAL:
            if (++pt->SCORE_timer > 180)
            {
                pt->SCORE_timer = 0;
                pt->p_state = P_DROP;
            }
        break;
    }

    if (pt->c_player->state != PLR_BLOCK) 
        updatePlayerInputs(pt->c_player);

    if (pt->c_player->JOY_vel && pt->c_player->state != PLR_SHOOT_MAX)
    {
        float   cos = (
            SDL_cos(pt->c_player->INPUT_angle) * pt->c_player->gvel),
                sin = (
            SDL_sin(pt->c_player->INPUT_angle) * pt->c_player->gvel);

        addPlayerVel(&pt->c_player->xvel, cos);
        addPlayerVel(&pt->c_player->yvel, sin);
    }
    else
    {
        if (pt->c_player->state != PLR_SPRINT)
        {
            subPlayerVel(&pt->c_player->xvel);
            subPlayerVel(&pt->c_player->yvel);
        }
    }

    if (pt->c_player->xvel) 
        updatePlayerX(pt->c_player, pt->level);

    if (pt->c_player->yvel) 
        updatePlayerY(pt->c_player, pt->level);

    if (pt->c_player->m_hold)
    {
        if (++pt->c_player->swing_timer > 30)
        {
            pt->c_player->swing_timer = 31;
            pt->c_player->state = PLR_SHOOT_MAX;
        }
    }

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            switch (plrs[i].state)
            {
                default: break;
                case PLR_SPRINT:
                    if (plrs[i].id == HOST_ID)
                    {
                        if ((++plrs[i].sprint_cdown_timer) > 60)
                        {
                            plrs[i].sprint_cdown_timer = 0;
                            plrs[i].gvel = STANDARD_VELOCITY;
                            plrs[i].state = PLR_SKATE_NP;
                        }
                    }
                case PLR_SKATE_NP:
                    if (pt->c_player->state_wait)
                    {
                        if (++pt->c_player->state_timer > 10)
                        {
                            pt->c_player->state_timer = 0;
                            pt->c_player->state_wait = false;
                        }
                    }

                    if (pt->puck.state == P_STATE_NORMAL && pt->p_state == P_PLAY)
                    {
                        if (checkCollision(plrs[i].r, pt->puck.r))
                        {
                            printf("PLAYER: %d pick up puck\n", plrs[i].id);
                            pt->puck.state = P_STATE_GRAB;
                            pt->puck.xvel = 0;
                            pt->puck.yvel = 0;

                            plrs[i].sprint_cdown_timer = 0;
                            plrs[i].gvel = STANDARD_VELOCITY;
                            plrs[i].state = PLR_SKATE_WP;

                            pt->network.puck.id = plrs[i].id;
                        }
                    }

                    plrs[i].r.w = 20;
                    plrs[i].r.h = 20;
                break;
                case PLR_SKATE_WP:
                    pt->puck.x = plrs[i].x;
                    pt->puck.y = plrs[i].y;
                    plrs[i].r.w = 20;
                    plrs[i].r.h = 20;
                break;
                case PLR_SHOOT:
                    if (pt->puck.state == P_STATE_GRAB)
                    {
                        printf("PLAYER: %d shoot puck\n", plrs[i].id);
                    
                        shootPuck(
                            &pt->puck, 
                            plrs[i].pvel, 
                            plrs[i].x - 4, plrs[i].y + 10, 
                            plrs[i].AIM_angle
                        );

                        pt->PUCK_freeze = true; 
                        
                        plrs[i].gvel -= plrs[i].gvel * 0.25f;
                        plrs[i].pvel = MIN_SHOOT_VELOCITY;
                        plrs[i].state_timer = 0;
                        plrs[i].state = PLR_SWING;

                        pt->network.puck.id = 0;

                        //Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                    }

                    plrs[i].r.w = 20;
                    plrs[i].r.h = 20;
                break;
                case PLR_SHOOT_MAX: // make sure player has the highest shoot velocity
                    plrs[i].pvel = MAX_SHOOT_VELOCITY;

                    if (++plrs[i].state_timer > 7) 
                        plrs[i].state_timer = 0;

                    plrs[i].r.w = 20;
                    plrs[i].r.h = 20;
                break;
                case PLR_SWING:
                    if (pt->puck.state == P_STATE_GRAB)
                    {
                        for (unsigned char j = 0; j < MAX_GAME_USERS; j++)
                        {
                            if ((plrs[j].state == PLR_SKATE_WP 
                            || plrs[j].state == PLR_SHOOT_MAX)
                            && checkCollision(plrs[i].club_r, plrs[j].r))
                            {
                                printf(
                                    "PLAYER: p: %d hit the puck off p: %d\n", 
                                    plrs[i].id, plrs[j].id);

                                // puck goes wild
                                shootPuck(
                                    &pt->puck, 1.5f, 
                                    plrs[j].x - 4, plrs[j].y, 
                                    plrs[i].AIM_angle);

                                plrs[j].state = PLR_SKATE_NP;

                                pt->PUCK_freeze = true;
                                pt->network.puck.id = 0;

                                //Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                                break;
                            }
                        }
                    }

                    if (plrs[i].id == HOST_ID)
                    {
                        if ((++pt->c_player->swing_timer) > 10)
                        {
                            pt->c_player->swing_timer = 0;
                            pt->c_player->gvel = STANDARD_VELOCITY;
                            pt->c_player->state = PLR_SKATE_NP;
                        }
                    }

                    plrs[i].r.w = 20;
                    plrs[i].r.h = 20;
                break;
                case PLR_BLOCK:
                    if (checkCollision(plrs[i].r, pt->puck.r))
                    {
                        pt->puck.xvel = -pt->puck.xvel;
                        pt->puck.yvel = -pt->puck.yvel;
                        pt->puck.fvelx += 0.1f;
                        pt->puck.fvely += 0.1f;
                    }

                    switch (plrs[i].facing)
                    {
                        case FACING_DOWN:
                            plrs[i].r.w = 16;
                            plrs[i].r.h = 30;
                        break;
                        case FACING_UP:
                            plrs[i].r.w = 16;
                            plrs[i].r.h = 30;
                        break;
                        case FACING_RIGHT:
                            plrs[i].r.w = 30;
                            plrs[i].r.h = 16;
                        break;
                        case FACING_LEFT:
                            plrs[i].r.w = 30;
                            plrs[i].r.h = 16;
                        break;
                    }

                    if (plrs[i].id == HOST_ID)
                    {
                        plrs[i].JOY_vel = 1;

                        if ((plrs[i].gvel -= 0.05f) < 0) plrs[i].gvel = 0;

                        if ((++pt->c_player->block_timer) > 60)
                        {
                            plrs[i].block_timer = 0;
                            plrs[i].bounce = false;
                            plrs[i].gvel = STANDARD_VELOCITY;
                            plrs[i].r.w = 20;
                            plrs[i].r.h = 20;
                            plrs[i].state = PLR_SKATE_NP;
                        }
                    }
                break;
            }

            for (unsigned char j = 0; j < pt->network.numplayers; j++)
            {
                if (plrs[i].id == pt->network.players_net[j].id)
                {
                    pt->network.players_net[j].angle = plrs[i].AIM_angle;
                    pt->network.players_net[j].x = plrs[i].x;
                    pt->network.players_net[j].y = plrs[i].y;
                    pt->network.players_net[j].state = plrs[i].state;
                }
            }
        }
    }

    updatePuck(pt, plrs);
    updateGoalKeepers(pt, plrs, pt->network.puck.id);

    pt->network.play_state = pt->p_state;

    pt->network.score.score1 = pt->score.score1;
    pt->network.score.score2 = pt->score.score2;

    pt->network.goalkeepers.gk1_status = pt->goalie[0].state;
    pt->network.goalkeepers.gk2_status = pt->goalie[1].state;

    pt->network.puck.x = pt->puck.x;
    pt->network.puck.y = pt->puck.y;
}

void updateNetClientGame(P_TEST *pt, P plrs[])
{
    pt->p_state = pt->network.play_state;

    pt->score.score1 = pt->network.score.score1;
    pt->score.score2 = pt->network.score.score2;
    
    pt->goalie[0].state = pt->network.goalkeepers.gk1_status;
    pt->goalie[1].state = pt->network.goalkeepers.gk2_status;

    pt->puck.x = pt->network.puck.x;
    pt->puck.y = pt->network.puck.y;

    updateNetPlayers(pt, plrs);
    updateNetGoalKeepers(pt, pt->network.puck.id);

    if (pt->c_player->state != PLR_BLOCK) 
        updatePlayerInputs(pt->c_player);

    if (pt->c_player->JOY_vel && pt->c_player->state != PLR_SHOOT_MAX)
    {
        float   cos = (
            SDL_cos(pt->c_player->INPUT_angle) * pt->c_player->gvel),
                sin = (
            SDL_sin(pt->c_player->INPUT_angle) * pt->c_player->gvel);

        addPlayerVel(&pt->c_player->xvel, cos);
        addPlayerVel(&pt->c_player->yvel, sin);
    }
    else
    {
        if (pt->c_player->state != PLR_SPRINT)
        {
            subPlayerVel(&pt->c_player->xvel);
            subPlayerVel(&pt->c_player->yvel);
        }
    }

    if (pt->c_player->xvel) 
        updatePlayerX(pt->c_player, pt->level);

    if (pt->c_player->yvel) 
        updatePlayerY(pt->c_player, pt->level);

    if (pt->c_player->state_wait)
    {
        if (++pt->c_player->state_timer > 10)
        {
            pt->c_player->state_timer = 0;
            pt->c_player->state_wait = false;
        }
    }

    if (pt->c_player->m_hold)
    {
        if (++pt->c_player->swing_timer > 30)
        {
            pt->c_player->swing_timer = 31;
            pt->c_player->state = PLR_SHOOT_MAX;
        }
    }

    switch (pt->c_player->state)
    {
        default: break;
        case PLR_SKATE_NP:
            if (pt->c_player->state_wait)
            {
                if (++pt->c_player->state_timer > 10)
                {
                    pt->c_player->state_timer = 0;
                    pt->c_player->state_wait = false;
                }
            }
        break;
        case PLR_SHOOT: // do i need this client side?
        break;
        case PLR_SHOOT_MAX:
            if (++pt->c_player->state_timer > 7) 
                pt->c_player->state_timer = 0;
        break;
        case PLR_SWING:
            if ((++pt->c_player->swing_timer) > 10)
            {
                pt->c_player->swing_timer = 0;
                pt->c_player->gvel = STANDARD_VELOCITY;
                pt->c_player->state = PLR_SKATE_NP;
            }
        break;
        case PLR_SPRINT:
            if ((++pt->c_player->sprint_cdown_timer) > 60)
            {
                pt->c_player->sprint_cdown_timer = 0;
                pt->c_player->gvel = STANDARD_VELOCITY;
                pt->c_player->state = PLR_SKATE_NP;
            }
        break;
        case PLR_BLOCK:
            pt->c_player->JOY_vel = 1;

            if ((pt->c_player->gvel -= 0.05f) < 0) pt->c_player->gvel = 0;

            if ((++pt->c_player->block_timer) > 60)
            {
                pt->c_player->block_timer = 0;
                pt->c_player->bounce = false;
                pt->c_player->gvel = STANDARD_VELOCITY;
                pt->c_player->state = PLR_SKATE_NP;
            }
        break;
    }

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            switch (plrs[i].state)
            {
                default: 
                    plrs[i].r.w = 20;
                    plrs[i].r.h = 20;
                break;
                case PLR_BLOCK:
                    switch (plrs[i].facing)
                    {
                        case FACING_DOWN:
                            plrs[i].r.w = 16;
                            plrs[i].r.h = 30;
                        break;
                        case FACING_UP:
                            plrs[i].r.w = 16;
                            plrs[i].r.h = 30;
                        break;
                        case FACING_RIGHT:
                            plrs[i].r.w = 30;
                            plrs[i].r.h = 16;
                        break;
                        case FACING_LEFT:
                            plrs[i].r.w = 30;
                            plrs[i].r.h = 16;
                        break;
                    }
                break;
            }
        }
    }
}

void renderPlayer(SDL_Renderer *r, SDL_Texture *t, P *p, int cx, int cy)
{
    // ONLY for testing
    SDL_Rect renderQuad = {
        p->x - (p->clip.w >> 1) - cx, 
        p->y - (p->clip.h >> 1) - cy, 
        p->clip.w, 
        p->clip.h
    };

    if (p->facing == 3) // flip
        SDL_RenderCopyEx(r, t, &p->t_clips[p->c_index], &renderQuad, 0, NULL, SDL_FLIP_HORIZONTAL);
    else 
        SDL_RenderCopy(r, t, &p->t_clips[p->c_index], &renderQuad);
}

void renderPlayTiles(SDL_Renderer *renderer, P_TEST pt)
{
    SDL_Rect r = {
        .x = 0, 
        .y = 0, 
        .w = pt.level.t_size, 
        .h = pt.level.t_size
    };

    for (int i = 0; i < (pt.level.t_map_pieces << 1); i+=2) 
    {
        if (checkCollision(r, pt.camera))
        {
           r.x -= pt.camera.x;
           r.y -= pt.camera.y;

            SDL_RenderCopy(
                renderer, 
                pt.texture.t, 
                &pt.t_clips[pt.level.map[i] + pt.level.map[i + 1]],
                &r);

            r.x += pt.camera.x;
            r.y += pt.camera.y;
        }

        r.x += r.w;

        if (r.x >= pt.level.r.w)
        {
            r.x = 0;
            r.y += r.h;
        }
    }
}

void renderMenuButton(SDL_Renderer *r, SDL_Rect *btn, bool select)
{
    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(r, btn);

    if (select)
    {
        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
        SDL_RenderDrawRect(r, btn);
    }
    else 
    {
        SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderDrawRect(r, btn);
    }
}

void setupPlay(P_TEST *pt, P *player)
{
    pt->players = player;
    pt->c_player = player;

    pt->c_player->spawned = true;

    pt->camera.x = pt->c_player->x - (pt->camera.w >> 1);
    pt->camera.y = pt->c_player->y - (pt->camera.h >> 1);
    
    pt->p_state = P_PLAY;

    pt->c_player->club_r.x = pt->c_player->x + pt->c_player->AIM_radx;
    pt->c_player->club_r.y = pt->c_player->y + pt->c_player->AIM_rady;

    pt->puck.x = pt->level.r.w >> 1;
    pt->puck.y = pt->level.r.h >> 1;

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;
    pt->puck.r.w = 8;
    pt->puck.r.h = 4;

    pt->puck.xvel = 0;
    pt->puck.yvel = 0;
    pt->puck.state = P_STATE_NORMAL;
    pt->puck.hit_counter = 0;

    pt->c_player->r.x = pt->c_player->x - pt->camera.x;
    pt->c_player->r.y = pt->c_player->y - pt->camera.y;

    pt->g_state = G_MAIN;
}

void setupGame(P_TEST *pt, SDL_Rect *gr, SDL_Rect *gkr, P_G *gkeep)
{
    pt->quit = false;

    pt->is_net = false;
    pt->state_change = false;
    pt->playing = false;
    pt->menu_hold = false;

    pt->JOY_select = 0;
    pt->SCORE_timer = 0;
    pt->STATE_timer = 0;
    pt->STATE_cntdwn = 3;

    pt->PUCK_freeze = false;
    pt->PUCK_freeze_timer = 0;

    for (unsigned char i = 0; i < 64; i++)
        pt->input_field.string[i] = 0;

    pt->input_field.str_pointer = 0;
    pt->input_field.str_len = 0;

    pt->score.score1 = 0;
    pt->score.score2 = 0;

    n_itoa(pt->score.score1, pt->score.score1_string);
    n_itoa(pt->score.score2, pt->score.score2_string);

    n_itoa(pt->STATE_cntdwn, pt->state_timer_string);

    pt->goal_r = gr;
    pt->gk_r = gkr;
    pt->goalie = gkeep;
    
    pt->sprint_hud_r.w = 0;
    pt->sprint_hud_r.h = 16;
    pt->sprint_hud_r.x = 300;
    pt->sprint_hud_r.y = W_HEIGHT - 26;

    pt->screen.w = W_WIDTH;
    pt->screen.h = W_HEIGHT;
    pt->screen.x = 0;
    pt->screen.y = 0;

    pt->camera.w = W_WIDTH;
    pt->camera.h = W_HEIGHT;
    pt->camera.x = 0;
    pt->camera.y = 0;

    pt->w_focus = false;
}

void setupGoals(SDL_Rect *r)
{
    r[0].x = 96;
    r[0].y = 112;
    r[0].w = 48;
    r[0].h = 48;

    r[1].x = 752;
    r[1].y = 112;
    r[1].w = 48;
    r[1].h = 48;
}

void setupGoalKeepers(SDL_Rect *r, P_G *goalie)
{
    r[0].x = 144;
    r[0].y = 80;
    r[0].w = 48;
    r[0].h = 112;

    r[1].x = 704;
    r[1].y = 80;
    r[1].w = 48;
    r[1].h = 112;

    initGoalkeeper(&goalie[0]);
    initGoalkeeper(&goalie[1]);

    goalie[0].id = 'l';
    goalie[1].id = 'r';

    goalie[0].x = 154;
    goalie[1].x = 742;
}

void setupPlayerHud(SDL_Rect r[])
{
    r[0].w = 64;
    r[0].h = 64;
    r[0].x = 0;
    r[0].y = 0;

    r[1].w = 64;
    r[1].h = 64;
    r[1].x = W_WIDTH - 64;
    r[1].y = 0;

    r[2].w = 64;
    r[2].h = 64;
    r[2].x = 0;
    r[2].y = W_HEIGHT - 64;

    r[3].w = 64;
    r[3].h = 64;
    r[3].x = W_WIDTH - 64;
    r[3].y = W_HEIGHT - 64;
}

void addPlayerGame(P *p, unsigned char id, int x, int y)
{
    resetPlayer(p, 0, 0);
    p->id = id;
    p->x = x;
    p->y = y;
    p->spawned = true;
}

void removePlayerGame(P *p)
{
    p->id = 0;
    p->x = 0;
    p->y = 0;
    p->spawned = false;
}

void resetPlay(P_TEST *pt, P plrs[], bool id)
{
    resetInputField(&pt->input_field);
    resetPuck(&pt->puck, pt->level.r.w >> 1, pt->level.r.h >> 1);

    for (unsigned i = 0; i < MAX_GAME_USERS; i++)
    {
        resetPlayer(
            &plrs[i],
            pt->level.r.w >> 1, 
            pt->level.r.h >> 1);

        if (id)
        {
            pt->players[i].id = 0;
            pt->players[i].ctrl_id = JOY_ID_NULL;
            pt->players[i].spawned = false;
        }
    }
}

void resetPlayer(P *p, int sx, int sy)
{
    for (int i = 0; i < 4; i++)
        p->input_q[i] = 0;

    p->state = PLR_SKATE_NP;

    p->AIM_angle = 4;
    p->AIM_deg = 0;
    p->AIM_radx = 0;
    p->AIM_rady = 0;
    p->AIM_done = false;
    p->AIM_timer = 0;

    p->crosshair.show = false;

    p->JOY_vel = 0;
    p->JOY_xdir = 0;
    p->JOY_ydir = 0;

    p->gvel = STANDARD_VELOCITY;
    p->vel = 0;
    p->xvel = 0;
    p->yvel = 0;
    p->pvel = MIN_SHOOT_VELOCITY;

    p->sprint = false;
    p->sprint_cdown = false;
    p->m_hold = false;
    p->m_move = false;
    p->bounce = false;
    p->state_wait = false;

    p->sprint_timer = 0;
    p->sprint_cdown_timer = 0;
    p->block_timer = 0;
    p->swing_timer = 0;
    p->state_timer = 0;

    p->c_index = 0;
    p->facing = 0;
    p->a_index = 0;

    p->x = sx;
    p->y = sy;
    p->r.x = p->x;
    p->r.y = p->y;

    p->club_r.x = p->x + p->AIM_radx;
    p->club_r.y = p->y + p->AIM_rady;
}

void resetPuck(Puck *p, int mx, int my)
{
    p->x = mx;
    p->y = my;
    p->r.x = p->x;
    p->r.y = p->y;
    p->xvel = 0;
    p->yvel = 0;
    p->fvelx = 0.01f;
    p->fvely = 0.01f;
    p->state = P_STATE_NORMAL;
    p->hit_counter = 0;
}

void resetInputField(I_FIELD *input)
{
    for (unsigned char i = 0; i < 64; i++)
        input->string[i] = 0;

    input->str_len = 0;
    input->str_pointer = 0;
}

void resetScores(P_SCORE *scores)
{
    scores->score1 = 0;
    scores->score2 = 0;
}

void resetGoalkeeper(P_G *g, int x)
{
    g->x = x;
    g->y = 112;
    g->s_timer = 0;
    g->state = GK_NORMAL;
}
