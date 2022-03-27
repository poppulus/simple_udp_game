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
        rightA =  a.x + a.w, 
        rightB = b.x + b.w, 
        topA = a.y, 
        topB = b.y, 
        bottomA = a.y + a.h, 
        bottomB = b.y + b.h;

    //If any of the sides from A are outside of B
    if( bottomA <= topB ) return false;
    if( topA >= bottomB ) return false;
    if( rightA <= leftB ) return false;
    if(leftA >= rightB) return false;

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
    // if one of the sides are outside of the goal, its false
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
    freeTexture(&ptest->texture);

    printf("free texture map\n");

    if (ptest->gunTexture != NULL)
        freeTexture(ptest->gunTexture);

    printf("free gun texture\n");

    SDL_GameControllerClose(ptest->controller);
    ptest->controller = NULL;

    printf("free controller\n");
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
        p->input_q[i] = 0;

    p->id = 0;

    p->state = PLR_SKATE_NP;

    p->AIM_angle = 4;
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
    p->vel = STANDARD_VELOCITY;
    p->xvel = 0;
    p->yvel = 0;
    p->pvel = 2;

    p->sprint = false;
    p->sprint_cdown = false;
    p->spawned = true;
    p->m_hold = false;
    p->m_move = false;
    p->bounce = false;

    p->sprint_timer = 0;
    p->sprint_cdown_timer = 0;
    p->block_timer = 0;

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
    btns[0].w = 320;
    btns[0].h = 40;
    btns[0].x = (W_WIDTH >> 1) - 160;
    btns[0].y = (W_HEIGHT >> 1) - 80;

    btns[1].w = 320;
    btns[1].h = 40;
    btns[1].x = (W_WIDTH >> 1) - 160;
    btns[1].y = (W_HEIGHT >> 1);

    btns[2].w = 320;
    btns[2].h = 40;
    btns[2].x = (W_WIDTH >> 1) - 160;
    btns[2].y = (W_HEIGHT >> 1) + 80;
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
    g->x = 154;
    g->y = 112;
    g->r.w = 20;
    g->r.h = 20;
    g->s_timer = 0;
    g->grab = false;
    g->shoot = false;
    g->swing = false;
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

