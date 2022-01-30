#include "util.h"

void closeSdl(SDL_Window **w, SDL_Renderer **r, Mix_Chunk *m[])
{
    if (*w != NULL)
    {
        SDL_DestroyWindow(*w);
        *w = NULL;
    }

    printf("free window\n");
    
    if (*r != NULL)
    {
        SDL_DestroyRenderer(*r);
        *r = NULL;
    }

    printf("free renderer\n");

    for (int i = 0; i < 2; i++)
    {
        if (m[i] != NULL)
        {
            Mix_FreeChunk(m[i]);
            m[i] = NULL;
        }
    }

    printf("free audio chunks\n");

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
            "Level Editor", 
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

    printf("init texture %s, %d\n", str, pt->texture.t);

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
            strncpy(n_str, pt->f_buffer, 64);

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
        pt->level.map = calloc(pt->level.t_map_pieces << 1, 1);
    }

    if (pt->level.collision == NULL)
        pt->level.collision = calloc(pt->level.t_map_pieces, 1);
    else
    {
        free(pt->level.collision);
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

    p->state = SKATE_NP;

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

    p->shoot = false;
    p->sprint = false;
    p->sprint_cdown = false;
    p->spawned = true;
    p->m_hold = false;
    p->m_move = false;
    p->block = false;
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
    g->x = 106;
    g->y = 256;
    g->r.w = 20;
    g->r.h = 20;
    g->s_timer = 0;
    g->shoot = false;
    g->swing = false;
    g->state = G_NORMAL;
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

void readInputs(SDL_Renderer *r, P_TEST *pt, SDL_Event ev)
{
    while (SDL_PollEvent(&ev) != 0)
    {
        
    }
}

void playRender(SDL_Renderer *r, FC_Font *f, P_TEST *pt, P players[])
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
    
    SDL_Rect gq = {
        pt->goal_r.x - pt->camera.x, 
        pt->goal_r.y - pt->camera.y, 
        pt->goal_r.w, 
        pt->goal_r.h
    }, gkq = {
        pt->gk_r->x - pt->camera.x, 
        pt->gk_r->y - pt->camera.y,
        pt->gk_r->w, pt->gk_r->h
    };

    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
    SDL_RenderFillRect(r, &gq);

    SDL_RenderFillRect(r, &gkq);
    
    if (pt->state == P_SCORE)
    {
        FC_DrawColor(
            f, r, 
            320, 20, 
            FC_MakeColor(0xff, 0xff, 0x00, 0xff), 
            "SCORE!");
    }

    /*
    for (int i = 0; i < 10; i++)
    {
        if (player->bullets[i].shoot)
        {
            SDL_Rect q = {
                player->bullets[i].x, 
                player->bullets[i].y, 
                4, 4};

            if (checkCollision(q, pt->camera))
            {
                SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderFillRect(r, &player->bullets[i].r);
            }
        }
    }

    for (int i = 0; i < 30; i++)
    {
        if (pt->bullet_hits.a[i].used)
        {
            SDL_Rect q = {
                pt->bullet_hits.a[i].x, 
                pt->bullet_hits.a[i].y, 
                4, 4};

            if (checkCollision(q, pt->camera))
            {
                q.x = pt->bullet_hits.a[i].x - pt->camera.x;
                q.y = pt->bullet_hits.a[i].y - pt->camera.y;

                SDL_SetRenderDrawColor(r, 0x30, 0x30, 0x30, 0xff);
                SDL_RenderFillRect(r, &q);
            }
        }
    }

    // for testing gun sprite
    SDL_Rect q = {
        player->x - 8 + pt->GUN_radx - pt->camera.x,
        player->y - 8 + pt->GUN_rady - pt->camera.y,
        17, 19};
    SDL_RenderCopyEx(
        r, 
        pt->gunTexturpt->t, 
        &pt->gunClips[3], 
        &q, 
        pt->GUN_deg, 
        NULL, 
        (pt->GUN_deg > 90 && pt->GUN_deg < 270) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);
    */

   SDL_Rect kq = {
       pt->goalie.r.x - pt->camera.x, 
       pt->goalie.r.y - pt->camera.y,
       pt->goalie.r.w, pt->goalie.r.h
    };

    SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);
    SDL_RenderFillRect(r, &kq);

    for (int i = 0; i < 2; i++)
    {
        if (players[i].spawned)
        {
            SDL_Rect cq = {
                players[i].club_r.x - pt->camera.x, 
                players[i].club_r.y - pt->camera.y, 
                players[i].club_r.w, players[i].club_r.h
            },
            pq = {
                players[i].r.x - pt->camera.x, 
                players[i].r.y - pt->camera.y, 
                players[i].r.w, players[i].r.h
            };

            // player hitbox
            if (players[i].block) 
                SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
            else 
                SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);

            SDL_RenderFillRect(r, &pq);

            // club hitbox
            if (!players[i].block)
            {
                if (players[i].swing) 
                    SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
                else 
                    SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);

                if (players[i].pvel == 5) 
                    SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);

                SDL_RenderFillRect(r, &cq);
            }
            
            // player texture
            if (checkCollision(players[i].r, pt->camera))
            {
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
                    ny = players[i].r.y - pt->camera.y;

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

            if (players[i].crosshair.show) 
            {
                SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderFillRect(r, &players[i].crosshair.r);
            }
        }
    }

    SDL_Rect pq = {
        pt->puck.r.x - pt->camera.x, 
        pt->puck.r.y - pt->camera.y, 
        pt->puck.r.w, pt->puck.r.h
    };

    // puck hitbox
    SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(r, &pq);
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

