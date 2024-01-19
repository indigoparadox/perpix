
#include "perpix.h"

MERROR_RETVAL grid_new(
   uint32_t w, uint32_t h, uint32_t pal_sz, struct PERPIX_GRID* g
) {
   MERROR_RETVAL retval = MERROR_OK;

   memset( g, '\0', sizeof( struct PERPIX_GRID ) );
   g->version = 1;
   g->w = w;
   g->h = h;
   g->palette_sz = pal_sz;
   g->palette_offset = offsetof( struct PERPIX_GRID, palette );
   g->px_offset = offsetof( struct PERPIX_GRID, px );

   #define GRID_PAL_FILL( idx, name, val ) \
      assert( idx < pal_sz ); \
      g->palette[idx] = val;

   PERPIX_DEF_COLOR_TABLE( GRID_PAL_FILL );

   return retval;
}