void renderGame(SDL_Renderer *r, FC_Font *f, P_TEST *pt, P players[])
{
    // background 
    SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(r, &pt->screen);

    // tiles
    renderPlayTiles(r, *pt);

    //sprint cooldown
    if (pt->c_player->sprint_cdown)
    {
        SDL_SetRenderDrawColor(r, 0x00, 0x00, 0xff, 0xff);
        SDL_RenderFillRect(r, &pt->sprint_hud_r);
    }

    unsigned char i = 0;

    for (; i < 2; i++)
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
        if (players[i].spawned)
        {
            if (!pt->is_net)
            {
                SDL_Rect cq = {
                    players[i].club_r.x - pt->camera.x, 
                    players[i].club_r.y - pt->camera.y, 
                    players[i].club_r.w, 
                    players[i].club_r.h
                },
                plq = {
                    players[i].r.x - pt->camera.x, 
                    players[i].r.y - pt->camera.y, 
                    players[i].r.w, 
                    players[i].r.h
                };

                switch (players[i].state)
                {
                    case PLR_SHOOT:
                        // player hitbox
                        SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
                        SDL_RenderFillRect(r, &plq);
                        // club hitbox
                        SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
                        SDL_RenderFillRect(r, &cq);
                    break;
                    case PLR_SWING:
                        // player hitbox
                        SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
                        SDL_RenderFillRect(r, &plq);
                        // club hitbox
                        SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
                        SDL_RenderFillRect(r, &cq);
                    break;
                    case PLR_BLOCK:
                        // player hitbox
                        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
                        SDL_RenderFillRect(r, &plq);
                    break;
                    default:
                        // player hitbox
                        SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
                        SDL_RenderFillRect(r, &plq);
                        // club hitbox
                        if (players[i].pvel == MAX_SHOOT_VELOCITY)
                            SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
                        else 
                            SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);

                        SDL_RenderFillRect(r, &cq);
                    break;
                }
            }
            
            // player texture
            if (checkCollision(players[i].r, pt->camera))
            {
                FC_DrawColor(
                    f, r, 
                    players[i].r.x - pt->camera.x, 
                    players[i].r.y - 32 - pt->camera.y, 
                    FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                    "P%d", i + 1);

                renderPlayer(
                    r, 
                    players[i].texture->t,
                    &players[i], 
                    pt->camera.x, 
                    pt->camera.y);
            }
            else 
            {
                int nx = players[i].r.x - pt->camera.x, 
                    ny = players[i].r.y - 32 - pt->camera.y;

                if (players[i].r.x + players[i].r.w < pt->camera.x)
                {
                    if (ny < 0) ny = 10;
                    else if (ny > W_HEIGHT - 32) ny = W_HEIGHT - 32;

                    FC_DrawColor(
                        f, r, 
                        10, ny, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
                else if (players[i].r.x > pt->camera.x + pt->camera.w)
                {
                    if (ny < 0) ny = 10;
                    else if (ny > W_HEIGHT - 32) ny = W_HEIGHT - 32;

                    FC_DrawColor(
                        f, r, 
                        W_WIDTH - 32, ny, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
                else if (players[i].r.y + players[i].r.h < pt->camera.y)
                {
                    if (nx < 0) nx = 10;
                    else if (nx > W_WIDTH - 32) nx = W_WIDTH - 32;

                    FC_DrawColor(
                        f, r, 
                        nx, 10, 
                        FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
                        "P%d", i + 1);
                }
                else if (players[i].r.y > pt->camera.y + pt->camera.h)
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
            /*
            if (players[i].crosshair.show) 
            {
                SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderFillRect(r, &players[i].crosshair.r);
            }
            */
        }
    }

    if (!pt->puck.grab)
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

    if (pt->p_state == P_GOAL)
    {
        FC_DrawColor(
            f, r, 
            320, 20, 
            FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
            "SCORE!");
    }

    if (pt->g_state == G_MENU)
    {
        if (!pt->is_net)
        {
            for (i = 0; i < 3; i++)
            {
                SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderFillRect(r, &pt->buttons[i]);

                SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
                SDL_RenderDrawRect(r, &pt->buttons[i]);

                switch (i)
                {
                    case 0:
                        FC_Draw(
                            f, r, 
                            (pt->buttons[i].x + 128), 
                            pt->buttons[i].y + 10, 
                            "Host");
                    break;
                    case 1:
                        FC_Draw(
                            f, r, 
                            (pt->buttons[i].x + 128), 
                            pt->buttons[i].y + 10, 
                            "Join");
                    break;
                    case 2:
                        FC_Draw(
                            f, r, 
                            (pt->buttons[i].x + 128), 
                            pt->buttons[i].y + 10, 
                            "Exit");
                    break;
                }
            }
        }
        else
        {   
            for (i = 1; i < 3; i++)
            {
                SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderFillRect(r, &pt->buttons[i]);

                SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
                SDL_RenderDrawRect(r, &pt->buttons[i]);

                switch (i)
                {
                    case 1:
                        FC_Draw(
                            f, r, 
                            (pt->buttons[1].x + 88), 
                            pt->buttons[1].y + 10, 
                            "Disconnect"); 
                    break;
                    case 2:
                        FC_Draw(
                            f, r, 
                            (pt->buttons[2].x + 128), 
                            pt->buttons[2].y + 10, 
                            "Exit");
                    break;
                }
            }
        }
    }
    else if (pt->g_state == G_JOIN || pt->g_state == G_HOST)
    {
        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderFillRect(r, &pt->buttons[1]);

        SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderDrawRect(r, &pt->buttons[1]);

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
    else if (pt->g_state == G_HOSTING)
    {
        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderFillRect(r, &pt->buttons[1]);

        FC_Draw(
            f, r, 
            pt->buttons[1].x, 
            pt->buttons[1].y + 10, 
            "Hosting setup ...");
    }
    else if (pt->g_state == G_JOINING)
    {
        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderFillRect(r, &pt->buttons[1]);

        FC_Draw(
            f, r, 
            pt->buttons[1].x, 
            pt->buttons[1].y + 10, 
            "Joining setup ...");
    }
}

void renderClientGame(SDL_Renderer *r, SDL_Rect cam, P players[])
{
    for (unsigned char i = 0; i < MAX_NET_USERS; i++)
    {
        if (players[i].spawned)
        {
            renderPlayer(
                r, 
                players[i].texture->t,
                &players[i], 
                cam.x, 
                cam.y);
        }
    }
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
    if (*p->dir)
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
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT:
                pt->quit = true;
            break;
            case SDL_KEYDOWN:
                if (!ev.key.repeat)
                {
                    pt->c_player->JOY_use = false;

                    switch (ev.key.keysym.sym)
                    {
                        case SDLK_ESCAPE: 
                            if (pt->g_state == G_PLAY)
                            {
                                SDL_SetRelativeMouseMode(SDL_DISABLE);
                                pt->w_focus = false;
                                pt->g_state = G_MENU;
                            }
                            else if (pt->g_state == G_MENU)
                            {
                                SDL_SetRelativeMouseMode(SDL_ENABLE);
                                pt->w_focus = true;
                                pt->g_state = G_PLAY;
                            }
                        break;
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                        case SDLK_RETURN2:
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
                        case SDLK_SPACE:
                        case SDLK_KP_SPACE:
                            if (!pt->c_player->sprint_cdown 
                            && pt->c_player->state != PLR_SWING 
                            && pt->c_player->state != PLR_SHOOT) 
                            {
                                pt->c_player->sprint = true;
                                pt->c_player->gvel = SPRINT_VELOCITY;
                                pt->c_player->vel = SPRINT_VELOCITY;
                            }
                        break;
                    }
                }
            break;
            case SDL_KEYUP:
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
                    case SDLK_SPACE:
                    case SDLK_KP_SPACE:
                    break;
                }
            break;
            case SDL_MOUSEMOTION:
                if (pt->w_focus)
                {
                    pt->c_player->JOY_use = false;

                    pt->c_player->crosshair.r.x += ev.motion.xrel;
                    pt->c_player->crosshair.r.y += ev.motion.yrel;

                    pt->c_player->m_move = true;
                }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (pt->g_state == G_PLAY)
                {
                    if (ev.button.button == SDL_BUTTON_LEFT)
                    {
                        switch (pt->c_player->state)
                        {
                            case PLR_SKATE_NP:
                                if (!pt->c_player->m_hold)
                                {
                                    pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                    pt->c_player->state = PLR_SWING;
                                    Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                                }
                            break;
                            default: break;
                        }
                        pt->c_player->m_hold = true;
                    }
                }
                else if (pt->g_state == G_MENU)
                {
                    if (ev.button.button == SDL_BUTTON_LEFT)
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[i]))
                            {
                                switch (i)
                                {
                                    case 0: if (!pt->is_net) pt->g_state = G_HOST; break;
                                    case 1: 
                                        if (!pt->is_net) pt->g_state = G_JOIN; 
                                        else 
                                        {
                                            // send disconnect signal
                                            if (pt->network.localuser.id)
                                            {
                                                pt->network.localuser.status = N_DISCONNECT;

                                                unsigned long timer = SDL_GetTicks64();

                                                while (!pt->network.lost)
                                                {
                                                    if ((SDL_GetTicks64() - timer) >= 1000) 
                                                        pt->network.tquit = true;
                                                }
                                            }
                                            
                                            // disconnect from net here
                                            closeNet(&pt->network);
                                            pt->is_net = false;

                                            for (unsigned i = 0; i < MAX_GAME_USERS; i++)
                                            {
                                                resetPlayer(
                                                    &pt->players[i],
                                                    pt->level.r.w >> 1, 
                                                    pt->level.r.h >> 1);

                                                pt->players[i].id = 0;
                                            }
                                            pt->c_player = &pt->players[0];
                                            pt->c_player->spawned = true;
                                        }
                                    break;
                                    case 2: 
                                        pt->quit = true; 
                                        pt->is_net = false;
                                    break;
                                }
                                break;
                            }
                        }
                    }
                }
            break;
            case SDL_MOUSEBUTTONUP:
                if (ev.button.button == SDL_BUTTON_LEFT && pt->w_focus)
                {
                    switch (pt->c_player->state)
                    {
                        case PLR_SKATE_WP:
                            if (pt->c_player->m_hold)
                            {
                                if (!pt->puck.hit)
                                {
                                    pt->puck.hit = true;

                                    if (pt->c_player->pvel >= 4) 
                                        pt->c_player->vel -= pt->c_player->gvel * 0.5f;
                                    else 
                                        pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                        
                                    pt->c_player->swing_timer = 0;
                                    pt->c_player->state = PLR_SHOOT;
                                }
                            }
                        break;
                        default: break;
                    }
                    pt->c_player->m_hold = false;
                }
            break;
            case SDL_JOYAXISMOTION:
                if (ev.jaxis.which == 0)
                {
                    switch (ev.jaxis.axis)
                    {
                        case 0:
                        case 1:
                            pt->c_player->JOY_use = true;

                            if (ev.jaxis.axis == 0)
                            {
                                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                                    pt->c_player->JOY_xdir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->c_player->JOY_xdir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                                else 
                                    pt->c_player->JOY_xdir = 0;
                            }
                            else if (ev.jaxis.axis == 1)
                            {
                                if (ev.jaxis.value < -JOYSTICK_DEADZONE) 
                                    pt->c_player->JOY_ydir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->c_player->JOY_ydir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                                else 
                                    pt->c_player->JOY_ydir = 0;
                            }

                            // this is too much, but it works
                            pt->c_player->JOY_vel = (
                                pt->c_player->JOY_xdir < 0 ? -pt->c_player->JOY_xdir : pt->c_player->JOY_xdir * SDL_cos(pt->c_player->INPUT_angle)) + (
                                pt->c_player->JOY_ydir < 0 ? -pt->c_player->JOY_ydir : pt->c_player->JOY_ydir * SDL_sin(pt->c_player->INPUT_angle));

                            if (pt->c_player->JOY_vel > 0.75f) pt->c_player->JOY_vel = 1.0f;
                        break;
                        case 3:
                        case 4:
                            pt->c_player->m_move = true;
                            pt->c_player->JOY_use = true;

                            if (ev.jaxis.axis == 3)
                            {
                                pt->c_player->AIM_xvel = (ev.jaxis.value / 32767.0f);

                                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                                    pt->c_player->AIM_xdir = -1; 
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->c_player->AIM_xdir = 1;
                                else 
                                    pt->c_player->AIM_xdir = 0;
                            }
                            else if (ev.jaxis.axis == 4)
                            {
                                pt->c_player->AIM_yvel = (ev.jaxis.value / 32767.0f);

                                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                                    pt->c_player->AIM_ydir = -1;
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->c_player->AIM_ydir = 1;
                                else 
                                    pt->c_player->AIM_ydir = 0;
                            }
                        break;
                        case 2:
                        break;
                        case 5:
                            switch (pt->c_player->state)
                            {
                                default: 
                                    if (ev.jaxis.value > -16000) pt->c_player->m_hold = true;
                                    else pt->c_player->m_hold = false;
                                break;
                                case PLR_SKATE_NP:
                                    if (ev.jaxis.value > -16000)
                                    {
                                        if (!pt->c_player->m_hold) 
                                        {
                                            pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                            pt->c_player->state = PLR_SWING;
                                            Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                                        }
                                        pt->c_player->m_hold = true;
                                    } 
                                    else pt->c_player->m_hold = false;
                                break;
                                case PLR_SKATE_WP:
                                    if (ev.jaxis.value > -16000) pt->c_player->m_hold = true;
                                    else 
                                    {
                                        if (pt->c_player->m_hold) 
                                        {
                                            if (!pt->puck.hit)
                                            {
                                                pt->puck.hit = true;

                                                if (pt->c_player->pvel >= 4) 
                                                    pt->c_player->vel -= pt->c_player->gvel * 0.5f;
                                                else 
                                                    pt->c_player->vel -= pt->c_player->gvel * 0.25f;

                                                pt->c_player->swing_timer = 0;
                                                pt->c_player->state = PLR_SHOOT;
                                            }
                                        }
                                        pt->c_player->m_hold = false;
                                    }
                                break;
                            }
                        break;
                    }
                }
            break;
            case SDL_JOYBUTTONDOWN:
                if (ev.jbutton.which == 0)
                {
                    if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_A)
                    {
                        if (!pt->c_player->sprint_cdown 
                        && (pt->c_player->state == PLR_SKATE_NP 
                        || pt->c_player->state == PLR_SKATE_WP)) 
                        {
                            pt->c_player->sprint = true;
                            pt->c_player->gvel = SPRINT_VELOCITY;
                            pt->c_player->vel = SPRINT_VELOCITY;
                        }
                    }
                    else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_B)
                    {
                        if (pt->c_player->state == PLR_SKATE_NP)
                        {
                            pt->c_player->state = PLR_BLOCK;
                            pt->c_player->vel += STANDARD_VELOCITY * 0.5f;
                            switch (pt->c_player->facing)
                            {
                                case FACING_DOWN:
                                    pt->c_player->r.w = 16;
                                    pt->c_player->r.h = 30;
                                break;
                                case FACING_UP:
                                    pt->c_player->r.w = 16;
                                    pt->c_player->r.h = 30;
                                break;
                                case FACING_RIGHT:
                                    pt->c_player->r.w = 30;
                                    pt->c_player->r.h = 16;
                                break;
                                case FACING_LEFT:
                                    pt->c_player->r.w = 30;
                                    pt->c_player->r.h = 16;
                                break;
                            }
                        }
                    }
                }
            break;
            case SDL_JOYDEVICEADDED:
                for (int i = 0; i < SDL_NumJoysticks(); i++) 
                {
                    if (SDL_IsGameController(i)) 
                    {
                        pt->controller = SDL_GameControllerOpen(i);
                        if (pt->controller) 
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
            break;
            case SDL_JOYDEVICEREMOVED:
                printf("Controller %d disconnected!\n", ev.jdevice.which);
            break;
        }
    }
}

