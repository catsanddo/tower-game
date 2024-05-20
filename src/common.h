#ifndef TYPES_H
#define TYPES_H

#define StrVArg(s) (int) (s).len, (s).str
#define StrIsNil(s) ((s).str == 0 || (s).len == 0)
#define StrLit(c) ((String) {c, sizeof(c) - 1})

#define SpriteIsNil(s) (s.handle < 0)
#define SpriteNil() ((SpriteHandle) {-1})

#define EntityIsNil(h) ((h).handle < 0)
#define EntityNil() ((EntityHandle) {-1})

#define Sign(n) ((n) > 0 ? 1 : (n) < 0 ? -1 : 0)
#define Abs(n) (n < 0 ? -n : n)
#define Max(a, b) (a > b ? a : b)
#define Min(a, b) (a < b ? a : b)
#define StaticArraySize(a) (sizeof(a) / sizeof(a[0]))

typedef struct Vec2 Vec2;
struct Vec2 {
    union {
        CE_i32 x;
        CE_i32 w;
    };
    union {
        CE_i32 y;
        CE_i32 h;
    };
};

typedef struct Coord Coord;
struct Coord {
    Vec2 position;
    Vec2 offset;
};

typedef struct String String;
struct String {
    char *str;
    CE_u64 len;
};

typedef struct SpriteHandle SpriteHandle;
struct SpriteHandle {
    CE_i64 handle;
};

typedef struct EntityHandle EntityHandle;
struct EntityHandle {
    CE_i64 handle;
};

#endif