void playTopDownShooter(P_TEST *pt, SDL_Event ev)
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
                            pt->w_focus = !pt->w_focus;
                            if (pt->w_focus) SDL_SetRelativeMouseMode(SDL_ENABLE);
                            else SDL_SetRelativeMouseMode(SDL_DISABLE);
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
                            if (!pt->c_player->sprint_cdown && !pt->c_player->swing) 
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
                if (ev.button.button == SDL_BUTTON_LEFT && pt->w_focus)
                {
                    if (!pt->c_player->block)
                    {
                        if (!pt->c_player->grab)
                        {
                            if (!pt->c_player->swing && !pt->c_player->m_hold)
                            {
                                pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                pt->c_player->swing = true;
                                Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                            }
                        
                            pt->c_player->m_hold = true;
                        }
                        else pt->c_player->m_hold = true;
                    }
                }
            break;
            case SDL_MOUSEBUTTONUP:
                if (ev.button.button == SDL_BUTTON_LEFT && pt->w_focus)
                {
                    if (!pt->c_player->block)
                    {
                        if (!pt->c_player->grab) pt->c_player->m_hold = false;
                        else 
                        {
                            if (!pt->c_player->swing && pt->c_player->m_hold)
                            {
                                pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                pt->c_player->swing = true;
                                Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                            }
                            pt->c_player->m_hold = false;
                        }
                    }
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
                            if (!pt->c_player->block)
                            {
                                if (!pt->c_player->grab)
                                {
                                    if (ev.jaxis.value > -16000)
                                    {
                                        if (!pt->c_player->swing && !pt->c_player->m_hold) 
                                        {
                                            pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                            pt->c_player->swing = true;
                                            Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                                        }
                                        pt->c_player->m_hold = true;
                                    } 
                                    else pt->c_player->m_hold = false;
                                }
                                else 
                                {
                                    if (ev.jaxis.value > -16000) pt->c_player->m_hold = true;
                                    else 
                                    {
                                        if (!pt->c_player->swing && pt->c_player->m_hold) 
                                        {
                                            pt->c_player->vel -= pt->c_player->gvel * 0.25f;
                                            pt->c_player->swing = true;
                                            Mix_PlayChannel(-1, pt->mix_chunks[S_MEDIUM], 0);
                                        }
                                        pt->c_player->m_hold = false;
                                    }
                                }
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
                        if (!pt->c_player->sprint_cdown && !pt->c_player->swing && !pt->c_player->block) 
                        {
                            pt->c_player->sprint = true;
                            pt->c_player->gvel = SPRINT_VELOCITY;
                            pt->c_player->vel = SPRINT_VELOCITY;
                        }
                    }
                    else if (ev.jbutton.button == SDL_CONTROLLER_BUTTON_B)
                    {
                        if (!pt->c_player->block && !pt->c_player->swing && !pt->c_player->shoot && !pt->c_player->grab)
                        {
                            pt->c_player->block = true;
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

    if (*gky < 224) *gky = 224;
    else if (*gky > 288) *gky = 288;
}

void updateBulletHits(B_HITS *hits, int bx, int by)
{
    hits->a[hits->index].x = bx;
    hits->a[hits->index].y = by;

    hits->a[hits->index].used = true;

    if (++hits->index > 29) hits->index = 0;
}

void updateTopDownShoot(P_TEST *pt, P players[])
{
    switch (pt->state)
    {
        case P_PLAY:
        break;
        case P_SCORE:
            if (++pt->SCORE_timer > 240)
            {
                /*
                for (int i = 0; i < 2; i++)
                {
                    resetPlayer(&players[i]);
                    players[i].x = (players[i].mx << pt->level.t_bit_size) + (pt->level.t_size >> 1);
                    players[i].y = (players[i].my << pt->level.t_bit_size) + (pt->level.t_size >> 1);

                    players[i].club_r.x = players[i].x - 8 + players[i].AIM_radx;
                    players[i].club_r.y = players[i].y - 8 + players[i].AIM_rady;
                }
                */
                /*
                pt->puck.x = pt->screen.w >> 1;
                pt->puck.y = pt->screen.h >> 1;
                pt->puck.r.x = pt->puck.x;
                pt->puck.r.y = pt->puck.y;
                pt->puck.xvel = 0;
                pt->puck.yvel = 0;
                pt->puck.fvel = 0.01f;
                pt->puck.hit = false;
                pt->puck.hit_counter = 0;
                pt->sprint_hud_r.w = 0;
                */

                //pt->camera.x = players[0].x - (pt->camera.w >> 1);
                //pt->camera.y = players[0].y - (pt->camera.h >> 1);

                //pt->goalie.y = 256;

                pt->SCORE_timer = 0;

                pt->state = P_PLAY;
            }
        break;
    }

    for (int i = 0; i < 2; i++)
    {
        if (players[i].spawned)
        {
            updatePlayer(&players[i], pt);

            if (players[i].swing && !players[i].grab && !pt->puck.hit && !players[i].block)
            {
                // check if another player has the puck
                for (int j = 0; j < 2; j++)
                {
                    if (&players[i] != &players[j])
                    {
                        if (players[j].grab 
                        && checkCollision(players[i].club_r, players[j].club_r))
                        {
                            // puck goes wild
                            pt->puck.hit = true;
                            pt->puck.xvel = SDL_cos(players[i].AIM_angle);
                            pt->puck.yvel = SDL_sin(players[i].AIM_angle);

                            pt->PUCK_freeze = true;

                            players[i].swing_timer = 0;
                            players[i].vel = players[i].gvel;

                            players[j].grab = false;

                            Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                            break;
                        }
                    }
                }
            }
            else if (!players[i].swing && !players[i].grab && !pt->puck.hit && !players[i].block)
            {
                int n = 0;

                for (int j = 0; j < 2; j++) if (players[j].grab) n++;

                if (n == 0)
                {
                    if (checkCollision(players[i].r, pt->puck.r))
                    {
                        pt->puck.xvel = 0;
                        pt->puck.yvel = 0;
                        pt->puck.fvel = 0.01f;
                        pt->puck.fvelx = 0.01f;
                        pt->puck.fvely = 0.01f;
                        pt->puck.x = players[i].club_r.x + (players[i].club_r.w >> 1);
                        pt->puck.y = players[i].club_r.y + (players[i].club_r.h >> 1);

                        players[i].pvel = 3;
                        players[i].grab = true;

                        Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                    }
                }
            }

            animatePlayer(&players[i]);
        }
    }

    //update puck

    if (pt->PUCK_freeze && (++pt->PUCK_freeze_timer > 2))
    {
        pt->PUCK_freeze = false;
        pt->PUCK_freeze_timer = 0;
    }

    if (pt->puck.hit && (pt->puck.hit_counter++ > 16)) 
    {
        pt->puck.hit = false;
        pt->puck.hit_counter = 0;
    }

    if (!pt->PUCK_freeze)
    {
        if (pt->puck.xvel > 0)
        {
            pt->puck.xvel -= pt->puck.fvel;
            if (pt->puck.xvel < 0) 
                pt->puck.xvel = 0;
        }
        else if (pt->puck.xvel < 0)
        {
            pt->puck.xvel += pt->puck.fvel;
            if (pt->puck.xvel > 0) 
                pt->puck.xvel = 0;
        }
        
        if (pt->puck.yvel > 0)
        {
            pt->puck.yvel -= pt->puck.fvel;
            if (pt->puck.yvel < 0) 
                pt->puck.yvel = 0;
        }
        else if (pt->puck.yvel < 0)
        {
            pt->puck.yvel += pt->puck.fvel;
            if (pt->puck.yvel > 0) 
                pt->puck.yvel = 0;
        }

        if (pt->puck.xvel)
        {
            float px = pt->puck.x + pt->puck.xvel;

            for (int i = 0; i < 2; i++)
            {
                if (players[i].block && !players[i].bounce)
                {
                    if (checkPuckCollision(px, pt->puck.y, players[i].r))
                    {
                        players[i].bounce = true;
                        pt->puck.xvel = -pt->puck.xvel;
                        pt->puck.fvel += 0.15f;
                    }
                }
            }

            if (checkPuckCollision(px, pt->puck.y, pt->goalie.r))
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvel += 0.05f;
            }

            if (checkPlayerPosition(
                (int)px >> pt->level.t_bit_size, 
                (int)pt->puck.y >> pt->level.t_bit_size, 
                pt->level.collision, pt->level.t_map_h)
            )
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvel += 0.05f;
            }
            else if (px < 0 || px > pt->level.r.w) 
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvel += 0.05f;
            }
            else pt->puck.x = px;
        }

        if (pt->puck.yvel)
        {
            float py = pt->puck.y + pt->puck.yvel;

            for (int i = 0; i < 2; i++)
            {
                if (players[i].block && !players[i].bounce)
                {
                    if (checkPuckCollision(pt->puck.x, py, players[i].r))
                    {
                        players[i].bounce = true;
                        pt->puck.yvel = -pt->puck.yvel;
                        pt->puck.fvel += 0.15f;
                    }
                }
            }

            if (checkPuckCollision(pt->puck.x, py, pt->goalie.r))
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvel += 0.05f;
            }

            if (checkPlayerPosition(
                (int)pt->puck.x >> pt->level.t_bit_size, 
                (int)py >> pt->level.t_bit_size, 
                pt->level.collision, pt->level.t_map_h)
            )
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvel += 0.05f;
            }
            else if (py < 0 || py > pt->level.r.h) 
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvel += 0.05f;
            }
            else pt->puck.y = py;
        }

        pt->puck.r.x = pt->puck.x;
        pt->puck.r.y = pt->puck.y;

        if (!pt->puck.xvel && !pt->puck.yvel)
            pt->puck.fvel = 0.01f;

        SDL_Rect *r = &pt->puck.r;
        bool grab = false;

        for (unsigned char i = 0; i < 2; i++) 
        {    if (players[i].grab) 
            {
                r = &players[i].r;
                grab = true;
                break;
            }
        }

        // update goalkeeper

        if (pt->state != P_SCORE && !pt->PUCK_freeze)
        {
            if (!pt->puck.xvel && !pt->puck.yvel)
            {
                if (!grab)
                {
                    if (checkCollision(pt->puck.r, pt->goalie.r))
                    {
                        // swing/shoot?
                        // puck goes to center
                        pt->puck.hit = true;
                        pt->puck.x = pt->goalie.x + 10;
                        pt->puck.y = pt->goalie.y;
                        pt->puck.xvel = 2;
                        pt->puck.yvel = 0;

                        pt->PUCK_freeze = true;

                        pt->goalie.state = G_NORMAL;

                        Mix_PlayChannel(-1, pt->mix_chunks[S_LOW], 0);
                    }
                    else 
                    {
                        if (pt->goalie.state == G_NORMAL)
                        {
                            if (checkCollision(pt->puck.r, *pt->gk_r))
                            {
                                pt->goalie.state = G_CLEAR_GOAL;
                            }
                            /*
                            if (pt->puck.y < 304 && pt->puck.y > 208)
                            {
                                if (pt->puck.x - pt->goalie.x < 30 
                                && pt->puck.x - pt->goalie.x > 0)
                                {
                                    pt->goalie.state = G_CLEAR_GOAL;
                                }
                            }
                            */
                        }
                    }
                }
                else pt->goalie.state = G_NORMAL;
            }

            if (pt->goalie.state == G_NORMAL)
            {
                float f = 0.85f;

                if (pt->goalie.x > 106) 
                {
                    if ((pt->goalie.x -= 1.125f) < 106)
                        pt->goalie.x = 106;
                }
                else if (pt->goalie.x < 106) 
                {
                    if ((pt->goalie.x += 1.125f) > 106)
                        pt->goalie.x = 106;
                }

                if (r->x - pt->goalie.x < 50)
                    f = 1.75f;
                else if (r->x - pt->goalie.x < 100)
                    f = 1.5f;
                else if (r->x - pt->goalie.x < 150)
                    f = 1.25f;
                else if (r->x - pt->goalie.x < 200)
                    f = 1.125f;

                if (r->y < 224)
                {
                    if (pt->goalie.y < 224) pt->goalie.y += f;
                    else adjustGoalie(&pt->goalie.y, r->y, f);
                }
                else if (r->y > 288)
                {
                    if (pt->goalie.y > 288) pt->goalie.y -= f;
                    else adjustGoalie(&pt->goalie.y, r->y, f);
                }
                else adjustGoalie(&pt->goalie.y, r->y, f);
            }
            else if (pt->goalie.state == G_CLEAR_GOAL)
            {
                if ((pt->goalie.x += 1.125f) > pt->puck.x)
                    pt->goalie.x = pt->puck.x;

                if (pt->puck.y > pt->goalie.y)
                    pt->goalie.y += 1.125f;
                else if (pt->puck.y < pt->goalie.y)
                    pt->goalie.y -= 1.125f;
            }
        }

        pt->goalie.r.x = pt->goalie.x - 10;
        pt->goalie.r.y = pt->goalie.y - 10;

        pt->rx = r->x; 
        pt->ry = r->y;

        if (!pt->state == P_SCORE 
        && checkGoal(pt->puck.r, pt->goal_r))
        {
            pt->state = P_SCORE;
        }
    }

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
}