void inputsJoin(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT:
                pt->quit = true;
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
                        pt->g_state = G_MENU;
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
                            if (pt->input_field.str_len > 8)
                            {
                                pt->g_state = G_JOINING;
                                if (startNetClient(&pt->network, pt->input_field.string))
                                {
                                    pt->is_net = true;

                                    resetPuck(&pt->puck, 0, 0);
                                    
                                    for (int i = 0; i < 2; i++) 
                                        resetPlayer(
                                            &pt->players[i], 
                                            pt->level.r.w >> 1, 
                                            pt->level.r.h >> 1);

                                    //pt->players[0].id = pt->network.localuser.id;
                                    //pt->players[0].spawned = true;

                                    pt->g_state = G_PLAY;
                                }
                                else pt->g_state = G_MENU;
                            }
                            resetInputField(&pt->input_field);
                        }
                    break;
                }
            break;
        }
    }
}

void inputsHost(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT:
                pt->quit = true;
            break;
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
                        pt->g_state = G_MENU;
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
                                pt->g_state = G_HOSTING;

                                // setup host connection
                                if (startNetHost(&pt->network, atoi(pt->input_field.string)))
                                {
                                    pt->is_net = true;

                                    resetPuck(&pt->puck, pt->level.r.w >> 1, pt->level.r.h >> 1);

                                    for (int i = 0; i < 2; i++) 
                                        resetPlayer(&pt->players[i], pt->level.r.w >> 1, pt->level.r.h >> 1);

                                    pt->players[0].id = 1;
                                    pt->players[0].spawned = true;

                                    pt->g_state = G_PLAY;
                                }
                            }
                            resetInputField(&pt->input_field);
                        }
                    break;
                }
            break;
        }
    }
}

void inputsNetGame(P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        switch (ev.type)
        {
            case SDL_QUIT:
                pt->is_net = false;
                pt->quit = true;
            break;
            case SDL_KEYDOWN:
                if (!ev.key.repeat)
                {
                    switch (ev.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            if (pt->g_state == G_PLAY) pt->g_state = G_MENU;
                            else if (pt->g_state == G_MENU) pt->g_state = G_PLAY;
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
                        case SDLK_SPACE:
                        case SDLK_KP_SPACE:
                            if (pt->c_player->state == PLR_SKATE_WP)
                            {
                                pt->c_player->state = PLR_SHOOT;
                            }
                        break;
                    }
                }
            break;
            case SDL_KEYUP:
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
                }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (pt->g_state == G_MENU && ev.button.button == SDL_BUTTON_LEFT)
                {
                    if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[1]))
                    {
                        // send disconnect signal
                        if (pt->network.localuser.id)
                        {
                            pt->network.localuser.status = N_DISCONNECT;

                            unsigned long timer = SDL_GetTicks64();

                            while (!pt->network.lost)
                            {
                                if ((SDL_GetTicks64() - timer) >= 1000) 
                                    pt->network.tquit = true;
                            }
                        }
                        // disconnect from net here
                        closeNet(&pt->network);
                        pt->is_net = false;

                        for (unsigned i = 0; i < MAX_GAME_USERS; i++)
                        {
                            resetPlayer(
                                &pt->players[i],
                                pt->level.r.w >> 1, 
                                pt->level.r.h >> 1);

                            pt->players[i].id = 0;
                        }

                        pt->c_player = &pt->players[0];
                        pt->c_player->spawned = true;
                    }
                    else if (checkMousePosition(ev.motion.x, ev.motion.y, pt->buttons[2]))
                    {
                        pt->quit = true; 
                        pt->is_net = false;
                    }
                }
            break;
        }
    }
}

void shootPuck(Puck *puck, float vel, float x, float y, float angle)
{
    puck->x = x;
    puck->y = y;
    puck->xvel = vel * SDL_cos(angle);
    puck->yvel = vel * SDL_sin(angle);
    puck->hit = true;
    puck->grab = false;
}

/*
bool playShootGun(P_TEST play, BUL bullets[], int sx, int sy, int dx, int dy)
{
    bool success = false;
    
    double angle = (SDL_atan2(
        dy - sy + play.GUN_rady, 
        dx - sx + play.GUN_radx));

    char sway = rand() % 7;

    float rx = (AIM_RADIUS * SDL_cos(angle)) + (-6 + sway),
          ry = (AIM_RADIUS * SDL_sin(angle)) + (-6 + sway);

    for (int i = 0; i < 10; i++)
    {
        if (!bullets[i].shoot)
        {
            bullets[i].shoot = true;

            bullets[i].x = sx + play.GUN_radx;
            bullets[i].y = sy + play.GUN_rady;

            bullets[i].xvel = (rx / play.GUN_speed);
            bullets[i].yvel = (ry / play.GUN_speed);

            success = true;
            break;
        }
    }

    return success;
}
*/

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

void adjustGoalie(float *gky, float ry, float vy)
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

void updateBulletHits(B_HITS *hits, int bx, int by)
{
    hits->a[hits->index].x = bx;
    hits->a[hits->index].y = by;

    hits->a[hits->index].used = true;

    if (++hits->index > 29) hits->index = 0;
}

void updateGame(P_TEST *pt, P players[])
{
    if (pt->is_net)
    {
        updateNetGame(pt, players);

        if (pt->network.join)
        {
            printf("NET: player join\n");
            pt->network.join = false;
            for (unsigned char i = 0; i < pt->network.numplayers; i++)
            {
                if (!pt->players[i].id)
                {
                    addPlayerGame(
                        &pt->players[i], 
                        pt->network.players_net[i].id, 
                        pt->network.players_net[i].x, 
                        pt->network.players_net[i].y);

                    printf("PLAYER: joined player %d\n", pt->players[i].id);
                }

                if (pt->players[i].id == pt->network.localplayer->id)
                    pt->c_player = &pt->players[i];
            }
        }

        if (pt->network.left)
        {
            printf("NET: player left\n");
            pt->network.left = false;
            unsigned char n = 0;
            for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
            {
                for (unsigned char j = 0; j < pt->network.numplayers; j++)
                {
                    if (players[i].id == pt->network.players_net[j].id) 
                    {
                        n = 1;
                        break;
                    }
                    else if (!players[i].id) 
                    {
                        n = 1; 
                        break;
                    }
                }

                if (!n)
                {
                    printf("PLAYER: found lost id:%d\n", players[i].id);
                    removePlayerGame(&players[i]);
                }

                if (pt->players[i].id == pt->network.localplayer->id)
                    pt->c_player = &pt->players[i];

                n = 0;
            }
        }
        
        if (pt->network.lost)
        {
            // disconnect from net here
            closeNet(&pt->network);
            pt->is_net = false;

            for (unsigned i = 0; i < MAX_GAME_USERS; i++)
            {
                resetPlayer(
                    &pt->players[i], 
                    pt->level.r.w >> 1, 
                    pt->level.r.h >> 1);

                pt->players[i].id = 0;
            }
            // set local player to first
            pt->c_player = &pt->players[0];
            pt->c_player->spawned = true;
        }
    }
    else updateLocalGame(pt, players);

    // set camera, duh
    setCamera(
        &pt->camera, 
        lerp(
            pt->camera.x + (pt->camera.w >> 1), 
            pt->rx, 0.08f), 
        lerp(
            pt->camera.y + (pt->camera.h >> 1), 
            pt->ry, 0.08f)
    );

    if (pt->camera.x < 0) pt->camera.x = 0;
    else if ((pt->camera.x + W_WIDTH) > pt->level.r.w) 
        pt->camera.x = pt->level.r.w - W_WIDTH;

    if (pt->camera.y > 0) pt->camera.y = 0;
    else if ((pt->camera.y + W_HEIGHT) < pt->level.r.h) 
        pt->camera.y = pt->level.r.h - W_HEIGHT;
}

