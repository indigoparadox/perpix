
#ifndef PERPIX_H
#define PERPIX_H

#include <maug.h>
#ifndef PERPIX_PLUGIN
#  include <retroflt.h>
#  include <retrofil.h>
#  include <retrogui.h>
#endif /* !PERPIX_PLUGIN F*/

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

#define perpix_read_lsbf_32( bytes, offset ) \
   ((bytes[offset]) | (bytes[offset + 1] << 8) | (bytes[offset + 2] << 16) \
   | (bytes[offset + 3] << 24))

#define perpix_read_lsbf_16( bytes, offset ) \
   ((bytes[offset]) | (bytes[offset + 1] << 8))

struct PERPIX_GRID {
   uint32_t version;
   uint32_t palette_offset;
   uint32_t px_offset;
   int32_t w;
   int32_t h;
   uint32_t bpp;
   uint32_t data_sz;
   uint32_t palette_ncolors;
};

struct PERPIX_DATA {
   int init;
   uint8_t flags;
   uint8_t fg_idx;
   uint8_t bg_idx;
   MAUG_MHANDLE grid_h;
};

struct PERPIX_PLUG_ENV {
   uint32_t version;
   uint8_t flags;
   struct PERPIX_GRID* grid;
   uint8_t* buf;
   size_t buf_sz;
   size_t layer_sz;
};

#include "ui.h"
#include "grid.h"

#endif /* PERPIX_H */

