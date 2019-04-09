#ifndef MISC_TOOLS_H
#define MISC_TOOLS_H

#include <assert.h>

//
// Finding the operating system
//
#define OS_WINDOWS 1
#define OS_MAC     2
#define OS_IOS     3
#define OS_LINUX   4
#define OS_WASM    5

#define OS 0

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

#undef OS
#define OS OS_WASM
#define OS_IS_MOBILE  0
#define OS_IS_DESKTOP 0
#define OS_IS_WEB     1

#elif defined(_WIN32)

#undef OS
#define OS OS_WINDOWS
#define OS_IS_MOBILE  0
#define OS_IS_DESKTOP 1
#define OS_IS_WEB     0

#elif defined __APPLE__

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE

#undef OS
#define OS OS_IOS
#define OS_IS_MOBILE  1
#define OS_IS_DESKTOP 0
#define OS_IS_WEB     0

#else

#undef OS
#define OS OS_MAC
#define OS_IS_MOBILE  0
#define OS_IS_DESKTOP 1
#define OS_IS_WEB     0

#endif

#else

#undef OS
#define OS OS_LINUX
#define OS_IS_MOBILE  0
#define OS_IS_DESKTOP 1
#define OS_IS_WEB     0

#endif

#define MAX_PATH_LENGTH 300

//
// Misc macros
//
#define ArraySize(a) (sizeof(a)/sizeof(a[0]))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))



//
// Variable types
//
#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u32 uint;

typedef float  f32;
typedef double f64;

#define MAX_U8  0xFFU
#define MAX_U16 0xFFFFU
#define MAX_U32 0xFFFFFFFFU
#define MAX_U64 0xFFFFFFFFFFFFFFFFULL

//
// Some math functions
//
inline float lerp(float x0, float x1, float t){
    return x0*(1.f-t) + x1*t;
}

//
// Variables for rendering
//
struct RgbColor {
    u8 r, g, b;
};
struct RgbaColor {
    u8 r, g, b, a;
};

//
// Static arrays
//

template <typename T, u32 aS> struct StaticArray{
    u32 size;
#if OS == OS_WINDOWS
    T data[aS];// = {}; // I hate c++
#else
    T data[aS] = {};
#endif
    
    T& operator[](u32 index){
        if(index > size){
            printf("Array out of index!\n");
            assert(0);
        }
        return data[index];
    }
    
    void operator=(StaticArray a){
        size = a.size;
        memcpy(a.data, data, sizeof(data));
    }
    
    void add(u32 plus){
        size += plus;
        assert(size <= aS);
    }
    
    void remove(u32 index){
        for(u32 i=index+1; i<size; i++){
            data[i-1] = data[i];
        }
        size--;
    }
    
    void slice(u32 s){
        size = s;
    }
    
    void push(T t){
        size++;
        assert(size <= aS);
        data[size-1] = t;
    }
    
    StaticArray(){
        size = 0;
    }
    
    StaticArray(u32 s){
        size = s;
    }
    
    StaticArray(u32 s, u32 d_s, T d[]){
        size = s;
        memcpy(data, d, d_s*sizeof(T));
    }
};
template <typename T, u32 aS> inline void read_array(StaticArray<T, aS> *a, FILE *fp){
    fread(&a->size, sizeof(u32), 1, fp);
    fread(a->data, sizeof(T), a->size, fp);
}
template <typename T, u32 aS> inline void write_array(StaticArray<T, aS> *a, FILE *fp){
    fwrite(&a->size, sizeof(u32), 1, fp);
    fwrite(a->data, sizeof(T), a->size, fp);
}
template <typename T, u32 aS> inline void free(StaticArray<T, aS> &a){
    a.size = 0;
}

//
// Static multi-arrays
//
template <typename T, u32 size_0, u32 size_1, u32 size_2> struct StaticArray3{
    u32 size[3];
    T data[size_0*size_1*size_2];
    
    
    StaticArray3() : size{0, 0, 0} { }
    
    StaticArray3(u32 s) : size{s, s, s} { }
    
    StaticArray3(u32 s0, u32 s1, u32 s2) : size{s0, s1, s2} { }
};
template <typename T, u32 size_0, u32 size_1, u32 size_2> inline void read_array(StaticArray3<T, size_0, size_1, size_2> *a, FILE *fp){
    fread(a->size, sizeof(u32), 3, fp);
    fread(a->array.data, sizeof(T), size_0*size_1*size_2, fp);
}
template <typename T, u32 size_0, u32 size_1, u32 size_2> inline void write_array(StaticArray3<T, size_0, size_1, size_2> *a, FILE *fp){
    fwrite(a->size, sizeof(u32), 3, fp);
    fwrite(a->array.data, sizeof(T), size_0*size_1*size_2, fp);
}
template <typename T, u32 size_0, u32 size_1, u32 size_2> inline void free(StaticArray3<T, size_0, size_1, size_2> &a){
    a.size[0] = 0;
    a.size[1] = 0;
    a.size[2] = 0;
}

