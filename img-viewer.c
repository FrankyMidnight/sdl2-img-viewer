#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define MAX_CHARS 256

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Rect dest_rect = {};

char *file_head = NULL;
char *file_list = NULL;
;
char *file_current = NULL;
size_t files_count = 0;
size_t file_idx = 0;

void Init();
void Quit();
void Draw();
void Update();
void load_dir();

int main(void)
{
    Init();
    load_dir();
    Draw();
    Update();
    return 0;
}

void Init()
{
    // SDL INIT
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Impossibile inizializzare SDL2 : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // WINDOW
    window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT,
                              SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        SDL_Log("Impossibile creare finestra : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // IMG INIT
    int img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP;
    int check_flags = IMG_Init(img_flags);
    if (check_flags != img_flags)
    {
        SDL_Log("IMG init failure : %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    // RENDERER
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_Log("Errore di inizializzazione del renderer : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}
void Quit()
{
    // CLEANUP AND FINISH
    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    // closedir(directory);
    free(file_list);
    exit(EXIT_SUCCESS);
}
void Draw()
{
    // TENTO SURFACE SUL PRIMO FILE IN LISTA
    surface = IMG_Load(file_current);
    if (surface == NULL)
    {
        // SDL_Log("Impossibile caricare immagine : %s\t errore : %s",file_current, IMG_GetError());
        return;
    }

    puts("tento texture");
    // TEXTURE
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL)
    {
        // SDL_Log("Impossibile creare Texture : %s", SDL_GetError());
        return;
    }

    // SRC RECT
    int surface_width = surface->w;
    int surface_height = surface->h;
    SDL_Log("surf width : %d\t surf height : %d", surface_width, surface_height);
    // ratio = w / h
    float src_ratio = (float)surface_width / surface_height;
    SDL_Log("src ratio : %.2f", src_ratio);
    // SDL_Rect src_rect = {.w = surface_width, .h = surface_height, .x = 0, .y = 0};

    // DEST RECT
    int y_offset = 0;
    int x_offset = 0;
    // SETTO HEIGHT IMMAGINE A HEIGHT FINESTRA
    int render_width = WIN_WIDTH;
    SDL_Log("render_width : %d", render_width);
    // CALC HEIGHT = WIDTH / RATIO
    int render_height = WIN_WIDTH / src_ratio;
    SDL_Log("render_height : %d", render_height);
    // SE SETTO WIDTH IMG A WIN WIDTH REGOLO Y OFFSET E LASCIO X OFFSET A 0
    y_offset = WIN_HEIGHT / 2 - render_height / 2;
    x_offset = 0;
    // SE NUOVA HEIGHT > WIN HEIGHT
    if (render_height > WIN_HEIGHT)
    {
        // SETTO IMG HEIGHT A HEIGHT FINESTRA
        render_height = WIN_HEIGHT;
        // CALC WIDTH = HEIGHT * RATIO
        render_width = WIN_HEIGHT * src_ratio;
        // SE SETTO HEIGHT IMG A WIN HEIGHT SETTO Y OFFSET A 0 E REGOLO X OFFSET
        y_offset = 0;
        x_offset = WIN_WIDTH / 2 - render_width / 2;
    }
    SDL_Log("Normalized WIDTH : %d\tHEIGHT : %d", render_width, render_height);
    // IMPOSTO DEST RECT
    dest_rect = (SDL_Rect){.w = render_width, .h = render_height, .x = x_offset, .y = y_offset};
    SDL_Log("dest_rect w : %d\th : %d\tx : %d\ty : %d", dest_rect.w, dest_rect.h, dest_rect.x, dest_rect.y);
    // FREE SURFACE
    SDL_FreeSurface(surface);
    puts("liberata surface");
}
void Update()
{
    // LOOP
    SDL_Event event;
    bool running = true;
    while (running)
    {
        // RENDER !!!
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_RenderPresent(renderer);

        // SVUOTA QUEUE EVENTI
        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            // ESCI SU CLICK CHIUSURA FINESTRA
            case SDL_QUIT:
                running = false;
                break;
                // ESCI SU PRESSIONE TASTO ESCAPE
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    // ESCO ANCHE CON ESCAPE E Q
                case SDLK_ESCAPE:
                case SDLK_q:
                    running = false;
                    break;
                case SDLK_l:
                    // SDL_Log("Premuto tasto l");
                    // SE INDICE È MINORE DEL TOTALE
                    if (file_idx < files_count - 1)
                    {
                        // AVANZO IN LISTA E INCREMENTO INDICE
                        file_current += MAX_CHARS;
                        file_idx++;
                        SDL_Log("file_idx : %zu\t file_current : %s", file_idx, file_current);
                    }
                    // SE INDICE È A FINE LISTA
                    else
                    {
                        // SETTO INDICE E FILE CORRENTE A 0
                        file_current = file_head;
                        file_idx = 0;
                        SDL_Log("file_idx : %zu\t file_current : %s", file_idx, file_current);
                    }
                    Draw();
                    break;
                case SDLK_h:
                    // SDL_Log("Premuto tasto h");
                    // SE INDICE MAGGIORE DI 0
                    if (file_idx > 0)
                    {
                        // CALO INDICE E RETROCEDO IL PUNTATORE
                        file_current -= MAX_CHARS;
                        file_idx--;
                        SDL_Log("file_idx : %zu\t file_current : %s", file_idx, file_current);
                    }
                    // SE INDICE È 0 SETTO INDICE E PUNTATORE A FINE LISTA ()
                    else
                    {
                        file_current += files_count * MAX_CHARS - MAX_CHARS;
                        file_idx = files_count - 1;
                        SDL_Log("file_idx : %zu\t file_current : %s", file_idx, file_current);
                    }
                    Draw();
                    break;
                }
                break;
            }
        }
    }
}

void load_dir()
{
    DIR *directory = NULL;
    char *path = ".";
    // OPEN CURRENT WORKING DIRECTORY
    directory = opendir(path);
    if (directory == NULL)
    {
        SDL_Log("Impossibile accedere a \".\"");
        Quit();
    }
    // CONTO I FILE PER PREPARARE SIZE DELLA LISTA DEI FILES
    while (readdir(directory))
    {
        files_count++;
    }
    // CHIUDO DIRECTORY (ORA È IMPOSSIBILE LEGGERE OLTRE LA FINE)
    closedir(directory);
    // ALLOCO MEMORIA PER LISTA FILE
    file_list = calloc(files_count, MAX_CHARS);
    file_head = file_list;
    // RIAPRO LA DIRECTORY
    directory = opendir(path);
    if (directory == NULL)
    {
        SDL_Log("Impossibile accedere a \".\"");
        Quit();
    }
    // RIEMPIO LISTA FILES
    for (size_t i = 0; i < files_count; i++)
    {
        strcpy(file_list, readdir(directory)->d_name);
        file_list += MAX_CHARS;
    }
    // CHIUDO DIRECTOORY DEFINITIVAMENTE
    closedir(directory);
    // RIMETTO A CAPO IL PUNTATORE
    file_list = file_head;
    // STAMPO PER CONFERMA
    for (size_t i = 0; i < files_count; i++)
    {
        SDL_Log("IDX : %5zu\tFILE : %s", i, file_list);
        file_list += MAX_CHARS;
    }
    // RIMETTO A CAPO IL PUNTATORE
    file_list = file_head;
    // PNTO IL CURRENT A TESTA LISTA
    file_current = file_head;
    SDL_Log("out of load_dir");
    SDL_Log("file_current : %s", file_current);
}
