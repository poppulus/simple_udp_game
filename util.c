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
    if ((puck.x < (goal.x + goal.w)) 
    && ((puck.x + puck.w) > goal.x) 
    && (puck.y < (goal.y + goal.h)) 
    && ((puck.y + puck.h) > goal.y))
    {
        return true;
    }

    return false;
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

void freePlayer(P *p)
{
    freeTexture(p->texture);
    p->t_clips = NULL;

    printf("free player texture\n");
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

void initPlayer(P *p, L l, unsigned char buffer[])
{
    p->c_index = 0;
    p->facing = 0;
    p->a_index = 0;

    p->shoot = false;
    p->gun_timer = 0;

    p->clip.w = 32;
    p->clip.h = 32;
    p->clip.x = 0;
    p->clip.y = 0;

    p->r.w = l.t_size;
    p->r.h = l.t_size;

    p->x = buffer[F_SPAWN_X] * buffer[F_SPAWN_X + 1];
    p->mx = (int)p->x >> l.t_bit_size;
    p->r.x = p->mx << l.t_bit_size;

    p->y = buffer[F_SPAWN_Y] * buffer[F_SPAWN_Y + 1];
    p->my = (int)p->y >> l.t_bit_size;
    p->r.y = p->my << l.t_bit_size;
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

void playRender(SDL_Renderer *r, FC_Font *f, P_TEST *pt, P *player)
{
    // background 
    SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(r, &pt->screen);

    // tiles
    renderPlayTiles(r, *pt);
    
    SDL_Rect gq = {
        pt->goal_r.x - pt->camera.x, 
        pt->goal_r.y - pt->camera.y, 
        pt->goal_r.w, 
        pt->goal_r.h
    };

    SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff);
    SDL_RenderFillRect(r, &gq);
    
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

    SDL_Rect pq = {
                pt->puck.r.x - pt->camera.x, 
                pt->puck.r.y - pt->camera.y, 
                pt->puck.r.w, pt->puck.r.h},
             cq = {
                pt->club_r.x - pt->camera.x, 
                pt->club_r.y - pt->camera.y, 
                pt->club_r.w, pt->club_r.h};

    // puck hitbox
    SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(r, &pq);

    // club hitbox
    if (player->swing) 
        SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff);
    else 
        SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff);

    SDL_RenderFillRect(r, &cq);

    // player
    renderPlayer(
        r, 
        player->texture->t,
        player, 
        pt->camera.x, 
        pt->camera.y);

    if (pt->crosshair.show) 
    {
        SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderFillRect(r, &pt->crosshair.r);
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

    if (p->facing == 3) p->c_index = 8 + p->a_index;
    else p->c_index = (p->facing * 4) + p->a_index;
}