template <typename T, u32 size_0, u32 size_1, u32 size_2> inline T& array_at(StaticArray3<T, size_0, size_1, size_2> &a, int x, int y, int z){
    if(x < 0 || x >= a.size[0]){
        printf("StaticArray3 at: Array out of bounds: [%i] %i %i\n", x, y, z);
        assert(0);
    }
    if(y < 0 || y >= a.size[1]){
        printf("StaticArray3 at: Array out of bounds: %i [%i] %i\n", x, y, z);
        assert(0);
    }
    if(z < 0 || z >= a.size[2]){
        printf("StaticArray3 at: Array out of bounds: %i %i [%i]\n", x, y, z);
        assert(0);
    }
    return a.data[a.size[2]*(a.size[1]*x + y) + z];
}
template <typename T, u32 size_0, u32 size_1, u32 size_2> inline void array_set(StaticArray3<T, size_0, size_1, size_2> &a, int x, int y, int z, T val){
    if(x < 0 || x >= a.size[0]){
        printf("StaticArray3 set: Array out of bounds\n");
        assert(0);
    }
    if(y < 0 || y >= a.size[1]){
        printf("StaticArray3 set: Array out of bounds\n");
        assert(0);
    }
    if(z < 0 || z >= a.size[2]){
        printf("StaticArray3 set: Array out of bounds\n");
        assert(0);
    }
    a.data[a.size[2]*(a.size[1]*x + y) + z] = val;
}
template <typename T, u32 size_0, u32 size_1, u32 size_2> inline u32 array_linear_size(StaticArray3<T, size_0, size_1, size_2> &a){
    return a.size[0]*a.size[1]*a.size[2];
}


//
// Catalog
//
const int CATALOG_SIZE = 200;
struct Catalog {
    char filename[MAX_PATH_LENGTH];
    void *info;
    void (*callback)(void *info);
    u64 last_modification;
};
extern StaticArray<Catalog, 200> catalogs;
inline void catalog_add(const char *filename, void *info, void (*callback)(void *)){
    u32 s = catalogs.size;
    catalogs.add(1);
    assert(catalogs.size < 200);
    strcpy((char *)catalogs[s].filename, filename);
    catalogs[s].info = info;
    catalogs[s].callback = callback;
    catalogs[s].last_modification = 0;
};
inline void catalog_update(){
    for(uint i=0; i<catalogs.size; i++){
        u64 last_modification = ggtp_file_modification_date(catalogs[i].filename);
        if(last_modification > catalogs[i].last_modification){
            catalogs[i].last_modification = last_modification;
            catalogs[i].callback(catalogs[i].info);
        }
    }
}

//
// Temp alloc
//
struct {
    u64 bytes_allocated, bytes_used;
    u8 *memory;
} _temporary_memory;

inline int allocate_temporary_memory(u64 bytes){
    _temporary_memory.memory = (u8 *)malloc(bytes);
    if(_temporary_memory.memory == NULL)
        return 0;
    _temporary_memory.bytes_allocated = bytes;
    _temporary_memory.bytes_used = 0;
    return 1;
}

inline void reset_temporary_memory(){
    _temporary_memory.bytes_used = 0;
}

inline void *talloc(u64 bytes){
    u64 used = _temporary_memory.bytes_used;
    _temporary_memory.bytes_used += bytes;
    if(_temporary_memory.bytes_used > _temporary_memory.bytes_allocated){
        _temporary_memory.bytes_used = used;
        return NULL;
    }
    return (void *)(_temporary_memory.memory + used);
}

#define start_temp_alloc() u64 __xdeg__starting_temporary_storage_mark = _temporary_memory.bytes_used
#define end_temp_alloc() _temporary_memory.bytes_used = __xdeg__starting_temporary_storage_mark