void updateGameDrop(P_TEST *pt, P players[])
{
    for (unsigned char i = 0; i < 2; i++)
    {
        if (players[i].spawned)
        {
            if (players[i].JOY_use)
            {
                if (players[i].AIM_xdir || players[i].AIM_ydir)
                {
                    players[i].AIM_angle = SDL_atan2(players[i].AIM_yvel, players[i].AIM_xvel);

                    players[i].crosshair.r.x = (
                        players[i].x - pt->camera.x + (AIM_RADIUS * SDL_cos(players[i].AIM_angle)));

                    players[i].crosshair.r.y = (
                        players[i].y - pt->camera.y + (AIM_RADIUS * SDL_sin(players[i].AIM_angle)));
                }
                else 
                {
                    players[i].crosshair.r.x = players[i].x - pt->camera.x;
                    players[i].crosshair.r.y = players[i].y - pt->camera.y;
                    players[i].crosshair.show = false;
                    players[i].m_move = false;
                }
            }
            else 
            {
                if (players[i].m_move)
                {
                    players[i].AIM_angle = (SDL_atan2(
                        pt->camera.y + players[i].crosshair.r.y - players[i].y, 
                        pt->camera.x + players[i].crosshair.r.x - players[i].x));
                }
            }

            if (players[i].m_move)
            {
                players[i].m_move = false;

                players[i].AIM_timer = 0;
                players[i].AIM_done = true;

                players[i].crosshair.show = true;
            }

            if (players[i].AIM_done && players[i].AIM_timer++ > 240) 
            {
                players[i].AIM_done = false;
                players[i].AIM_timer = 0;

                players[i].crosshair.r.x = players[i].x - pt->camera.x;
                players[i].crosshair.r.y = players[i].y - pt->camera.y;
                players[i].crosshair.show = false;
            }

            if (!players[i].crosshair.show) 
                players[i].AIM_angle = players[i].INPUT_angle;

            if (players[i].state == PLR_SWING)
            {
                if (checkCollision(players[i].club_r, pt->puck.r))
                {
                    // puck goes wild
                    pt->puck.hit = true;
                    pt->puck.xvel = 1.5f * SDL_cos(players[i].AIM_angle);
                    pt->puck.yvel = 1.5f * SDL_sin(players[i].AIM_angle);

                    pt->PUCK_freeze = true;

                    pt->p_state = P_PLAY;

                    Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                }
            }

            players[i].r.x = players[i].x - 10;
            players[i].r.y = players[i].y;

            animatePlayer(&players[i]);
        }
    }
}

void updateGamePlay(P_TEST *pt, P players[])
{
    for (unsigned char i = 0; i < 2; i++)
    {
        if (players[i].spawned)
        {
            updatePlayer(&players[i], pt);

            switch (players[i].state)
            {
                case PLR_SKATE_NP:
                    if (!pt->puck.hit)
                    {
                        bool grab = false;

                        for (int j = 0; j < 2; j++) 
                        {
                            if (players[j].state == PLR_SKATE_WP) 
                            {
                                grab = true; 
                                break;
                            }
                            else if (pt->goalie[j].grab)
                            {
                                grab = true; 
                                break;
                            }
                        }

                        if (!grab)
                        {
                            if (checkCollision(players[i].r, pt->puck.r))
                            {
                                pt->puck.xvel = 0;
                                pt->puck.yvel = 0;
                                pt->puck.fvelx = 0.01f;
                                pt->puck.fvely = 0.01f;
                                pt->puck.x = players[i].club_r.x + (players[i].club_r.w >> 1);
                                pt->puck.y = players[i].club_r.y + (players[i].club_r.h >> 1);

                                players[i].pvel = 2;
                                players[i].state = PLR_SKATE_WP;

                                Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                            }
                        }
                    }
                break;
                case PLR_SKATE_WP:
                break;
                case PLR_SWING:
                    // check if another player has the puck
                    for (int j = 0; j < 2; j++)
                    {
                        if (&players[i] != &players[j])
                        {
                            if (players[j].state == PLR_SKATE_WP
                            && checkCollision(players[i].club_r, players[j].club_r))
                            {
                                // puck goes wild
                                pt->puck.hit = true;
                                pt->puck.xvel = SDL_cos(players[i].AIM_angle);
                                pt->puck.yvel = SDL_sin(players[i].AIM_angle);

                                pt->PUCK_freeze = true;

                                players[j].state = PLR_SKATE_NP;

                                Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                                break;
                            }
                        }
                    }
                break;
                default: break;
            }
            animatePlayer(&players[i]);
        }
    }

    if (!pt->PUCK_freeze)
    {
        SDL_Rect *r = &pt->puck.r;
        bool grab = false;
        unsigned char i = 0;

        for (; i < 2; i++)
        {
            if (pt->goalie[i].grab)
            {
                r = &pt->goalie[i].r;
                grab = true;
                break;
            }
        }

        for (i = 0; i < pt->network.numplayers; i++) 
        {
            if (players[i].state == PLR_SKATE_WP) 
            {
                r = &players[i].r;
                grab = true;
                break;
            }
        }

        updatePuck(pt, players);
        updateGoalKeepers(pt, players, grab);

        for (i = 0; i < 2; i++)
        {
            if (checkGoal(pt->puck.r, pt->goal_r[i]))
                pt->p_state = P_GOAL;
        }

        pt->rx = r->x;
        pt->ry = r->y;
    }
}

void updateGameGoal(P_TEST *pt, P players[])
{
    for (int i = 0; i < 2; i++)
    {
        if (players[i].spawned) 
        {
            updatePlayer(&players[i], pt);
            animatePlayer(&players[i]);
        }
            
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

        if (checkPlayerPosition(
            (int)px >> pt->level.t_bit_size, 
            (int)pt->puck.y >> pt->level.t_bit_size, 
            pt->level.collision, pt->level.t_map_h)
        )
        {
            pt->puck.xvel = -pt->puck.xvel;
            pt->puck.fvelx += 0.3f;
            pt->puck.fvely += 0.02f;
        }
        else if (px < 0 || px > pt->level.r.w) 
        {
            pt->puck.xvel = -pt->puck.xvel;
            pt->puck.fvelx += 0.3f;
            pt->puck.fvely += 0.02f;
        }
        else pt->puck.x = px;
    }

    if (pt->puck.yvel)
    {
        float py = pt->puck.y + pt->puck.yvel;

        if (checkPlayerPosition(
            (int)pt->puck.x >> pt->level.t_bit_size, 
            (int)py >> pt->level.t_bit_size, 
            pt->level.collision, pt->level.t_map_h)
        )
        {
            pt->puck.yvel = -pt->puck.yvel;
            pt->puck.fvelx += 0.02f;
            pt->puck.fvely += 0.3f;
        }
        else if (py < 0 || py > pt->level.r.h) 
        {
            pt->puck.yvel = -pt->puck.yvel;
            pt->puck.fvelx += 0.02f;
            pt->puck.fvely += 0.3f;
        }
        else pt->puck.y = py;
    }

    if (!pt->puck.xvel && !pt->puck.yvel)
    {
        pt->puck.fvelx = 0.01f;
        pt->puck.fvely = 0.01f;
    }

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;

    if (++pt->SCORE_timer > 240)
    {
        for (int i = 0; i < MAX_GAME_USERS; i++) 
            resetPlayer(&players[i], pt->level.r.w >> 1, pt->level.r.h >> 1);
        
        resetPuck(&pt->puck, pt->level.r.w >> 1, pt->level.r.h >> 1);

        pt->sprint_hud_r.w = 0;

        pt->rx = pt->puck.r.x;
        pt->ry = pt->puck.r.y;

        pt->SCORE_timer = 0;

        pt->p_state = P_DROP;
    }
}

