
#ifndef BMP_H
#define BMP_H

struct CONVERT_GRID;
struct CONVERT_OPTIONS;

struct
#ifdef __GNUC__
__attribute__( (__packed__) )
#endif /* __GNUC__ */
BMP_FILE_HEADER {
   char id[2];
   uint32_t file_sz;
   uint16_t reserved1;
   uint16_t reserved2;
   uint32_t bmp_offset;
};

struct
#ifdef __GNUC__
__attribute__( (__packed__) )
#endif /* __GNUC__ */
BMP_DATA_HEADER {
   uint32_t header_sz;
   int32_t bitmap_w;
   int32_t bitmap_h;
   uint16_t planes;
   uint16_t bpp;
   uint32_t compression;
   uint32_t image_sz;
   int32_t hres;
   int32_t vres;
   uint32_t colors;
   uint32_t colors_important;
};

struct BMP_OPTS {
   uint32_t sz;
   uint8_t flags;
};

#define BMP_OPTS_FLAG_UPSIDE_DOWN 0x01

#define BMP_COMPRESSION_NONE (0)

#if 0
/**
 * \return The byte size of the given ::CONVERT_GRID if written as a bitmap.
 */
int32_t bmp_grid_sz(
   const struct CONVERT_GRID* grid, struct CONVERT_OPTIONS* o );
uint8_t bmp_colors_count( uint8_t );
int32_t bmp_write_file(
   const char*, const struct CONVERT_GRID*, struct CONVERT_OPTIONS* );
int32_t bmp_write(
   uint8_t*, uint32_t, const struct CONVERT_GRID*, struct CONVERT_OPTIONS* );
struct CONVERT_GRID* bmp_read_file( const char*, struct CONVERT_OPTIONS* );
struct CONVERT_GRID* bmp_read(
   const uint8_t*, uint32_t, struct CONVERT_OPTIONS* );
#endif

#define bmp_int( type, buf, offset ) *((type*)&(buf[offset]))

#endif /* BMP_H */