#define KB(n) ((n) << 10)
#define MB(n) ((n) << 20)
#define GB(n) ((n) << 30)

//
// Font loading
//

int load_font_into_texture(const char *path, GLenum target);

const u8 TEXT_ALIGN_BOTTOM = 0;
const u8 TEXT_ALIGN_TOP    = 1;
const u8 TEXT_ALIGN_LEFT   = 0;
const u8 TEXT_ALIGN_RIGHT  = 2;
const u8 TEXT_MONOSPACE    = 4;

struct mt_Vertex_PTCa {
    Vec3 p;
    Vec2 t;
    RgbaColor c;
};

u32 render_text(float x, float y, float z, u8 quality, const char *s, mt_Vertex_PTCa *vertices, float font_size, float *width, float *height, RgbaColor color, u8 options);
float render_text_length(u8 size, const char *s);
float render_text_length(u8 size, const char *s, uint length);
int text_vert_num(const char *s);

#define FONT_QUALITIES 4
const u8 FONT_QUALITY_12 = 0;
const u8 FONT_QUALITY_16 = 1;
const u8 FONT_QUALITY_32 = 2;
const u8 FONT_QUALITY_64 = 3;

//
//
//

#endif // !defined(MISC_TOOLS_H)

//
// ==================
// = IMPLEMENTATION =
// ==================
//

#ifdef MISC_TOOLS_IMPLEMENTATION

//
// Catalog
//
StaticArray<Catalog, 200> catalogs;

//
// Font loading
//

#if OS_IS_MOBILE
#define FONT_TEXTURE_SIZE  512
#define FONT_TEXTURE_SIZEf 512.f
#define FONT_SIZE_IN_TEXTURE 64
#else
#define FONT_TEXTURE_SIZE  1024
#define FONT_TEXTURE_SIZEf 1024.f
#define FONT_SIZE_IN_TEXTURE 128
#endif

const int font_qualities[] = {12, 16, 32, 64};

typedef struct{
    char letter;
    Vec2 coords1, coords2, size, offset, advance;
} Glyph;

Glyph *glyphs;
unsigned int glyphNum;

