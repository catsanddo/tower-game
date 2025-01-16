#include "render.h"
#include "script.h"
#include "config.h"
#include "common.h"
#include "log.h"
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define Q(s) if ((s) < 0) { \
    LogWarning("%s(%d): %s", __FILE__, __LINE__, SDL_GetError()); \
}

static SDL_Rect CameraTransform(Renderer *renderer, Coord co)
{
    Camera cam = renderer->camera;

    if (renderer->camera_disable) {
        return (SDL_Rect) {
            co.position.x + co.offset.x,
            co.position.y + co.offset.y,
            TILE, TILE,
        };
    }
    
    SDL_Rect result;
    result.w = TILE;
    result.h = TILE;
    
    int basex = WIDTH / 2 - TILE / 2;
    int basey = HEIGHT / 2 - TILE / 2;
    result.x = basex + (co.position.x - cam.co.position.x) * TILE - cam.co.offset.x + co.offset.x;
    result.y = basey + (co.position.y - cam.co.position.y) * TILE - cam.co.offset.y + co.offset.y;

    if (!renderer->camera_lock) {
        return result;
    }

    // NOTE: kind of feels like a hack, but it works
    // got even more hacky, but still works
    if (cam.co.position.x * TILE + cam.co.offset.x < 5 * TILE - TILE / 2) {
        result.x = co.position.x * TILE + co.offset.x;
    } else if (cam.co.position.x * TILE + cam.co.offset.x + 5 * TILE >= renderer->map.map_width * TILE - TILE / 2) {
        result.x = co.position.x * TILE + co.offset.x;
        result.x -= renderer->map.map_width * TILE - WIDTH;
    }
    if (cam.co.position.y * TILE + cam.co.offset.y < 4 * TILE) {
        result.y = co.position.y * TILE + co.offset.y;
    } else if (cam.co.position.y * TILE + cam.co.offset.y + 4 * TILE >= renderer->map.map_height * TILE - TILE) {
        result.y = co.position.y * TILE + co.offset.y;
        result.y -= renderer->map.map_height * TILE - HEIGHT;
    }

    return result;
}

static void SetDrawColor(Renderer *renderer, SDL_Color color)
{
    Q(SDL_SetRenderDrawColor(renderer->renderer, color.r, color.g, color.b, color.a));
}

static void MapDraw(Renderer *r)
{
    RenderMap *map = &r->map;
    Camera *cam = &r->camera;

    if (!map->tilesheet && !map->tile_map) {
        LogWarning("Drawing unloaded map!");
    }

    for (CE_u16 y = 0; y < map->map_height; ++y) {
        for (CE_u16 x = 0; x < map->map_width; ++x) {
            CE_u32 tile_index = map->tile_map[y * map->map_width + x];
            if (tile_index == 0) {
                continue;
            }
            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (tile_index & TILE_FLIP) {
                flip |= SDL_FLIP_HORIZONTAL;
                tile_index &= ~TILE_FLIP;
            }
            SDL_Rect dest = CameraTransform(r, (Coord) {{x, y}, {0}});

            SDL_Rect screen = {0, 0, WIDTH, HEIGHT};
            if (SDL_HasIntersection(&screen, &dest)) {
                SDL_Rect src;
                src.w = src.h = TILE;
                src.x = (tile_index - 1) % map->tiles_width * TILE;
                src.y = (tile_index - 1) / map->tiles_width * TILE;

                Q(SDL_RenderCopyEx(r->renderer, map->tilesheet, &src, &dest,
                        0, 0, flip));
            }
        }
    }
}

static Sprite *SpriteGet(Renderer *renderer, SpriteHandle id)
{
    if (id.handle < 0 || id.handle >= MAX_ENTITIES) {
        return 0;
    }
    
    Sprite *result = &renderer->sprite_pool[id.handle];

    // null texture is flag for inactive sprite
    if (result->texture == 0) {
        result = 0;
    }

    return result;
}