void playTopDownShooter(P_TEST *pt, SDL_Event ev, P *player)
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
                    pt->JOY_use = false;

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
                            enqueue(pt->input_q, KEY_LEFT);
                        break;
                        case SDLK_w: 
                            enqueue(pt->input_q, KEY_UP);
                        break;
                        case SDLK_d: 
                            enqueue(pt->input_q, KEY_RIGHT);
                        break;
                        case SDLK_s: 
                            enqueue(pt->input_q, KEY_DOWN); 
                        break;
                        case SDLK_SPACE:
                        case SDLK_KP_SPACE:
                            if (!player->sprint_cdown && !player->swing) 
                            {
                                player->sprint = true;
                                player->gvel = SPRINT_VELOCITY;
                                player->vel = SPRINT_VELOCITY;
                            }
                        break;
                    }
                }
            break;
            case SDL_KEYUP:
                switch (ev.key.keysym.sym)
                {
                    case SDLK_a: 
                        dequeue(pt->input_q, KEY_LEFT);
                    break;
                    case SDLK_w: 
                        dequeue(pt->input_q, KEY_UP);
                    break;
                    case SDLK_d: 
                        dequeue(pt->input_q, KEY_RIGHT);
                    break;
                    case SDLK_s: 
                        dequeue(pt->input_q, KEY_DOWN);
                    break;
                    case SDLK_SPACE:
                    case SDLK_KP_SPACE:
                    break;
                }
            break;
            case SDL_MOUSEMOTION:
                if (pt->w_focus)
                {
                    pt->JOY_use = false;

                    pt->crosshair.r.x += ev.motion.xrel;
                    pt->crosshair.r.y += ev.motion.yrel;

                    pt->m_move = true;
                }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT && pt->w_focus)
                {
                    if (!player->swing)
                    {
                        player->vel -= player->gvel * 0.25f;
                        player->swing = true;
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
                            pt->JOY_use = true;

                            if (ev.jaxis.axis == 0)
                            {
                                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                                    pt->JOY_xdir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->JOY_xdir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                                else 
                                    pt->JOY_xdir = 0;
                            }
                            else if (ev.jaxis.axis == 1)
                            {
                                if (ev.jaxis.value < -JOYSTICK_DEADZONE) 
                                    pt->JOY_ydir = ((ev.jaxis.value + JOYSTICK_DEADZONE) / 24000.0f);
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->JOY_ydir = ((ev.jaxis.value - JOYSTICK_DEADZONE) / 24000.0f);
                                else 
                                    pt->JOY_ydir = 0;
                            }

                            // this is too much, but it works
                            pt->JOY_vel = (
                                pt->JOY_xdir < 0 ? -pt->JOY_xdir : pt->JOY_xdir * SDL_cos(player->INPUT_angle)) + (
                                pt->JOY_ydir < 0 ? -pt->JOY_ydir : pt->JOY_ydir * SDL_sin(player->INPUT_angle));

                            if (pt->JOY_vel > 0.75f) pt->JOY_vel = 1.0f;
                        break;
                        case 3:
                        case 4:
                            pt->m_move = true;
                            pt->JOY_use = true;

                            if (ev.jaxis.axis == 3)
                            {
                                pt->AIM_xvel = (ev.jaxis.value / 32767.0f);

                                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                                    pt->AIM_xdir = -1; 
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->AIM_xdir = 1;
                                else 
                                    pt->AIM_xdir = 0;
                            }
                            else if (ev.jaxis.axis == 4)
                            {
                                pt->AIM_yvel = (ev.jaxis.value / 32767.0f);

                                if (ev.jaxis.value < -JOYSTICK_DEADZONE)
                                    pt->AIM_ydir = -1;
                                else if (ev.jaxis.value > JOYSTICK_DEADZONE)
                                    pt->AIM_ydir = 1;
                                else 
                                    pt->AIM_ydir = 0;
                            }
                        break;
                        case 2:
                        break;
                        case 5:
                            if (ev.jaxis.value > -16000)
                            {
                                if (!player->swing && !pt->m_hold) 
                                {
                                    player->vel -= player->gvel * 0.25f;
                                    player->swing = true;
                                }

                                pt->m_hold = true;
                            } 
                            else pt->m_hold = false;
                        break;
                    }
                }
            break;
            case SDL_JOYBUTTONDOWN:
                if (ev.jbutton.which == 0)
                {
                    if (ev.jbutton.button == 0)
                    {
                        if (!player->sprint_cdown && !player->swing) 
                        {
                            player->sprint = true;
                            player->gvel = SPRINT_VELOCITY;
                            player->vel = SPRINT_VELOCITY;
                        }
                    }
                }
            break;
        }
    }
}

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

void updateBulletHits(B_HITS *hits, int bx, int by)
{
    hits->a[hits->index].x = bx;
    hits->a[hits->index].y = by;

    hits->a[hits->index].used = true;

    if (++hits->index > 29) hits->index = 0;
}