void updatePlayer(P *p, P_TEST *pt)
{
    switch (p->state)
    {
        case PLR_SKATE_NP:
            movePlayer(p, pt->camera);

            if (p->sprint && (++p->sprint_timer > 60))
            {
                p->sprint_timer = 0;
                p->sprint = false;
                p->sprint_cdown = true;
                p->gvel = STANDARD_VELOCITY;
                p->vel = STANDARD_VELOCITY;
            }

            if (p->sprint_cdown && (++p->sprint_cdown_timer > 180))
            {
                p->sprint_cdown = false;
                p->sprint_cdown_timer = 0;

                pt->sprint_hud_r.w = 0;
            }
            else if (p->sprint_cdown)
            {
                pt->sprint_hud_r.w = (p->sprint_cdown_timer / 6) << 1;
            }

            if (p->m_move)
            {
                p->m_move = false;

                p->AIM_timer = 0;
                p->AIM_done = true;

                p->crosshair.show = true;
            }

            if (p->AIM_done && p->AIM_timer++ > 240) 
            {
                p->AIM_done = false;
                p->AIM_timer = 0;

                p->crosshair.r.x = p->x - pt->camera.x;
                p->crosshair.r.y = p->y - pt->camera.y;
                p->crosshair.show = false;
            }

            if (!p->crosshair.show) p->AIM_angle = p->INPUT_angle;
        break;
        case PLR_SKATE_WP:
            movePlayer(p, pt->camera);

            if (p->sprint && (++p->sprint_timer > 60))
            {
                p->sprint_timer = 0;
                p->sprint = false;
                p->sprint_cdown = true;
                p->gvel = STANDARD_VELOCITY;
                p->vel = STANDARD_VELOCITY;
            }

            if (p->sprint_cdown && (++p->sprint_cdown_timer > 300))
            {
                p->sprint_cdown = false;
                p->sprint_cdown_timer = 0;

                pt->sprint_hud_r.w = 0;
            }
            else if (p->sprint_cdown)
            {
                pt->sprint_hud_r.w = (p->sprint_cdown_timer / 6) << 1;
            }

            if (p->m_move)
            {
                p->m_move = false;

                p->AIM_timer = 0;
                p->AIM_done = true;

                p->crosshair.show = true;
            }

            if (p->AIM_done && p->AIM_timer++ > 240) 
            {
                p->AIM_done = false;
                p->AIM_timer = 0;

                p->crosshair.r.x = p->x - pt->camera.x;
                p->crosshair.r.y = p->y - pt->camera.y;
                p->crosshair.show = false;
            }

            if (p->m_hold)
            {
                if ((p->pvel += 0.1f) > MAX_SHOOT_VELOCITY)
                    p->pvel = MAX_SHOOT_VELOCITY;
            }

            if (!p->crosshair.show) p->AIM_angle = p->INPUT_angle;

            pt->puck.x = p->club_r.x + (p->club_r.w >> 1);
            pt->puck.y = p->club_r.y + (p->club_r.h >> 1);
        break;
        case PLR_SHOOT:
            p->swing_timer++;

            if (p->pvel < 4 && pt->PUCK_freeze) p->JOY_vel = 0;
            else p->JOY_vel = 1;

            if (p->swing_timer < 3)
            {
                pt->puck.x = p->x;
                pt->puck.y = p->y;
            }
            else if (p->swing_timer == 3)
            {
                shootPuck(
                    &pt->puck, 
                    pt->c_player->pvel, 
                    pt->c_player->x, 
                    pt->c_player->y, 
                    pt->c_player->AIM_angle
                );

                pt->PUCK_freeze = true;

                Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
            }
            else if (p->swing_timer > 20)
            {
                p->swing_timer = 0;
                p->vel = p->gvel;
                p->pvel = 2;
                p->state = PLR_SKATE_NP;
            }
        break;
        case PLR_SWING:
            if (++p->swing_timer > 10)
            {
                p->swing_timer = 0;
                p->vel = p->gvel;
                p->pvel = 2;
                p->state = PLR_SKATE_NP;
            }
        break;
        case PLR_SPRINT:

        break;
        case PLR_BLOCK:
            if ((p->vel -= 0.05f) < 0) p->vel = 0;

            if (++p->block_timer > 60)
            {
                p->bounce = false;
                p->state = PLR_SKATE_NP;
                p->block_timer = 0;
                p->vel = STANDARD_VELOCITY;

                p->r.w = 20;
                p->r.h = 20;
            }
        break;
        default: break;
    }

    float rx = AIM_RADIUS * SDL_cos(p->AIM_angle), 
          ry = AIM_RADIUS * SDL_sin(p->AIM_angle);

    // need to look over this 
    if (pt->camera.x + p->crosshair.r.x - p->x < 0)
    {
        if (pt->camera.x + p->crosshair.r.x - p->x < rx) 
            p->crosshair.r.x = p->x - pt->camera.x + rx;
    }
    else if (pt->camera.x + p->crosshair.r.x - p->x > 0)
    {
        if (pt->camera.x + p->crosshair.r.x - p->x > rx)
            p->crosshair.r.x = p->x - pt->camera.x + rx;
    }
    //
    if (pt->camera.y + p->crosshair.r.y - p->y < 0)
    {
        if (pt->camera.y + p->crosshair.r.y - p->y < ry) 
            p->crosshair.r.y = p->y - pt->camera.y + ry;
    }
    else if (pt->camera.y + p->crosshair.r.y - p->y > 0)
    {
        if (pt->camera.y + p->crosshair.r.y - p->y > ry)
            p->crosshair.r.y = p->y - pt->camera.y + ry;
    }
    //

    if (p->JOY_vel)
    {
        float   cos = SDL_cos(p->INPUT_angle) * p->vel * p->JOY_vel,
                sin = SDL_sin(p->INPUT_angle) * p->vel * p->JOY_vel;

        if (p->xvel > cos)
        {
            p->xvel -= 0.2f;
            if (p->xvel < cos) p->xvel = cos;
        }
        else if (p->xvel < cos)
        {
            p->xvel += 0.2;
            if (p->xvel > cos) p->xvel = cos;
        }

        if (p->yvel > sin)
        {
            p->yvel -= 0.2f;
            if (p->yvel < sin) p->yvel = sin;
        }
        else if (p->yvel < sin)
        {
            p->yvel += 0.2f;
            if (p->yvel > sin) p->yvel = sin;
        }
    }
    else 
    {
        if (p->xvel > 0)
        {
            p->xvel -= 0.05f;
            if (p->xvel < 0) p->xvel = 0;
        }
        else if (p->xvel < 0)
        {
            p->xvel += 0.05f;
            if (p->xvel > 0) p->xvel = 0;
        }
        
        if (p->yvel > 0)
        {
            p->yvel -= 0.05f;
            if (p->yvel < 0) p->yvel = 0;
        }
        else if (p->yvel < 0)
        {
            p->yvel += 0.05f;
            if (p->yvel > 0) p->yvel = 0;
        }
    }

    if (p->state == PLR_BLOCK)
    {
        switch (p->facing)
        {
            case FACING_DOWN:
                p->r.x = p->x - 8;
                p->r.y = p->y;
            break;
            case FACING_UP:
                p->r.x = p->x - 8;
                p->r.y = p->y - 10;
            break;
            case FACING_RIGHT:
                p->r.x = p->x - 10;
                p->r.y = p->y + 5;
            break;
            case FACING_LEFT:
                p->r.x = p->x - 20;
                p->r.y = p->y + 5;
            break;
        }
    }
    else 
    {
        p->r.x = p->x - 10;
        p->r.y = p->y;
    }

    if (p->xvel)
    {
        bool move_ok = true;
        float x = p->x + p->xvel;
        SDL_Rect c = {x - 10, p->r.y, p->r.w, p->r.h};

        for (unsigned char i = 0; i < 2; i++)
        {
            if (checkCollision(c, pt->goal_r[i])) 
            {
                move_ok = false;
                break;
            }
        }
        
        if (!checkPlayerPosition(
            (int)x >> pt->level.t_bit_size, 
            (int)p->y >> pt->level.t_bit_size, 
            pt->level.collision, 
            pt->level.t_map_h)
        )
        {
            if (!(x < 0 || x > pt->level.r.w) && move_ok) p->x = x;
            else p->xvel = 0;
        }
        else p->xvel = 0;
    }

    if (p->yvel)
    {
        bool move_ok = true;
        float y = p->y + p->yvel;
        SDL_Rect c = {p->r.x, y - 10, p->r.w, p->r.h};

        for (unsigned char i = 0; i < 2; i++)
        {
            if (checkCollision(c, pt->goal_r[i])) 
            {
                move_ok = false;
                break;
            }
        }

        if (!checkPlayerPosition(
            (int)p->x >> pt->level.t_bit_size, 
            (int)y >> pt->level.t_bit_size, 
            pt->level.collision, 
            pt->level.t_map_h)
        )
        {
            if (!(y < 0 || y > pt->level.r.h) && move_ok) p->y = y;
            else p->yvel = 0;
        } 
        else p->yvel = 0;  
    }

    p->AIM_radx = 10 * SDL_cos(p->AIM_angle);
    p->AIM_rady = 10 * SDL_sin(p->AIM_angle);

    p->club_r.x = p->x - 8 + p->AIM_radx;
    p->club_r.y = p->y - 8 + p->AIM_rady;

    p->AIM_deg = (p->AIM_angle * 180) / AIM_PI;

    if (p->AIM_deg < 0) p->AIM_deg += 360;

    if (p->AIM_deg > AIM_RIGHT || p->AIM_deg < AIM_DOWN)
    {
        p->facing = FACING_RIGHT; // right
        if (p->JOY_use && p->JOY_vel) 
            p->input_q[0] = KEY_RIGHT;
    }
    else if (p->AIM_deg > AIM_DOWN && p->AIM_deg < AIM_LEFT)
    {
        p->facing = FACING_DOWN; // down
        if (p->JOY_use && p->JOY_vel) 
            p->input_q[0] = KEY_DOWN;
    }
    else if (p->AIM_deg > AIM_LEFT && p->AIM_deg < AIM_UP)
    {
        p->facing = FACING_LEFT; // left
        if (p->JOY_use && p->JOY_vel) 
            p->input_q[0] = KEY_LEFT;
    }
    else if (p->AIM_deg > AIM_UP && p->AIM_deg < AIM_RIGHT)
    {
        p->facing = FACING_UP; // up
        if (p->JOY_use && p->JOY_vel) 
            p->input_q[0] = KEY_UP;
    }
}