int load_font_into_texture(const char *path, GLenum target){
    start_temp_alloc();
    stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
    
    u8 *temp_bitmap = (u8 *)talloc(FONT_TEXTURE_SIZE*FONT_TEXTURE_SIZE*sizeof(u8));
    
    GLuint ftex;
    
    FILE *f = fopen(path, "rb");
    if(f == NULL){
        printf("Couldn't open font '%s'\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);
    
    unsigned char *ttf_buffer = (unsigned char *)malloc(fsize + 1);
    fread(ttf_buffer, fsize, 1, f);
    fclose(f);
    
    stbtt_BakeFontBitmap(ttf_buffer, 0, FONT_SIZE_IN_TEXTURE, temp_bitmap, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 32, 96, cdata); // no guarantee this fits!
    free(ttf_buffer);
    
    glActiveTexture(target);
	printf("%p\n", glGenTextures);
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    end_temp_alloc();
    
    glyphNum = 96;
    glyphs = (Glyph *)malloc(glyphNum*sizeof(Glyph));
    const float scale_factor = 1.f;
    const float scale = scale_factor / FONT_SIZE_IN_TEXTURE;
    for(i8 i=0; i<96; i++){
        glyphs[i].letter = 32+i;
        glyphs[i].coords1.x = cdata[i].x0 / FONT_TEXTURE_SIZEf;
        glyphs[i].coords1.y = cdata[i].y1 / FONT_TEXTURE_SIZEf;
        glyphs[i].coords2.x = cdata[i].x1 / FONT_TEXTURE_SIZEf;
        glyphs[i].coords2.y = cdata[i].y0 / FONT_TEXTURE_SIZEf;
        glyphs[i].size = Vec2((f32)(cdata[i].x1-cdata[i].x0), (f32)(cdata[i].y1-cdata[i].y0)) * scale;
        glyphs[i].offset.x = cdata[i].xoff * scale;
        glyphs[i].offset.y = cdata[i].yoff * scale;
        glyphs[i].advance.x = cdata[i].xadvance * scale;
        glyphs[i].advance.y = 0.f;
    }
    
    return 0;
}


int text_vert_num(const char *s){
    // Render log
    // Count the total squares
    uint length = (uint)strlen(s);
    
    int squares = 0;
    for(uint i=0; i<length; i++){
        for(uint j=0; j<glyphNum; j++)
            if(s[i] == glyphs[j].letter){
            if(glyphs[j].size.x > 0)
                squares += 6;
            break;
        }
    }
    return squares;
}

float render_text_length(u8 size, const char *s){
    float x = 0.f;
    float maxX = 0.f;
    
    uint length = (uint)strlen(s);
    
    for(uint i=0; i<length; i++){
        if(s[i] == '\n'){
            if(x > maxX)
                maxX = x;
            x = 0.f;
        }
        for(uint j=0; j<glyphNum; j++)
            if(s[i] == glyphs[j].letter){
            Glyph &g = glyphs[j];
            
            x += g.advance.x;
            break;
        }
    }
    if(x > maxX)
        maxX = x;
    
    return maxX;
}
float render_text_length(u8 size, const char *s, uint length){
    float x = 0.f;
    float maxX = 0.f;
    
    uint l = (uint)strlen(s);
    
    for(uint i=0; i<l && i<length; i++){
        if(s[i] == '\n'){
            if(x > maxX)
                maxX = x;
            x = 0.f;
        }
        for(uint j=0; j<glyphNum; j++)
            if(s[i] == glyphs[j].letter){
            Glyph &g = glyphs[j];
            
            x += g.advance.x;
            break;
        }
    }
    if(x > maxX)
        maxX = x;
    
    return maxX;
}

u32 render_text(float x, float y, float z, u8 quality, const char *s, mt_Vertex_PTCa *vertices, float font_size, float *width, float *height, RgbaColor color, u8 flags){
    mt_Vertex_PTCa *o_vertices = vertices;
    if(flags & TEXT_ALIGN_TOP)
        y -= font_size;
    float x0 = x;
    float y0 = y;
    
    uint length = (uint)strlen(s);
    float maxX = x;
    
    int squares = 0;
    for(uint i=0; i<length; i++){
        char c = s[i];
        if(c >= 32 && c <= 126){
            int j = c-32;
            if(glyphs[j].size.x > 0)
                squares += 6;
            break;
        }
    }
    
    y += -font_size;
    mt_Vertex_PTCa *last_line_vertices = vertices;
    for(uint i=0; i<length; i++){
        char c = s[i];
        if(c == '\n'){
            if(x > maxX)
                maxX = x;
            if(flags & TEXT_ALIGN_RIGHT){
                float dif = x-x0;
                for(auto v=last_line_vertices; v<vertices; v++)
                    v->p.x -= dif;
            }
            last_line_vertices = vertices;
            x = x0;
            y -= font_size;
        }else if(c >= 32 && c <= 126){
            int j = c-32;
            Glyph &g = glyphs[j];
            
            const float advance_x = 0.55f*font_size;
            float half_advance_x = 0.55f*(advance_x - font_size*g.advance.x);
            
            if(flags & TEXT_MONOSPACE)
                x += half_advance_x;
            
            if(g.size.x > 0){
                float x1 = x+font_size*g.offset.x, y1 = y+font_size*(1.f-g.offset.y);
                float x2 = x1+font_size*g.size.x, y2 = y1-font_size*g.size.y;
                *(vertices++) = {Vec3(x1, y1, z), Vec2(g.coords1.x, g.coords2.y), color};
                *(vertices++) = {Vec3(x2, y1, z), Vec2(g.coords2.x, g.coords2.y), color};
                *(vertices++) = {Vec3(x1, y2, z), Vec2(g.coords1.x, g.coords1.y), color};
                *(vertices++) = {Vec3(x2, y1, z), Vec2(g.coords2.x, g.coords2.y), color};
                *(vertices++) = {Vec3(x1, y2, z), Vec2(g.coords1.x, g.coords1.y), color};
                *(vertices++) = {Vec3(x2, y2, z), Vec2(g.coords2.x, g.coords1.y), color};
            }
            
            if(flags & TEXT_MONOSPACE)
                x += advance_x-half_advance_x;
            else
                x += font_size*g.advance.x;
            y -= font_size*g.advance.y;
        }
    }
    if(x > maxX)
        maxX = x;
    
    if(width != nullptr)
        *width = maxX-x0;
    if(height != nullptr)
        *height = y0-y+font_size;
    return (u32)(vertices-o_vertices);
}

#endif