void updateTopDownShoot(P_TEST *pt, P *player)
{
    switch (pt->state)
    {
        case P_PLAY:
        break;
        case P_SCORE:
            if (++pt->SCORE_timer > 240)
            {
                player->x = (player->mx << pt->level.t_bit_size) + (pt->level.t_size >> 1);
                player->y = (player->my << pt->level.t_bit_size) + (pt->level.t_size >> 1);
                player->xvel = 0;
                player->yvel = 0;
                player->swing_timer = 0;
                player->swing = false;
                player->gvel = STANDARD_VELOCITY;
                player->vel = STANDARD_VELOCITY;
                player->sprint = false;
                player->sprint_timer = 0;
                player->sprint_cdown = false;
                player->sprint_cdown_timer = 0;

                pt->camera.x = player->x - (pt->camera.w >> 1);
                pt->camera.y = player->y - (pt->camera.h >> 1);

                pt->club_r.x = player->x - 8 + pt->GUN_radx;
                pt->club_r.y = player->y - 8 + pt->GUN_rady;

                pt->puck.x = player->x;
                pt->puck.y = player->y;
                pt->puck.r.x = pt->puck.x;
                pt->puck.r.y = pt->puck.y;
                pt->puck.xvel = 0;
                pt->puck.yvel = 0;
                pt->puck.fvel = 0.01f;
                pt->puck.hit = false;
                pt->puck.hit_counter = 0;

                pt->crosshair.r.x = pt->camera.w >> 1;
                pt->crosshair.r.y = pt->camera.h >> 1;
                pt->crosshair.show = false;

                pt->SCORE_timer = 0;

                pt->state = P_PLAY;
            }
        break;
    }

    if (player->sprint && (++player->sprint_timer > 60))
    {
        player->sprint_timer = 0;
        player->sprint = false;
        player->sprint_cdown = true;
        player->gvel = STANDARD_VELOCITY;
        player->vel = STANDARD_VELOCITY;
    }
    
    if (player->sprint_cdown && (++player->sprint_cdown_timer > 300))
    {
        player->sprint_cdown = false;
        player->sprint_cdown_timer = 0;
    }

    if (player->swing)
    {
        if (!pt->puck.hit)
        {
            if (checkCollision(pt->club_r, pt->puck.r))
            {
                pt->puck.xvel = 5 * SDL_cos(pt->GUN_angle);
                pt->puck.yvel = 5 * SDL_sin(pt->GUN_angle);
                pt->puck.hit = true;

                pt->PUCK_freeze = true;

                player->swing_timer = 0;
                player->vel = player->gvel;

                Mix_PlayChannel(-1, pt->mix_chunks[0], 0);
            }

            if (++player->swing_timer > 8)
            {
                player->swing = false;
                player->swing_timer = 0;
                player->vel = player->gvel;
            }
        }
        else if (pt->puck.hit)
        {
            if (++player->swing_timer > 16)
            {
                player->swing = false;
                player->swing_timer = 0;
                player->vel = player->gvel;
            }
        }
    }

    if (pt->PUCK_freeze && (++pt->PUCK_freeze_timer > 4))
    {
        pt->PUCK_freeze = false;
        pt->PUCK_freeze_timer = 0;
    }

    if (pt->puck.hit && (pt->puck.hit_counter++ > 16)) 
    {
        pt->puck.hit = false;
        pt->puck.hit_counter = 0;
    }

    if (pt->JOY_use)
    {
        if (pt->JOY_xdir == 0 && pt->JOY_ydir == 0)
        {
            player->INPUT_angle = 4;
            pt->JOY_vel = 0;
            pt->input_q[0] = 0;
        }
        else player->INPUT_angle = SDL_atan2(pt->JOY_ydir, pt->JOY_xdir);
        
        if ((pt->AIM_xdir || pt->AIM_ydir) && !player->swing)
        {
            pt->GUN_angle = SDL_atan2(pt->AIM_yvel, pt->AIM_xvel);

            pt->crosshair.r.x = player->x - pt->camera.x + (100 * SDL_cos(pt->GUN_angle));
            pt->crosshair.r.y = player->y - pt->camera.y + (100 * SDL_sin(pt->GUN_angle));
        }
        else 
        {
            pt->m_move = false;
            pt->AIM_done = true;
        }
    }
    else 
    {
        switch (*player->dir)
        {
            case KEY_LEFT:
                if (pt->input_q[1] == KEY_UP) 
                    player->INPUT_angle = I_UP_LEFT;
                else if (pt->input_q[1] == KEY_DOWN) 
                    player->INPUT_angle = I_DOWN_LEFT;
                else 
                    player->INPUT_angle = INPUT_LEFT;

                pt->JOY_vel = 1;
            break;
            case KEY_RIGHT:
                if (pt->input_q[1] == KEY_UP) 
                    player->INPUT_angle = I_UP_RIGHT;
                else if (pt->input_q[1] == KEY_DOWN) 
                    player->INPUT_angle = I_DOWN_RIGHT;
                else 
                    player->INPUT_angle = INPUT_RIGHT;

                pt->JOY_vel = 1;
            break;
            case KEY_UP:
                if (pt->input_q[1] == KEY_LEFT) 
                    player->INPUT_angle = I_UP_LEFT;
                else if (pt->input_q[1] == KEY_RIGHT) 
                    player->INPUT_angle = I_UP_RIGHT;
                else 
                    player->INPUT_angle = INPUT_UP;

                pt->JOY_vel = 1;
            break;
            case KEY_DOWN:
                if (pt->input_q[1] == KEY_LEFT) 
                    player->INPUT_angle = I_DOWN_LEFT;
                else if (pt->input_q[1] == KEY_RIGHT) 
                    player->INPUT_angle = I_DOWN_RIGHT;
                else 
                    player->INPUT_angle = INPUT_DOWN;

                pt->JOY_vel = 1;
            break;
            default:
                player->INPUT_angle = 4;
                pt->JOY_vel = 0;
            break;
        }

        if (pt->m_move && !player->swing)
        {
            pt->GUN_angle = (SDL_atan2(
                pt->camera.y + pt->crosshair.r.y - player->y, 
                pt->camera.x + pt->crosshair.r.x - player->x));
        }
    }

    float rx = 100 * SDL_cos(pt->GUN_angle), 
          ry = 100 * SDL_sin(pt->GUN_angle);

    // need to look over this 
    if (pt->camera.x + pt->crosshair.r.x - player->x < 0)
    {
        if (pt->camera.x + pt->crosshair.r.x - player->x < rx) 
            pt->crosshair.r.x = player->x - pt->camera.x + rx;
    }
    else if (pt->camera.x + pt->crosshair.r.x - player->x > 0)
    {
        if (pt->camera.x + pt->crosshair.r.x - player->x > rx)
            pt->crosshair.r.x = player->x - pt->camera.x + rx;
    }
    //
    if (pt->camera.y + pt->crosshair.r.y - player->y < 0)
    {
        if (pt->camera.y + pt->crosshair.r.y - player->y < ry) 
            pt->crosshair.r.y = player->y - pt->camera.y + ry;
    }
    else if (pt->camera.y + pt->crosshair.r.y - player->y > 0)
    {
        if (pt->camera.y + pt->crosshair.r.y - player->y > ry)
            pt->crosshair.r.y = player->y - pt->camera.y + ry;
    }
    //

    if (player->INPUT_angle != 4 && !player->swing)
    {
        float   cos = SDL_cos(player->INPUT_angle) * player->vel * pt->JOY_vel,
                sin = SDL_sin(player->INPUT_angle) * player->vel * pt->JOY_vel;

        if (!pt->puck.hit)
        {
            if (player->xvel > cos)
            {
                player->xvel -= 0.375f;
                if (player->xvel < cos) player->xvel = cos;
            }
            else if (player->xvel < cos)
            {
                player->xvel += 0.375f;
                if (player->xvel > cos) player->xvel = cos;
            }

            if (player->yvel > sin)
            {
                player->yvel -= 0.375f;
                if (player->yvel < sin) player->yvel = sin;
            }
            else if (player->yvel < sin)
            {
                player->yvel += 0.375f;
                if (player->yvel > sin) player->yvel = sin;
            }
        }
    }
    else
    {
        if (!pt->puck.hit)
        {
            if (player->xvel > 0)
            {
                player->xvel -= 0.15f;
                if (player->xvel < 0) player->xvel = 0;
            }
            else if (player->xvel < 0)
            {
                player->xvel += 0.15f;
                if (player->xvel > 0) player->xvel = 0;
            }
            
            if (player->yvel > 0)
            {
                player->yvel -= 0.15f;
                if (player->yvel < 0) player->yvel = 0;
            }
            else if (player->yvel < 0)
            {
                player->yvel += 0.15f;
                if (player->yvel > 0) player->yvel = 0;
            }
        }
    }

    if (!pt->puck.hit)
    {
        if (player->xvel)
        {
            float x = player->x + player->xvel;

            if (!checkPlayerPosition(
                (int)x >> pt->level.t_bit_size, 
                (int)player->y >> pt->level.t_bit_size, 
                pt->level.collision, 
                pt->level.t_map_h)
            )
            {
                if (!(x < 0 || x > pt->level.r.w)) 
                    player->x = x;
                else 
                    player->xvel = 0;
            }
            else player->xvel = 0;
        }

        if (player->yvel)
        {
            float y = player->y + player->yvel;

            if (!checkPlayerPosition(
                (int)player->x >> pt->level.t_bit_size, 
                (int)y >> pt->level.t_bit_size, 
                pt->level.collision, 
                pt->level.t_map_h)
            )
            {
                if (!(y < 0 || y > pt->level.r.h)) 
                    player->y = y;
                else 
                    player->yvel = 0;
            } 
            else player->yvel = 0;  
        }
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

            if (checkPlayerPosition(
                (int)px >> pt->level.t_bit_size, 
                (int)pt->puck.y >> pt->level.t_bit_size, 
                pt->level.collision, pt->level.t_map_h)
            )
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvel += 0.01f;
            }
            else if (px < 0 || px > pt->level.r.w) 
            {
                pt->puck.xvel = -pt->puck.xvel;
                pt->puck.fvel += 0.01f;
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
                pt->puck.fvel += 0.01f;
            }
            else if (py < 0 || py > pt->level.r.h) 
            {
                pt->puck.yvel = -pt->puck.yvel;
                pt->puck.fvel += 0.01f;
            }
            else pt->puck.y = py;
        }

        if (!pt->puck.xvel && !pt->puck.yvel)
            pt->puck.fvel = 0.01f;

        pt->puck.r.x = pt->puck.x;
        pt->puck.r.y = pt->puck.y;

        if (!pt->state == P_SCORE 
        && checkGoal(pt->puck.r, pt->goal_r))
        {
            pt->state = P_SCORE;
        }
    }

    if (pt->m_move)
    {
        pt->m_move = false;

        pt->AIM_timer = 0;
        pt->AIM_done = true;

        pt->crosshair.show = true;
    }

    if (pt->AIM_done && pt->AIM_timer++ > 240) 
    {
        pt->AIM_done = false;
        pt->AIM_timer = 0;

        pt->crosshair.r.x = pt->screen.w >> 1;
        pt->crosshair.r.y = pt->screen.h >> 1;
        pt->crosshair.show = false;
    }

    if (pt->crosshair.show)
    {
        player->rx = player->x + (pt->crosshair.r.x - (pt->camera.w >> 1));
        player->ry = player->y + (pt->crosshair.r.y - (pt->camera.h >> 1));
    }
    else 
    {
        player->rx = player->x;
        player->ry = player->y;

        if (player->INPUT_angle != 4)
        {
            if (!player->swing) pt->GUN_angle = player->INPUT_angle;
        }
    }

    pt->GUN_radx = 10 * SDL_cos(pt->GUN_angle);
    pt->GUN_rady = 10 * SDL_sin(pt->GUN_angle);

    pt->club_r.x = player->x - 8 + pt->GUN_radx;
    pt->club_r.y = player->y - 8 + pt->GUN_rady;

    pt->GUN_deg = (pt->GUN_angle * 180) / M_PI;

    if (pt->GUN_deg < 0) pt->GUN_deg += 360;

    if (pt->GUN_deg > AIM_RIGHT || pt->GUN_deg < AIM_DOWN)
    {
        player->facing = 2; // right
        if (pt->JOY_use && pt->JOY_vel) pt->input_q[0] = KEY_RIGHT;
    }
    else if (pt->GUN_deg > AIM_DOWN && pt->GUN_deg < AIM_LEFT)
    {
        player->facing = 0; // down
        if (pt->JOY_use && pt->JOY_vel) pt->input_q[0] = KEY_DOWN;
    }
    else if (pt->GUN_deg > AIM_LEFT && pt->GUN_deg < AIM_UP)
    {
        player->facing = 3; // left
        if (pt->JOY_use && pt->JOY_vel) pt->input_q[0] = KEY_LEFT;
    }
    else if (pt->GUN_deg > AIM_UP && pt->GUN_deg < AIM_RIGHT)
    {
        player->facing = 1; // up
        if (pt->JOY_use && pt->JOY_vel) pt->input_q[0] = KEY_UP;
    }

    // shot bullets
    /*
    for (int i = 0; i < 10; i++)
    {
        if (player->bullets[i].shoot)
        {
            player->bullets[i].x += player->bullets[i].xvel;
            player->bullets[i].y += player->bullets[i].yvel;

            player->bullets[i].r.x = player->bullets[i].x - pt->camera.x;
            player->bullets[i].r.y = player->bullets[i].y - pt->camera.y;

            if (player->bullets[i].x > pt->level.r.w 
            || player->bullets[i].x < 0) 
                player->bullets[i].shoot = false;
            else if (player->bullets[i].y > pt->level.r.h 
            || player->bullets[i].y < 0) 
                player->bullets[i].shoot = false;
            else 
            {
                // the naming is wonky but it does the job, checking bullet collision
                // this could be dependent on speed and tile/target size
                if (checkPlayerPosition(
                    (int)player->bullets[i].x >> pt->level.t_bit_size, 
                    (int)player->bullets[i].y >> pt->level.t_bit_size, 
                    pt->level.collision, pt->level.t_map_h)
                )
                {
                    // first check

                    updateBulletHits(
                        &pt->bullet_hits, 
                        (int)player->bullets[i].x, 
                        (int)player->bullets[i].y);
                    
                    player->bullets[i].shoot = false;
                    continue;
                }
                // need to check if we already passed a target/tile
                else if (
                    checkPlayerPosition(
                        ((int)(
                            player->bullets[i].x - (player->bullets[i].xvel / 2)) 
                            >> pt->level.t_bit_size),
                        ((int)(
                            player->bullets[i].y - (player->bullets[i].yvel / 2)) 
                            >> pt->level.t_bit_size),
                        pt->level.collision, pt->level.t_map_h
                    )
                )
                {
                    // second check

                    updateBulletHits(
                        &pt->bullet_hits, 
                        (int)player->bullets[i].x, 
                        (int)player->bullets[i].y);

                    player->bullets[i].shoot = false;
                    continue;
                }
                else
                {
                    
                }
            }
        }
    }
    */

    animatePlayer(player);

    setCamera(
        &pt->camera, 
        lerp(
            pt->camera.x + (pt->camera.w >> 1), 
            player->rx, 0.08f),
        lerp(
            pt->camera.y + (pt->camera.h >> 1), 
            player->ry, 0.08f)
    );
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

    if (p->facing == 3) 
        SDL_RenderCopyEx(r, t, &p->t_clips[p->c_index], &renderQuad, 0, NULL, SDL_FLIP_HORIZONTAL);
    else 
        SDL_RenderCopy(r, t, &p->t_clips[p->c_index], &renderQuad);

    // for sprite flipping
    //SDL_RenderCopyEx(r, t, &p->t_clips[p->a_index], &renderQuad, 0, NULL, SDL_FLIP_NONE);
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
    for (int i = 0; i < 4; i++)
        pt->input_q[i] = 0;

    player->xvel = 0;
    player->yvel = 0;

    player->x = (player->mx << pt->level.t_bit_size) + (pt->level.t_size >> 1);
    player->y = (player->my << pt->level.t_bit_size) + (pt->level.t_size >> 1);

    pt->camera.x = player->x - (pt->camera.w >> 1);
    pt->camera.y = player->y - (pt->camera.h >> 1);
    
    pt->state = P_PLAY;

    pt->club_r.x = player->x - 8 + pt->GUN_radx;
    pt->club_r.y = player->y - 8 + pt->GUN_rady;

    pt->puck.x = player->x;
    pt->puck.y = player->y;

    pt->puck.r.x = pt->puck.x;
    pt->puck.r.y = pt->puck.y;
    pt->puck.r.w = 8;
    pt->puck.r.h = 4;

    pt->puck.xvel = 0;
    pt->puck.yvel = 0;
    pt->puck.hit = false;
    pt->puck.hit_counter = 0;

    player->shoot = false;
    player->gun_timer = 0;

    pt->crosshair.r.x = pt->camera.w >> 1;
    pt->crosshair.r.y = pt->camera.h >> 1;

    for (int i = 0; i < 10; i++)
        player->bullets[i].shoot = false;
    
}

void resetPlay(P_TEST *pt, P *player)
{
    pt->camera.x = 0;
    pt->camera.y = 0;

    player->xvel = 0;
    player->yvel = 0;

    player->state = FALLING;

    for (int i = 0; i < 4; i++)
        pt->input_q[i] = 0;
}