void updatePlayerInputs(P *p)
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

void updatePuck(P_TEST *pt, P players[])
{
    if (pt->PUCK_freeze && (++pt->PUCK_freeze_timer > 4))
    {
        pt->PUCK_freeze = false;
        pt->PUCK_freeze_timer = 0;
    }

    if (pt->puck.hit && (++pt->puck.hit_counter > 15)) 
    {
        pt->puck.hit = false;
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

        if (checkPlayerPosition(
            (int)px >> pt->level.t_bit_size, 
            (int)pt->puck.y >> pt->level.t_bit_size, 
            pt->level.collision, pt->level.t_map_h)
        )
        {
            pt->puck.xvel = -pt->puck.xvel;
            pt->puck.fvelx += 0.3f;
            pt->puck.fvely += 0.02f;
        }
        else if (px < 0 || px > pt->level.r.w) 
        {
            pt->puck.xvel = -pt->puck.xvel;
            pt->puck.fvelx += 0.3f;
            pt->puck.fvely += 0.02f;
        }
        else pt->puck.x = px;
    }

    if (pt->puck.yvel)
    {
        float py = pt->puck.y + pt->puck.yvel;

        if (checkPlayerPosition(
            (int)pt->puck.x >> pt->level.t_bit_size, 
            (int)py >> pt->level.t_bit_size, 
            pt->level.collision, pt->level.t_map_h)
        )
        {
            pt->puck.yvel = -pt->puck.yvel;
            pt->puck.fvelx += 0.02f;
            pt->puck.fvely += 0.3f;
        }
        else if (py < 0 || py > pt->level.r.h) 
        {
            pt->puck.yvel = -pt->puck.yvel;
            pt->puck.fvelx += 0.02f;
            pt->puck.fvely += 0.3f;
        }
        else pt->puck.y = py;
    }

    if (!pt->puck.xvel && !pt->puck.yvel)
    {
        pt->puck.fvelx = 0.01f;
        pt->puck.fvely = 0.01f;
    }

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;
}