static void SpritesDraw(Renderer *renderer) {
    for (CE_u64 i = renderer->sprite_queue.start; i < renderer->sprite_queue.end; ++i) {
        Sprite *sprite = SpriteGet(renderer, renderer->sprite_queue.queue[i]);
        if (sprite == 0) {
            LogWarning("Rendering invalid sprite (%llu)", renderer->sprite_queue.queue[i].handle);
            continue;
        }
        SDL_Rect src = {
            sprite->frame * TILE, sprite->animation * TILE,
            TILE, TILE,
        };
        SDL_Rect dest = CameraTransform(renderer, sprite->co);
        Q(SDL_RenderCopy(renderer->renderer, sprite->texture, &src, &dest));

        if (renderer->animations_disable) {
            continue;
        }
        if (sprite->is_playing && sprite->current_delay >= sprite->frame_delay) {
            sprite->current_delay = 0;
            sprite->frame += 1;
            if (sprite->frame >= sprite->frame_max) {
                sprite->frame = 0;
            }
        } else if (sprite->is_playing) {
            sprite->current_delay += 1000 / MAX_FPS;
        }
    }
}

static SDL_Surface *RenderText(char *text, SDL_Color fg, SDL_Color bg)
{
    CE_u64 length = strlen(text);
    int w = length * 7 + 1;
    int h = 8;
    int letter_x = 1;

    SDL_Surface *result = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
    CE_u32 *buffer = result->pixels;
    CE_u32 fg_color = SDL_MapRGB(result->format, fg.r, fg.g, fg.b);
    CE_u32 bg_color = SDL_MapRGB(result->format, bg.r, bg.g, bg.b);
    SDL_FillRect(result, 0, bg_color);

    while (*text != 0) {
        CE_u8 *byte = &font[*text * 8];
        int flag = 0;
        if (*text == 'o') {
            flag = 1;
        }
        for (int y = 0; y < 8; ++y) {
            int head = 0x80;
            for (int x = 0; x < 8; ++x) {
                CE_u32 color;
                if (*byte & head) {
                    color = *(CE_u32 *) &fg_color;
                } else {
                    color = *(CE_u32 *) &bg_color;
                }
                int i = y * w + letter_x + x;
                buffer[i] = color;
                head = head >> 1;
            }
            byte += 1;
        }
        letter_x += 7;
        text += 1;
    }

    return result;
}

static void TextBoxDraw(Renderer *renderer)
{
    SDL_Surface *row;
    // TODO: change these magic values later
    SDL_Surface *body = SDL_CreateRGBSurface(0, 136, 34, 32, 0, 0, 0, 0);
    int rows_count = renderer->text_box.line+1;
    if (rows_count >= TBOX_ROWS) {
        // Last row is a buffer and should be hidden
        rows_count = TBOX_ROWS - 1;
    }
    
    if (!body) {
        LogError("Failed to allocate text box surface: %s", SDL_GetError());
        return;
    }
    CE_u32 base_color = SDL_MapRGBA(body->format, renderer->base.r,
            renderer->base.g, renderer->base.b, renderer->base.a);
    Q(SDL_FillRect(body, 0, base_color));

    int line_height = TTF_FontLineSkip(renderer->font) + 1;
    for (int i = 0; i < rows_count; ++i) {
        char *text = renderer->text_box.rows[i];
        // empty string
        if (text[0] == 0) {
            continue;
        }
        // row = TTF_RenderText_Solid(renderer->font, text, renderer->accent);
        row = RenderText(text, renderer->accent, renderer->base);
        if (!row) {
            LogWarning("Unable to render text '%s': %s", text, TTF_GetError());
            continue;
        }
        SDL_Rect dest = {0, line_height*i, row->w, row->h};
        Q(SDL_BlitSurface(row, 0, body, &dest));
        SDL_FreeSurface(row);
    }
    
    SetDrawColor(renderer, renderer->base);
    Q(SDL_RenderFillRect(renderer->renderer, &renderer->text_box.box));
    SetDrawColor(renderer, renderer->accent);
    Q(SDL_RenderDrawRect(renderer->renderer, &renderer->text_box.box));
    
    SDL_Texture *tmp_texture = SDL_CreateTextureFromSurface(renderer->renderer, body);
    SDL_FreeSurface(body);
    if (!tmp_texture) {
        LogError("Failed to allocate text box texture: %s", SDL_GetError());
        return;
    }

    SDL_Rect src;
    SDL_Rect dest;
    SDL_Rect tmp = renderer->text_box.box;
    tmp.x += 2;
    tmp.y += 2;
    tmp.w -= 4;
    tmp.h -= 4;
    src = tmp;
    tmp.y -= renderer->text_box.scroll;
    SDL_IntersectRect(&src, &tmp, &dest);
    src.h = dest.h;
    src.x = 0;
    src.y = renderer->text_box.scroll;
    
    Q(SDL_RenderCopy(renderer->renderer, tmp_texture, &src, &dest));

    if (TextBoxIsScrolling(renderer)) {
        renderer->text_box.scroll += 1;
        if (renderer->text_box.scroll >= line_height) {
            renderer->text_box.state = TBOX_STATIC;
            renderer->text_box.scroll = 0;
            renderer->text_box.line -= 1;
            for (int i = 1; i < TBOX_ROWS; ++i) {
                memcpy(renderer->text_box.rows[i-1], renderer->text_box.rows[i], TBOX_LIMIT);
            }
            renderer->text_box.rows[TBOX_ROWS-1][0] = 0;
        }
    }
}

