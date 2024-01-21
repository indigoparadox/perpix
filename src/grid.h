
#ifndef GRID_H
#define GRID_H

#include <math.h>

#define grid_palette( p_grid ) \
   (uint32_t*)(((uint8_t*)(p_grid)) + (p_grid)->palette_offset)

#define grid_px( p_grid ) \
   (uint8_t*)(((uint8_t*)(p_grid)) + (p_grid)->px_offset)

#define grid_px_at( p_grid, p_grid_px, x, y ) \
   (p_grid_px[(y * (p_grid)->w) + x])

#define grid_pack_layer( p_grid_pack )

MERROR_RETVAL grid_pack_new_h( MAUG_MHANDLE* p_grid_pack_h );

MERROR_RETVAL grid_pack_add_layer(
   uint32_t w, uint32_t h, uint32_t pal_ncolors, MAUG_MHANDLE* p_grid_pack_h
);

/**
 * \note This is built as a static function for simplicity in making it
 *       available to plugins while still having local variables.
 */
static struct PERPIX_GRID* grid_get_layer_p(
   struct PERPIX_GRID_PACK* grid_pack, uint32_t layer
) {
   struct PERPIX_GRID* grid_iter = NULL;
   uint8_t* grid_pack_buf = (uint8_t*)grid_pack;

   if( 0 == grid_pack->count || layer >= grid_pack->count ) {
      error_printf( "invalid layer " UPRINTF_U32_FMT
         " requested (of " UPRINTF_U32_FMT ")!",
         layer, grid_pack->count );
      return NULL;
   }

   /* Advance to first grid. */
   grid_pack_buf += sizeof( struct PERPIX_GRID_PACK );
   grid_iter = (struct PERPIX_GRID*)grid_pack_buf;
   while( layer > 0 ) {
      grid_pack_buf += grid_iter->sz;
      grid_iter = (struct PERPIX_GRID*)grid_pack_buf;
      layer--;
   }

   return grid_iter;
}

#endif /* !GRID_H */