void updateGame(P_TEST *p, P players[], Mix_Chunk *chunks[])
{

}

void updatePlayer(P *p, P_TEST *pt)
{
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

    if (p->block)
    {
        p->JOY_vel = 1;

        if ((p->vel -= 0.05f) < 0) p->vel = 0;

        if (++p->block_timer > 60)
        {
            p->bounce = false;
            p->block = false;
            p->block_timer = 0;
            p->vel = STANDARD_VELOCITY;

            p->r.w = 20;
            p->r.h = 20;
        }
    }

    if (p->m_hold)
    {
        if (p->grab && ((p->pvel += 0.1f) > 5))
            p->pvel = 5;
    }

    if (p->swing)
    {
        p->JOY_vel = 0;

        if (p->grab)
        {
            pt->puck.x = p->x;
            pt->puck.y = p->y;
            pt->puck.xvel = p->pvel * SDL_cos(p->AIM_angle);
            pt->puck.yvel = p->pvel * SDL_sin(p->AIM_angle);
            pt->puck.hit = true;

            pt->PUCK_freeze = true;

            p->swing_timer = 0;
            p->vel = p->gvel;
            p->grab = false;
        }
        
        if (++p->swing_timer > (p->grab ? 20 : 10))
        {
            p->swing = false;
            p->swing_timer = 0;
            p->vel = p->gvel;
            p->pvel = 2;
        }
    }

    if (p->grab)
    {
        pt->puck.x = p->club_r.x + (p->club_r.w >> 1);
        pt->puck.y = p->club_r.y + (p->club_r.h >> 1);
    }

    if (p->JOY_use)
    {
        if (!p->block)
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
        }

        if (p->AIM_xdir || p->AIM_ydir)
        {
            p->AIM_angle = SDL_atan2(p->AIM_yvel, p->AIM_xvel);

            p->crosshair.r.x = p->x - pt->camera.x + (AIM_RADIUS * SDL_cos(p->AIM_angle));
            p->crosshair.r.y = p->y - pt->camera.y + (AIM_RADIUS * SDL_sin(p->AIM_angle));
        }
        else 
        {
            p->crosshair.r.x = p->x - pt->camera.x;
            p->crosshair.r.y = p->y - pt->camera.y;
            p->crosshair.show = false;
            p->m_move = false;
        }
    }
    else 
    {
        if (!p->block)
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
        
        if (p->m_move && !p->swing)
        {
            p->AIM_angle = (SDL_atan2(
                pt->camera.y + p->crosshair.r.y - p->y, 
                pt->camera.x + p->crosshair.r.x - p->x));
        }
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

    if (p->xvel)
    {
        float x = p->x + p->xvel;

        if (!checkPlayerPosition(
            (int)x >> pt->level.t_bit_size, 
            (int)p->y >> pt->level.t_bit_size, 
            pt->level.collision, 
            pt->level.t_map_h)
        )
        {
            if (!(x < 0 || x > pt->level.r.w)) p->x = x;
            else p->xvel = 0;
        }
        else p->xvel = 0;
    }

    if (p->yvel)
    {
        float y = p->y + p->yvel;

        if (!checkPlayerPosition(
            (int)p->x >> pt->level.t_bit_size, 
            (int)y >> pt->level.t_bit_size, 
            pt->level.collision, 
            pt->level.t_map_h)
        )
        {
            if (!(y < 0 || y > pt->level.r.h)) p->y = y;
            else p->yvel = 0;
        } 
        else p->yvel = 0;  
    }

    if (p->block)
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
    
    if (!p->crosshair.show)
    {
        if (!p->swing) p->AIM_angle = p->INPUT_angle;
    }

    p->AIM_radx = 10 * SDL_cos(p->AIM_angle);
    p->AIM_rady = 10 * SDL_sin(p->AIM_angle);

    p->club_r.x = p->x - 8 + p->AIM_radx;
    p->club_r.y = p->y - 8 + p->AIM_rady;

    p->AIM_deg = (p->AIM_angle * 180) / M_PI;

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
    pt->c_player = player;

    pt->camera.x = pt->c_player->x - (pt->camera.w >> 1);
    pt->camera.y = pt->c_player->y - (pt->camera.h >> 1);
    
    pt->state = P_PLAY;

    pt->c_player->club_r.x = pt->c_player->x - 8 + pt->c_player->AIM_radx;
    pt->c_player->club_r.y = pt->c_player->y - 8 + pt->c_player->AIM_rady;

    pt->puck.x = pt->c_player->x;
    pt->puck.y = pt->c_player->y;

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;
    pt->puck.r.w = 8;
    pt->puck.r.h = 4;

    pt->puck.xvel = 0;
    pt->puck.yvel = 0;
    pt->puck.hit = false;
    pt->puck.hit_counter = 0;

    pt->c_player->r.x = pt->c_player->x - pt->camera.x;
    pt->c_player->r.y = pt->c_player->y - pt->camera.y;
}

void resetPlay(P_TEST *pt, P *player)
{
    pt->camera.x = 0;
    pt->camera.y = 0;

    player->xvel = 0;
    player->yvel = 0;

    player->state = SKATE_NP;

    for (int i = 0; i < 4; i++)
        pt->c_player->input_q[i] = 0;
}

void resetPlayer(P *player)
{
    player->xvel = 0;
    player->yvel = 0;
    player->swing_timer = 0;
    player->swing = false;
    player->gvel = STANDARD_VELOCITY;
    player->vel = STANDARD_VELOCITY;
    player->pvel = 2;
    player->sprint = false;
    player->sprint_timer = 0;
    player->sprint_cdown = false;
    player->sprint_cdown_timer = 0;

    //player->crosshair.r.x = player->x - pt->camera.x;
    //player->crosshair.r.y = player->y - pt->camera.y;
    player->crosshair.show = false;
}
