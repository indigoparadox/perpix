
#ifndef GRID_H
#define GRID_H

#include <math.h>

#define grid_palette( p_grid ) \
   (uint32_t*)(((uint8_t*)(p_grid)) + (p_grid)->palette_offset)

#define grid_px( p_grid ) \
   (uint8_t*)(((uint8_t*)(p_grid)) + (p_grid)->px_offset)

#define grid_px_at( p_grid, p_grid_px, x, y ) \
   (p_grid_px[(y * (p_grid)->w) + x])

MERROR_RETVAL grid_pack_new_h( MAUG_MHANDLE* p_grid_pack_h );

MERROR_RETVAL grid_pack_add_layer(
   uint32_t w, uint32_t h, uint32_t pal_ncolors, MAUG_MHANDLE* p_grid_pack_h
);

#endif /* !GRID_H */

