
#ifndef PERPIX_H
#define PERPIX_H

#include <maug.h>
#include <retroflt.h>

#define PERPIX_FLAG_REDRAW_UI 0x01

#ifndef PERPIX_GRID_W_MAX
#  define PERPIX_GRID_W_MAX 128
#endif /* !PERPIX_GRID_W_MAX */

#ifndef PERPIX_GRID_H_MAX
#  define PERPIX_GRID_H_MAX 128
#endif /* !PERPIX_GRID_H_MAX */

#ifndef PERPIX_GRID_PAL_SZ
#  define PERPIX_GRID_PAL_SZ 16
#endif /* !PERPIX_GRID_PAL_SZ */

#define PERPIX_DEF_COLOR_TABLE( f ) f( 0, black, 0x00000000 ) f( 1, blue, 0x000000aa ) f( 2, green, 0x0000aa00 ) f( 3, cyan, 0x0000aaaa ) f( 4, red, 0x00aa0000 ) f( 5, magenta, 0x00aa00aa ) f( 6, brown, 0x00aa5500 ) f( 7, light_gray, 0x00aaaaaa ) f( 8, dark_gray, 0x00555555 ) f( 9, light_blue, 0x005555ff ) f( 10, light_green, 0x0055ff55 ) f( 11, light_cyan, 0x0055ffff ) f( 12, light_red, 0x00ff5555 ) f( 13, light_magenta, 0x00ff55ff ) f( 14, yellow, 0x00ffff55 ) f( 15, white, 0x00ffffff )

struct PERPIX_GRID {
   uint8_t version;
   uint32_t palette_offset;
   uint32_t px_offset;
   uint32_t w;
   uint32_t h;
   uint16_t bpp;
   uint32_t data_sz;
   uint32_t palette_sz;
   uint32_t palette[PERPIX_GRID_PAL_SZ];
   uint8_t px[PERPIX_GRID_H_MAX][PERPIX_GRID_W_MAX];
};

#include "palette.h"
#include "grid.h"

#endif /* PERPIX_H */

