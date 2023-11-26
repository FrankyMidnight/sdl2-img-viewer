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
#include <string.h>
#include <threads.h>

int WIN_WIDTH = 800;
int WIN_HEIGHT = 600;
const int MAX_CHARS = 256;

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Rect dest_rect = {};
int running = 1;
DIR *directory = NULL;
char *path = ".";
int Init();
void Quit();
void Draw();
void Update();
void load_dir();
void update_windowsize();
size_t total_files = {0};
char *file_list;
size_t current_idx = 0;
size_t file_idx = 0;
char *file_head;

int main(void)
{
    Init();
    load_dir();
    Draw();
    Update();
    return 0;
}

int Init()
{
    // SDL INIT
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Impossibile inizializzare SDL2 : %s", SDL_GetError());
        return 1;
    }
    // WINDOW
    window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        SDL_Log("Impossibile creare finestra : %s", SDL_GetError());
        return 2;
    }
    // IMG INIT
    int img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP;
    int check_flags = IMG_Init(img_flags);
    if (check_flags != img_flags)
    {
        SDL_Log("IMG init failure : %s", IMG_GetError());
        return 3;
    }
    // RENDERER
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_Log("Errore di inizializzazione del renderer : %s", SDL_GetError());
        return 4;
    }
    return 0;
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
    free(file_list);
}
void Draw()
{
    // SURFACE
    surface = IMG_Load(file_list);
    if (surface == NULL)
    {
        SDL_Log("Impossibile caricare immagine : %s", IMG_GetError());
        return;
    }
    // TEXTURE
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL)
    {
        SDL_Log("Impossibile creare Texture : %s", SDL_GetError());
        return;
    }

    // SRC RECT
    int surface_width = surface->w;
    int surface_height = surface->h;
    SDL_Log("surface_width : %5d\tsurface_height : %5d", surface_width, surface_height);
    // ratio = w / h
    float src_ratio = (float)surface_width / surface_height;
    SDL_Log("src_ratio : %2.f", src_ratio);

    // DEST RECT
    // LEGGO SIZE ATTUALE DELLA WINDOW
    SDL_GetWindowSize(window, &WIN_WIDTH, &WIN_HEIGHT);
    SDL_Log("WINDOW SIZE w : %5d\th : %5d",WIN_WIDTH, WIN_HEIGHT);
    // IMPOSTO WIDTH DEL RENDERING IMMAGINE A WIN WIDTH
    int dest_width = WIN_WIDTH;
    // CALCOLO DEL RENDERING IMAGINE HEIGHT = WIDTH / RATIO
    int dest_height = WIN_WIDTH / src_ratio;
    // in questo caso regolo y_offset
    int y_offset = WIN_HEIGHT / 2 - dest_height / 2;
    int x_offset = 0;
    // CHECK SE DEST HEIGHT > WIN HEIGHT
    if (dest_height > WIN_HEIGHT)
    {
        dest_height = WIN_HEIGHT;
        dest_width = WIN_HEIGHT * src_ratio;
        x_offset = WIN_WIDTH / 2 - dest_width / 2;
        y_offset = 0;
    }
    dest_rect = (SDL_Rect){.w = dest_width, .h = dest_height, .x = x_offset, .y = y_offset};
    SDL_Log("dest_rec w : %5d\th : %5d\tx_offset : %5d\ty_offset : %5d ",dest_width, dest_height, x_offset, y_offset);
    // FREE SURFACE
    SDL_FreeSurface(surface);
}
void Update()
{
    // LOOP
    SDL_Event event;

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
                running = 0;
                break;
            
                // ESCI SU PRESSIONE TASTO ESCAPE
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                case SDLK_q:
                    running = 0;
                    break;
                // case SDLK_l:
                case SDLK_RIGHT:
                    if (file_idx > total_files - 1)
                    {
                        file_idx = 0;
                        file_list = file_head;
                        printf("FILE_IDX : %5zu\tFILE : %s\n ", file_idx, file_list);
                    }
                    else
                    {
                        file_idx++;
                        file_list += MAX_CHARS;
                        printf("FILE_IDX : %5zu\tFILE : %s\n ", file_idx, file_list);
                    }

                    SDL_Log("Premuto tasto l");
                    Draw();
                    break;
                // case SDLK_h:
                case SDLK_LEFT:
                    if (file_idx == 0)
                    {
                        file_list = file_head + (total_files * MAX_CHARS) - MAX_CHARS;
                        file_idx = total_files - 1;
                        printf("FILE_IDX : %5zuFILE : %s\n ", file_idx, file_list);
                    }
                    else
                    {
                        file_idx--;
                        file_list -= MAX_CHARS;
                        printf("FILE_IDX : %5zuFILE : %s\n ", file_idx, file_list);
                    }
                    SDL_Log("Premuto tasto h");
                    Draw();
                    break;
                }
                break;
            }
            switch (event.window.event)
            {
                case SDL_WINDOWEVENT_RESIZED:
                update_windowsize();
                break;
            }
        }
    }
}

void load_dir()
{
    // APRO DIRECTORY
    directory = opendir(path);
    // ESCO SE IMPOSSIBILE APRIRE DIR
    if (directory == NULL)
    {
        SDL_Log("Impossibile accedere a \".\"");
        Quit();
    }
    // LEGGO SIZE DEI FILES IN DIR
    while (readdir(directory))
    {
        total_files++;
    }
    closedir(directory);
    directory = opendir(path);
    printf("Trovati %zu files\n", total_files);
    // PREPARO L'ARRAY DI 256 CHAR per file
    file_list = calloc(total_files, MAX_CHARS);
    file_head = file_list;
    // RIEMPIO LA LISTA DEI FILES IN DIR
    // struct dirent *entry;
    // SALVO INIZIO LISTA
    if (file_list == NULL)
    {
        SDL_Log("ERRORE ALLOCAZIONE MEMORIA\n");
        Quit();
    }
    // salvo testa della lista
    char *temp_file_list = file_list;
    // RIEMPIO LA LISTA
    for (size_t i = 0; i < total_files; i++)
    {
        strcpy(file_list, readdir(directory)->d_name);
        // strlcpy(file_list, dir_item,strlen(file_list));
        puts(file_list);
        file_list += MAX_CHARS;
    }
    // ripristino testa della lista
    file_list = temp_file_list;
    // salvo testa della lista
    temp_file_list = file_list;
    // STAMPO LISTA FILE TROVATI
    for (size_t i = 0; i < total_files; i++)
    {
        printf("IDX : %10zu\tfile : %s\n", i, file_list);
        file_list += MAX_CHARS;
    }
    // ripristino testa della lista
    file_list = temp_file_list;
    closedir(directory);
}

void update_windowsize()
{
    int w = 0;
    int h = 0;;
    SDL_GetWindowSizeInPixels(window, &w , &h );
    SDL_Log("WIndow resized , new w : %5d\tnew h : %5d",w,h);
    WIN_WIDTH = w;
    WIN_HEIGHT = h;
    Draw();
}