static void PromptRender(Renderer *renderer)
{
    SDL_Surface *text_surface;
    SDL_Texture *text;
    SDL_Rect text_box;

    int base_y = 48;
    if (renderer->text_box.box.y == 100) {
        base_y += 28;
    }

    // render the box
    text_box.x = 116;
    text_box.y = base_y;//76;
    text_box.w = 34;
    text_box.h = 24;
    SetDrawColor(renderer, renderer->base);
    Q(SDL_RenderFillRect(renderer->renderer, &text_box));
    SetDrawColor(renderer, renderer->accent);
    Q(SDL_RenderDrawRect(renderer->renderer, &text_box));

    // render marker
    text_box.x = 120;
    // text_box.y = 82;
    text_box.y = base_y+6;
    text_box.w = 3;
    text_box.h = 3;
    if (renderer->prompt_state == PROMPT_NO) {
        text_box.y += 9;
    }
    Q(SDL_RenderDrawRect(renderer->renderer, &text_box));

    // render text options
    text_surface = TTF_RenderText_Solid(renderer->font, "Yes", renderer->accent);
    if (!text_surface) {
        LogWarning("Unable to render text 'Yes': %s", TTF_GetError());
    }
    text_box.w = text_surface->w;
    text_box.h = text_surface->h;
    text = SDL_CreateTextureFromSurface(renderer->renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (!text) {
        LogWarning("Unable to make 'Yes' texture: %s", SDL_GetError());
    }

    text_box.x = 125;
    // text_box.y = 80;
    text_box.y = base_y+4;
    Q(SDL_RenderCopy(renderer->renderer, text, 0, &text_box));
    
    text_surface = TTF_RenderText_Solid(renderer->font, "No", renderer->accent);
    if (!text_surface) {
        LogWarning("Unable to render text 'No': %s", text, TTF_GetError());
    }
    text_box.w = text_surface->w;
    text_box.h = text_surface->h;
    text = SDL_CreateTextureFromSurface(renderer->renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (!text) {
        LogWarning("Unable to make 'No' texture: %s", SDL_GetError());
    }

    text_box.x = 126;
    // text_box.y = 89;
    text_box.y = base_y+13;
    Q(SDL_RenderCopy(renderer->renderer, text, 0, &text_box));
}

int RendererInit(Renderer *renderer, SDL_Window *window)
{
    *renderer = (Renderer) {0};

    CE_u32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    renderer->renderer = SDL_CreateRenderer(window, -1, flags);

    if (!renderer->renderer) {
        AlertError("SDL Error", "Creating renderer: %s", SDL_GetError());
        return 0;
    }

    SDL_RendererInfo info;
    Q(SDL_GetRendererInfo(renderer->renderer, &info));
    if (info.flags != flags) {
        AlertError("SDL Error", "Renderer does not support requested features (%u)", info.flags);
        return 0;
    }
    LogMsg("Renderer: %s", info.name);
    LogMsg("Flags: %u", info.flags);
    LogMsg("Texture Width: %d", info.max_texture_width);
    LogMsg("Texture Height: %d", info.max_texture_height);

    // TODO: select pixel format from RendererInfo?
    renderer->canvas = SDL_CreateTexture(renderer->renderer,
            SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_TARGET,
            WIDTH, HEIGHT);

    if (!renderer->canvas) {
        AlertError("SDL Error", "Creating canvas: %s", SDL_GetError());
        return 0;
    }
    
    renderer->font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (!renderer->font) {
        AlertError("Resource Error", "Could not open '%s': %s", FONT_PATH, TTF_GetError());
        return 0;
    }

    renderer->camera_lock = 1;

    return 1;
}

void RendererDeinit(Renderer *renderer)
{
    SDL_DestroyTexture(renderer->canvas);
    SDL_DestroyRenderer(renderer->renderer);

    TTF_CloseFont(renderer->font);

    *renderer = (Renderer) {0};
}

void RendererUpdate(Renderer *renderer)
{
    // Animation?
    // I actually don't know if this function makes any sense
}

void RendererDraw(Renderer *renderer)
{
    SetDrawColor(renderer, renderer->base);
    Q(SDL_RenderClear(renderer->renderer));
    Q(SDL_SetRenderTarget(renderer->renderer, renderer->canvas));
    Q(SDL_RenderClear(renderer->renderer));

    if (!renderer->map_disable) {
        MapDraw(renderer);
    }

    // Sprite rendering
    if (!renderer->sprite_disable) {
        SpritesDraw(renderer);
    }

    // Reset sprite queue
    // NOTE: super important to flush this every cycle
    //       because of the immediate mode nature of the queueing calls
    renderer->sprite_queue.start = 0;
    renderer->sprite_queue.end = 0;
    
    // Debug crosshair
    // SDL_Rect center = {
    //     WIDTH / 2 - TILE / 2, HEIGHT / 2 - TILE / 2,
    //     TILE, TILE,
    // };
    // Q(SDL_SetRenderDrawColor(renderer->renderer, 0xff, 0x80, 0, 0x80));
    // Q(SDL_RenderDrawRect(renderer->renderer, &center));
    // Q(SDL_RenderDrawPoint(renderer->renderer, WIDTH / 2, HEIGHT / 2));

    // Curtain layer
    if (renderer->curtain.height > 0) {
        SDL_Rect area = {-TILE / 2, 0, TILE, renderer->curtain.height};
        SetDrawColor(renderer, renderer->accent);
        for (; area.x < WIDTH; area.x += TILE) {
            if (area.y == 0) {
                area.y = HEIGHT - renderer->curtain.height;
            } else {
                area.y = 0;
            }
            Q(SDL_RenderFillRect(renderer->renderer, &area));
        }
    }
    CE_i32 direction = Sign(renderer->curtain.target_height - renderer->curtain.height);
    renderer->curtain.height += direction * 4;

    if (!renderer->tbox_disable && TextBoxIsOpen(renderer)) {
        TextBoxDraw(renderer);
        if (renderer->prompt_state) {
            PromptRender(renderer);
            renderer->prompt_state = 0;
        }
    }

    if (renderer->hook) {
        renderer->hook(renderer, renderer->hook_data);
    }

    Q(SDL_SetRenderTarget(renderer->renderer, 0));
    Q(SDL_RenderCopy(renderer->renderer, renderer->canvas, 0, 0));
    SDL_RenderPresent(renderer->renderer);
}

void RendererSetHook(Renderer *renderer, RenderHookFn func, void *data)
{
    renderer->hook = func;
    renderer->hook_data = data;
}

void RendererEnableMap(Renderer *renderer)
{
    renderer->map_disable = 0;
}

void RendererEnableSprite(Renderer *renderer)
{
    renderer->sprite_disable = 0;
}

void RendererEnableTextBox(Renderer *renderer)
{
    renderer->tbox_disable = 0;
}

void RendererEnableCamera(Renderer *renderer)
{
    renderer->camera_disable = 0;
}

void RendererEnableCameraLock(Renderer *renderer)
{
    renderer->camera_lock = 1;
}

void RendererEnableAnimations(Renderer *renderer)
{
    renderer->animations_disable = 0;
}

void RendererDisableMap(Renderer *renderer)
{
    renderer->map_disable = 1;
}

void RendererDisableSprite(Renderer *renderer)
{
    renderer->sprite_disable = 1;
}

void RendererDisableTextBox(Renderer *renderer)
{
    renderer->tbox_disable = 1;
}

void RendererDisableCamera(Renderer *renderer)
{
    renderer->camera_disable = 1;
}

void RendererDisableCameraLock(Renderer *renderer)
{
    renderer->camera_lock = 0;
}

void RendererDisableAnimations(Renderer *renderer)
{
    renderer->animations_disable = 1;
}

void SetCamera(Renderer *renderer, Coord co)
{
    renderer->camera.co = co;
}

SpriteHandle SpriteLoad(Game *game, const char *name, CE_u32 frame_delay)
{
    char *sprite_path;
    SDL_Texture *texture;
    CE_u64 length;
    
    length = snprintf(0, 0, "res/sprites/%s.png", name);
    sprite_path = CE_ArenaPush(game->map_arena, length+1);
    snprintf(sprite_path, length+1, "res/sprites/%s.png", name);

    SDL_Surface *sprite_surface = IMG_Load(sprite_path);
    if (!sprite_surface) {
        AlertError("Resource Error", "Could not open sprite file: '%s'", sprite_path);
        return SpriteNil();
    }

    texture = SDL_CreateTextureFromSurface(game->renderer->renderer, sprite_surface);
    SDL_FreeSurface(sprite_surface);
    if (!texture) {
        LogError("Failed to allocate sprite texture: %s", SDL_GetError());
        return SpriteNil();
    }
    
    return SpriteNew(game->renderer, texture, frame_delay);
}

SpriteHandle SpriteNew(Renderer *renderer, SDL_Texture *texture, CE_u32 frame_delay)
{
    Sprite *pool = renderer->sprite_pool;
    CE_i64 result = -1;
    for (CE_i64 i = 0; i < MAX_ENTITIES; ++i) {
        if (pool[i].texture == 0) {
            result = i;
            break;
        }
    }

    if (result < 0) {
        LogWarning("Could not allocate sprite: out of slots.");
        return SpriteNil();
    }

    int width = 0;
    Q(SDL_QueryTexture(texture, 0, 0, &width, 0));

    pool[result] = (Sprite) {0};
    pool[result].texture = texture;
    pool[result].frame_delay = frame_delay;
    pool[result].frame_max = width / TILE;

    return (SpriteHandle) {result};
}

void SpriteFree(Renderer *renderer, SpriteHandle id)
{
    Sprite *sprite = SpriteGet(renderer, id);
    if (sprite == 0) {
        LogWarning("Double free of sprite (%lld)", id.handle);
        return;
    }

    SDL_DestroyTexture(sprite->texture);
    sprite->texture = 0;
}

void SpriteDraw(Renderer *renderer, SpriteHandle id, Coord co)
{
    Sprite *sprite = SpriteGet(renderer, id);
    if (sprite == 0) {
        LogWarning("Failed to queue invalid sprite (%lld)", id.handle);
        return;
    }

    sprite->co = co;
    if (renderer->sprite_queue.end >= MAX_ENTITIES) {
        LogWarning("Sprite queue is full");
        return;
    }

    renderer->sprite_queue.queue[renderer->sprite_queue.end] = id;
    renderer->sprite_queue.end += 1;
}

void SpriteSetAnimation(Renderer *renderer, SpriteHandle id, CE_u32 animation)
{
    Sprite *sprite = SpriteGet(renderer, id);
    if (sprite == 0) {
        LogWarning("Setting animation of nil sprite (%lld)", id.handle);
        return;
    }

    sprite->animation = animation;
}

void SpriteStartAnimation(Renderer *renderer, SpriteHandle id)
{
    Sprite *sprite = SpriteGet(renderer, id);
    if (sprite == 0) {
        LogWarning("Starting animation of nil sprite (%lld)", id.handle);
        return;
    }
    
    sprite->is_playing = 1;
}

void SpriteStopAnimation(Renderer *renderer, SpriteHandle id)
{
    Sprite *sprite = SpriteGet(renderer, id);
    if (sprite == 0) {
        LogWarning("Stoping animation of nil sprite (%lld)", id.handle);
        return;
    }
    
    sprite->is_playing = 0;
}

void SpriteResetAnimation(Renderer *renderer, SpriteHandle id)
{
    Sprite *sprite = SpriteGet(renderer, id);
    if (sprite == 0) {
        LogWarning("Resetting animation of nil sprite (%lld)", id.handle);
        return;
    }
    
    sprite->frame = 0;
}

void TextBoxOpenTop(Renderer *renderer)
{
    TextBoxClear(renderer);
    renderer->text_box.state = TBOX_STATIC;
    renderer->text_box.box = TBOX_TOP;
}

void TextBoxOpenBottom(Renderer *renderer)
{
    TextBoxClear(renderer);
    renderer->text_box.state = TBOX_STATIC;
    renderer->text_box.box = TBOX_BOTTOM;
}

void TextBoxClose(Renderer *renderer)
{
    renderer->text_box.state = TBOX_CLOSED;
}

void TextBoxClear(Renderer *renderer)
{
    for (CE_u64 i = 0; i < TBOX_ROWS; ++i) {
        renderer->text_box.rows[i][0] = 0;
    }
    renderer->text_box.scroll = 0;
    renderer->text_box.line = 0;
    if (TextBoxIsScrolling(renderer)) {
        renderer->text_box.state = TBOX_STATIC;
    }
}

void TextBoxScroll(Renderer *renderer)
{
    if (TextBoxIsOpen(renderer)) {
        renderer->text_box.state = TBOX_SCROLLING;
    }
    renderer->text_box.scroll = 0;
}

void TextBoxNewLine(Renderer *renderer)
{
    if (renderer->text_box.line + 1 >= TBOX_ROWS) {
        LogWarning("Advancing text box past limit");
        return;
    }
    renderer->text_box.line += 1;
    renderer->text_box.rows[renderer->text_box.line][0] = 0;
    if (renderer->text_box.line == TBOX_ROWS-1) {
        TextBoxScroll(renderer);
    }
}

void TextBoxAppend(Renderer *renderer, String text)
{
    char *line_buffer = renderer->text_box.rows[renderer->text_box.line];
    CE_u64 line_length = strlen(line_buffer);

    if (text.len + 1 > TBOX_LIMIT - line_length) {
        LogWarning("Line buffer overflow; truncating text: (%llu) %.*s",
                TBOX_LIMIT - line_length - 1, StrVArg(text));
        text.len = TBOX_LIMIT - line_length - 1;
    }
    
    strncat(line_buffer, text.str, text.len);
}

int TextBoxIsOpen(Renderer *renderer)
{
    return renderer->text_box.state != TBOX_CLOSED;
}

int TextBoxIsScrolling(Renderer *renderer)
{
    return renderer->text_box.state == TBOX_SCROLLING;
}

void PromptDraw(Renderer *renderer, int choice)
{
    renderer->prompt_state = choice;
}

void CurtainRaise(Renderer *renderer)
{
    renderer->curtain.target_height = 0;
}

void CurtainFall(Renderer *renderer)
{
    renderer->curtain.target_height = HEIGHT;
}

int CurtainIsDown(Renderer *renderer)
{
    return renderer->curtain.height == HEIGHT;
}

int CurtainInMotion(Renderer *renderer)
{
    return renderer->curtain.height != renderer->curtain.target_height;
}