void updateGoalKeepers(P_TEST *pt, P players[], bool grab)
{
    if (pt->p_state != P_GOAL && !pt->PUCK_freeze)
    {
        for (unsigned char g = 0; g < 2; g++)
        {
            switch (pt->goalie[g].state)
            {
                case GK_NORMAL:
                    if (!pt->puck.xvel && !pt->puck.yvel && !grab && !pt->puck.hit)
                    {
                        if (g == 0)
                        {
                            if (checkCollision(pt->puck.r, pt->gk_r[0]))
                                pt->goalie[g].state = GK_CLEAR_GOAL;
                        }
                        else 
                        {
                            if (checkCollision(pt->puck.r, pt->gk_r[1]))
                                pt->goalie[g].state = GK_CLEAR_GOAL;
                        }
                    }

                    float f = 0.85f;

                    if (g == 0)
                    {
                        if (pt->goalie[g].x > 154) 
                        {
                            if ((pt->goalie[g].x -= 1.125f) < 154)
                                pt->goalie[g].x = 154;
                        }
                        else if (pt->goalie[g].x < 154) 
                        {
                            if ((pt->goalie[g].x += 1.125f) > 154)
                                pt->goalie[g].x = 154;
                        }

                        if (pt->puck.r.x - pt->goalie[g].x < 100)
                            f = 1.5f;
                        else if (pt->puck.r.x - pt->goalie[g].x < 150)
                            f = 1.25f;
                        else if (pt->puck.r.x - pt->goalie[g].x < 200)
                            f = 1.125f;
                    }
                    else 
                    {
                        if (pt->goalie[g].x > 742) 
                        {
                            if ((pt->goalie[g].x -= 1.125f) < 742)
                                pt->goalie[g].x = 742;
                        }
                        else if (pt->goalie[g].x < 742) 
                        {
                            if ((pt->goalie[g].x += 1.125f) > 742)
                                pt->goalie[g].x = 742;
                        }

                        if (pt->goalie[g].x - pt->puck.r.x < 100)
                            f = 1.5f;
                        else if (pt->goalie[g].x - pt->puck.r.x < 150)
                            f = 1.25f;
                        else if (pt->goalie[g].x - pt->puck.r.x < 200)
                            f = 1.125f;
                    }

                    if (pt->puck.r.y < 112)
                    {
                        if (pt->goalie[g].y < 112) pt->goalie[g].y += f;
                        else adjustGoalie(&pt->goalie[g].y, pt->puck.r.y, f);
                    }
                    else if (pt->puck.r.y > 160)
                    {
                        if (pt->goalie[g].y > 160) pt->goalie[g].y -= f;
                        else adjustGoalie(&pt->goalie[g].y, pt->puck.r.y, f);
                    }
                    else adjustGoalie(&pt->goalie[g].y, pt->puck.r.y, f);
                break;
                case GK_CLEAR_GOAL:
                    if (!grab)
                    {
                        if (checkCollision(pt->puck.r, pt->goalie[g].r))
                        {
                            pt->puck.x = pt->goalie[g].x;
                            pt->puck.y = pt->goalie[g].y;

                            pt->goalie[g].grab = true;
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
                    if (pt->goalie[g].shoot)
                    {
                        if (++pt->goalie[g].s_timer > 10)
                        {
                            pt->goalie[g].s_timer = 0;
                            pt->goalie[g].shoot = false;
                            pt->goalie[g].state = GK_NORMAL;
                        }
                    }
                break;
                case GK_GRAB:
                    // swing/shoot?
                    // puck goes to center
                    if (++pt->goalie[g].s_timer > 10)
                    {
                        pt->puck.hit = true;
                        pt->puck.xvel = g == 0 ? 2.5f : -2.5f;
                        pt->puck.yvel = 0;

                        pt->PUCK_freeze = true;

                        pt->goalie[g].s_timer = 0;
                        pt->goalie[g].shoot = true;
                        pt->goalie[g].grab = false;

                        pt->goalie[g].state = GK_SHOOT;

                        Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                    }
                break;
            }
            pt->goalie[g].r.x = pt->goalie[g].x - 10;
            pt->goalie[g].r.y = pt->goalie[g].y - 10;
        }
    }
}

void updateLocalGame(P_TEST *pt, P players[])
{
    switch (pt->p_state)
    {
        case P_DROP:
            updateGameDrop(pt, players);
        break;
        case P_PLAY:
            updateGamePlay(pt, players);
        break;
        case P_GOAL:
            updateGameGoal(pt, players);
        break;
    }
}

void updateHostGame(P_TEST *pt, P plrs[])
{
    updatePlayer(&plrs[0], pt);
    animatePlayer(&plrs[0]);

    if (!plrs[pt->network.numplayers - 1].id)
    {
        if (pt->network.players_net[pt->network.numplayers - 1].id)
        {
            plrs[pt->network.numplayers - 1].id = (
                pt->network.players_net[pt->network.numplayers - 1].id);

            plrs[pt->network.numplayers - 1].spawned = true;

            printf("PLAYER: id %d\n", plrs[pt->network.numplayers - 1].id);
        }
    }

    for (unsigned char i = 1; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            for (unsigned char j = 1; j < MAX_NET_USERS; j++)
            {
                if (plrs[i].id == pt->network.players_net[j].id)
                {
                    plrs[i].x = pt->network.players_net[j].x;
                    plrs[i].y = pt->network.players_net[j].y;
                    break;
                }
            }
            updatePlayer(&plrs[i], pt);

            switch (plrs[i].state)
            {
                case PLR_SKATE_NP:
                    if (!pt->puck.hit)
                    {
                        bool grab = false;

                        for (int p = 0; p < MAX_GAME_USERS; p++) 
                        {
                            if (plrs[p].state == PLR_SKATE_WP) 
                            {
                                grab = true; 
                                break;
                            }
                            else if (pt->goalie[p].grab)
                            {
                                grab = true; 
                                break;
                            }
                        }

                        if (!grab)
                        {
                            if (checkCollision(plrs[i].r, pt->puck.r))
                            {
                                pt->puck.xvel = 0;
                                pt->puck.yvel = 0;
                                pt->puck.fvelx = 0.01f;
                                pt->puck.fvely = 0.01f;
                                pt->puck.x = plrs[i].club_r.x + (plrs[i].club_r.w >> 1);
                                pt->puck.y = plrs[i].club_r.y + (plrs[i].club_r.h >> 1);

                                plrs[i].pvel = 2;
                                plrs[i].state = PLR_SKATE_WP;

                                Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                            }
                        }
                    }
                break;
                case PLR_SKATE_WP:
                break;
                case PLR_SWING:
                    // check if another player has the puck
                    for (int p = 0; p < MAX_GAME_USERS; p++)
                    {
                        if (&plrs[i] != &plrs[p])
                        {
                            if (plrs[p].state == PLR_SKATE_WP
                            && checkCollision(plrs[i].club_r, plrs[p].club_r))
                            {
                                // puck goes wild
                                pt->puck.hit = true;
                                pt->puck.xvel = SDL_cos(plrs[i].AIM_angle);
                                pt->puck.yvel = SDL_sin(plrs[i].AIM_angle);

                                pt->PUCK_freeze = true;

                                plrs[p].state = PLR_SKATE_NP;

                                Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                                break;
                            }
                        }
                    }
                break;
                default: break;
            }
            animatePlayer(&plrs[i]);
        }
    }

    if (!pt->PUCK_freeze)
    {
        SDL_Rect *r = &pt->puck.r;
        bool grab = false;
        unsigned char i = 0;

        for (; i < 2; i++)
        {
            if (pt->goalie[i].grab)
            {
                r = &pt->goalie[i].r;
                grab = true;
                break;
            }
        }

        for (i = 0; i < pt->network.numplayers; i++) 
        {
            if (plrs[i].state == PLR_SKATE_WP) 
            {
                r = &plrs[i].r;
                grab = true;
                break;
            }
        }

        updatePuck(pt, plrs);
        updateGoalKeepers(pt, plrs, grab);

        for (i = 0; i < 2; i++)
        {
            if (checkGoal(pt->puck.r, pt->goal_r[i]))
                pt->p_state = P_GOAL;
        }

        pt->rx = r->x;
        pt->ry = r->y;
    }
}

void updateClientGame(P_TEST *pt, P plrs[])
{
    updatePlayer(pt->c_player, pt);
    animatePlayer(pt->c_player);

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            for (unsigned char j = 0; j < pt->network.numplayers; j++)
            {
                if ((plrs[i].id == pt->network.players_net[j].id) 
                && (pt->network.players_net[j].id != pt->network.localplayer->id))
                {
                    plrs[i].x = pt->network.players_net[j].x;
                    plrs[i].y = pt->network.players_net[j].y;

                    updatePlayer(&plrs[i], pt);
                    animatePlayer(&plrs[i]);
                    break;
                }
            }
        }
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
    }

    pt->rx = pt->puck.x;
    pt->ry = pt->puck.y;

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            plrs[i].r.x = plrs[i].x - 10;
            plrs[i].r.y = plrs[i].y;

            if (plrs[i].state == PLR_SKATE_WP)
            {
                pt->rx = plrs[i].r.x;
                pt->ry = plrs[i].r.y;
            }

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
            for (unsigned char j = 0; j < pt->network.numplayers; j++)
            {
                if (plrs[i].id == pt->network.players_net[j].id)
                {
                    if (pt->network.players_net[j].id != pt->c_player->id)
                    {
                        plrs[i].state = pt->network.players_net[j].state;
                        plrs[i].x = pt->network.players_net[j].x;
                        plrs[i].y = pt->network.players_net[j].y;
                    }
                    else if (pt->network.players_net[j].id == pt->c_player->id)
                    {
                        if (pt->network.players_net[j].state == PLR_SHOOT)
                        {
                            printf("PLAYER: %d shoot from host\n", pt->c_player->id);
                            plrs[i].state = PLR_SKATE_NP;
                            pt->puck.grab = false;
                        }
                    }

                    if (plrs[i].id == pt->network.puck.id)
                    {
                        if (plrs[i].state == PLR_SKATE_NP)
                            plrs[i].state = PLR_SKATE_WP;

                        pt->puck.grab = true;
                    }
                    else if (!pt->network.puck.id)
                    {
                        if (plrs[i].state == PLR_SKATE_WP)
                            plrs[i].state = PLR_SKATE_NP;

                        pt->puck.grab = false;
                    }
                }
            }
        }
    }
}

void updateNetHostGame(P_TEST *pt, P plrs[])
{
    if (!plrs[pt->network.numplayers - 1].id)
    {
        if (pt->network.players_net[pt->network.numplayers - 1].id)
        {
            plrs[pt->network.numplayers - 1].id = (
                pt->network.players_net[pt->network.numplayers - 1].id);

            plrs[pt->network.numplayers - 1].spawned = true;

            printf("PLAYER: id %d\n", plrs[pt->network.numplayers - 1].id);
        }
    }

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            for (unsigned char j = 0; j < pt->network.numplayers; j++)
            {
                if (plrs[i].id == pt->network.players_net[j].id)
                {
                    if (pt->network.players_net[j].id != pt->c_player->id)
                    {
                        if (!(pt->network.players_net[j].state == PLR_SKATE_WP && !pt->puck.grab))
                            plrs[i].state = pt->network.players_net[j].state;
                        else 
                            plrs[i].state = PLR_SKATE_NP;

                        plrs[i].x = pt->network.players_net[j].x;
                        plrs[i].y = pt->network.players_net[j].y;
                    }
                }
            }
        }
    }

    updatePlayerInputs(pt->c_player);

    if (pt->c_player->JOY_vel)
    {
        float   cos = (
            SDL_cos(pt->c_player->INPUT_angle) * pt->c_player->vel * pt->c_player->JOY_vel),
                sin = (
            SDL_sin(pt->c_player->INPUT_angle) * pt->c_player->vel * pt->c_player->JOY_vel);

        pt->c_player->xvel = cos;
        pt->c_player->yvel = sin;
    }
    else
    {
        pt->c_player->xvel = 0;
        pt->c_player->yvel = 0;
    }

    if (pt->c_player->xvel) 
        updatePlayerX(pt->c_player, pt->level);

    if (pt->c_player->yvel) 
        updatePlayerY(pt->c_player, pt->level);

    for (unsigned char i = 0; i < MAX_GAME_USERS; i++)
    {
        if (plrs[i].spawned)
        {
            switch (plrs[i].state)
            {
                default: break;
                case PLR_SKATE_NP:
                    if (!pt->puck.grab && !pt->puck.hit)
                    {
                        if (checkCollision(plrs[i].r, pt->puck.r))
                        {
                            printf("PLAYER: %d pick up puck\n", plrs[i].id);
                            pt->puck.grab = true;
                            plrs[i].state = PLR_SKATE_WP;
                            pt->network.puck.id = plrs[i].id;
                        }
                    }
                break;
                case PLR_SHOOT:
                    if (!pt->puck.hit)
                    {
                        printf("PLAYER: %d shoot puck\n", plrs[i].id);
                    
                        shootPuck(
                            &pt->puck, 
                            plrs[i].pvel, 
                            plrs[i].x, 
                            plrs[i].y, 
                            plrs[i].AIM_angle
                        );

                        pt->PUCK_freeze = true;
                        
                        plrs[i].state = PLR_SKATE_NP;
                        pt->network.puck.id = 0;

                        //Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                    }
                break;
            }

            for (unsigned char j = 0; j < pt->network.numplayers; j++)
            {
                if (plrs[i].id == pt->network.players_net[j].id)
                {
                    pt->network.players_net[j].state = plrs[i].state;
                }
            }
        }
    }

    updatePuck(pt, plrs);

    pt->network.puck.x = pt->puck.x;
    pt->network.puck.y = pt->puck.y;
}

