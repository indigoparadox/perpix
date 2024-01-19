
#include "perpix.h"

void palette_draw( uint16_t x, uint16_t y, struct PERPIX_GRID* grid ) {
   size_t i = 0;
   uint16_t y_iter = y;
   MERROR_RETVAL set_retval = 0;

   for( i = 0 ; grid->palette_sz > i ; i++ ) {
      
      set_retval = retroflat_set_palette( i, grid->palette[i] );
      if( set_retval ) {
         /* If you can't beat 'em, join 'em! */
         retroflat_get_palette( i, &(grid->palette[i]) );
      }

      retroflat_rect(
         NULL, i, x, y_iter, PERPIX_PALETTE_SQ_SZ, PERPIX_PALETTE_SQ_SZ,
         RETROFLAT_FLAGS_FILL );

      y_iter += PERPIX_PALETTE_SQ_SZ;
      if( i + 1 == grid->palette_sz / 2 ) {
         x += PERPIX_PALETTE_SQ_SZ;
         y_iter = y;
      }
   }

}