void updateNetClientGame(P_TEST *pt, P plrs[])
{
    pt->puck.x = pt->network.puck.x;
    pt->puck.y = pt->network.puck.y;

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;

    if (pt->PUCK_freeze && (++pt->PUCK_freeze_timer > 4))
    {
        pt->PUCK_freeze = false;
        pt->PUCK_freeze_timer = 0;
    }

    if (pt->puck.hit && (++pt->puck.hit_counter > 60)) 
    {
        pt->puck.hit = false;
        pt->puck.hit_counter = 0;
    }

    updateNetPlayers(pt, plrs);

    /*
    if (pt->network.pgrab)
    {
        pt->network.pgrab = false;
        pt->network.pshoot = false;
        if (pt->network.localplayer->state == PLR_SKATE_NP)
        {
            if (!pt->puck.grab && !pt->puck.hit)
            {
                pt->puck.grab = true;
                pt->c_player->state = PLR_SKATE_WP;
            }
        }
    }
    else if (pt->network.pshoot)
    {
        pt->network.pshoot = false;
        pt->network.pgrab = false;
        if (pt->network.localplayer->state == PLR_SHOOT)
        {
            pt->puck.hit = true;
            pt->puck.grab = false;
            
            pt->c_player->state = PLR_SKATE_NP;
            pt->PUCK_freeze = true;
            //Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
        }
    }

    if (pt->network.localplayer->state == PLR_SKATE_NP)
    {
        if (!pt->puck.grab && !pt->puck.hit)
        {
            if (checkCollision(pt->c_player->r, pt->puck.r))
            {
                pt->c_player->state = PLR_SKATE_WP;
            }
        }
    }
    */

    updatePlayerInputs(pt->c_player);

    if (pt->c_player->JOY_vel)
    {
        float   cos = (
            SDL_cos(pt->c_player->INPUT_angle) * pt->c_player->vel * pt->c_player->JOY_vel),
                sin = (
            SDL_sin(pt->c_player->INPUT_angle) * pt->c_player->vel * pt->c_player->JOY_vel);

        pt->c_player->xvel = cos;
        pt->c_player->yvel = sin;
    }
    else
    {
        pt->c_player->xvel = 0;
        pt->c_player->yvel = 0;
    }

    if (pt->c_player->xvel) 
        updatePlayerX(pt->c_player, pt->level);

    if (pt->c_player->yvel) 
        updatePlayerY(pt->c_player, pt->level);
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

void setupPlay(P_TEST *pt, P *player)
{
    pt->players = player;
    pt->c_player = player;

    pt->camera.x = pt->c_player->x - (pt->camera.w >> 1);
    pt->camera.y = pt->c_player->y - (pt->camera.h >> 1);
    
    pt->p_state = P_PLAY;

    pt->c_player->club_r.x = pt->c_player->x - 8 + pt->c_player->AIM_radx;
    pt->c_player->club_r.y = pt->c_player->y - 8 + pt->c_player->AIM_rady;

    pt->puck.x = pt->level.r.w >> 1;
    pt->puck.y = pt->level.r.h >> 1;

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;
    pt->puck.r.w = 8;
    pt->puck.r.h = 4;

    pt->puck.xvel = 0;
    pt->puck.yvel = 0;
    pt->puck.hit = false;
    pt->puck.grab = false;
    pt->puck.hit_counter = 0;

    pt->c_player->r.x = pt->c_player->x - pt->camera.x;
    pt->c_player->r.y = pt->c_player->y - pt->camera.y;

    pt->g_state = G_PLAY;
}

void setupGame(P_TEST *pt, SDL_Rect *gr, SDL_Rect *gkr, P_G *gkeep)
{
    pt->is_net = false;

    pt->PUCK_freeze = false;
    pt->PUCK_freeze_timer = 0;

    for (unsigned char i = 0; i < 64; i++)
        pt->input_field.string[i] = 0;

    pt->input_field.str_pointer = 0;
    pt->input_field.str_len = 0;

    /*
    int y = 0;
    for (int i = 0; i < 25; i++)
    {
        pt->gunClips[i].w = 17;
        pt->gunClips[i].h = 19;
        pt->gunClips[i].x = ((i % 4) << 4) + 1;
        pt->gunClips[i].y = (y << 4) + 2;

        if ((i % 4) == 3) y++;
    }

    play_test.bullet_hits.index = 0;

    for (int i = 0; i < 30; i++)
    {
        play_test.bullet_hits.a[i].used = false;
        play_test.bullet_hits.a[i].x = 0;
        play_test.bullet_hits.a[i].y = 0;
    }
    */

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

    for (unsigned char i = 0; i < 2; i++)
    {
        initGoalkeeper(&goalie[i]);
        if (i == 1) goalie[i].x = 742;
    }
}

void addPlayerGame(P *p, unsigned char id, int x, int y)
{
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

void resetPlay(P_TEST *pt, P *player)
{
    pt->camera.x = 0;
    pt->camera.y = 0;

    player->xvel = 0;
    player->yvel = 0;

    player->state = PLR_SKATE_NP;

    for (int i = 0; i < 4; i++)
        pt->c_player->input_q[i] = 0;
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
    p->vel = STANDARD_VELOCITY;
    p->xvel = 0;
    p->yvel = 0;
    p->pvel = 2;

    p->sprint = false;
    p->sprint_cdown = false;
    p->m_hold = false;
    p->m_move = false;
    p->bounce = false;
    p->spawned = false;

    p->sprint_timer = 0;
    p->sprint_cdown_timer = 0;
    p->block_timer = 0;

    p->c_index = 0;
    p->facing = 0;
    p->a_index = 0;

    p->x = p->mx << 4;
    p->y = p->my << 4;
    p->r.x = p->x;
    p->r.y = p->y;

    p->club_r.x = p->x - 8 + p->AIM_radx;
    p->club_r.y = p->y - 8 + p->AIM_rady;
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
    p->hit = false;
    p->grab = false;
    p->hit_counter = 0;
}

void resetInputField(I_FIELD *input)
{
    for (unsigned char i = 0; i < 64; i++)
        input->string[i] = 0;

    input->str_len = 0;
    input->str_pointer = 0;
}

void movePlayer(P *p, SDL_Rect camera)
{
    if (p->JOY_use)
    {
        if (p->JOY_xdir == 0 && p->JOY_ydir == 0)
        {
            p->JOY_vel = 0;
            p->input_q[0] = 0;
        }
        else 
        {
            p->INPUT_angle = SDL_atan2(p->JOY_ydir, p->JOY_xdir);
        }

        if (p->AIM_xdir || p->AIM_ydir)
        {
            p->AIM_angle = SDL_atan2(p->AIM_yvel, p->AIM_xvel);

            p->crosshair.r.x = p->x - camera.x + (AIM_RADIUS * SDL_cos(p->AIM_angle));
            p->crosshair.r.y = p->y - camera.y + (AIM_RADIUS * SDL_sin(p->AIM_angle));
        }
        else 
        {
            p->crosshair.r.x = p->x - camera.x;
            p->crosshair.r.y = p->y - camera.y;
            p->crosshair.show = false;
            p->m_move = false;
        }
    }
    else 
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
        
        if (p->m_move)
        {
            p->AIM_angle = (SDL_atan2(
                camera.y + p->crosshair.r.y - p->y, 
                camera.x + p->crosshair.r.x - p->x));
        }
    }
}
