
#include "perpix.h"

static uint8_t g_ui_zoom = 10;

void ui_draw_palette( struct PERPIX_GRID* grid ) {
   size_t i = 0;
   uint16_t x_iter = UI_PALETTE_X,
      y_iter = UI_PALETTE_Y;
   MERROR_RETVAL set_retval = 0;
   uint32_t* p_palette = NULL;

   p_palette = grid_palette( grid );

   for( i = 0 ; grid->palette_ncolors > i ; i++ ) {
      
      set_retval = retroflat_set_palette( i, p_palette[i] );
      if( set_retval ) {
         /* If you can't beat 'em, join 'em! */
         retroflat_get_palette( i, &(p_palette[i]) );
      }

      retroflat_rect(
         NULL, i, x_iter, y_iter, UI_PALETTE_SQ_SZ, UI_PALETTE_SQ_SZ,
         RETROFLAT_FLAGS_FILL );

      y_iter += UI_PALETTE_SQ_SZ;
      if( i + 1 == grid->palette_ncolors / 2 ) {
         x_iter += UI_PALETTE_SQ_SZ;
         y_iter = UI_PALETTE_Y;
      }
   }
}

void ui_draw_grid( struct PERPIX_GRID* grid ) {
   uint16_t x_iter = 0,
      y_iter = 0;
   uint8_t* p_px = NULL;

   p_px = grid_px( grid );
   
   for( y_iter = 0 ; grid->h > y_iter ; y_iter++ ) {
      for( x_iter = 0 ; grid->w > x_iter ; x_iter++ ) {

         /* Don't bother with the palette... ui_draw_palette() should've
          * taken care of it!
          */

         retroflat_rect(
            NULL,
            grid_px_at( grid, p_px, x_iter, y_iter ),
            UI_GRID_X + (x_iter * (g_ui_zoom + 1)),
            UI_GRID_Y + (y_iter * (g_ui_zoom + 1)),
            UI_PALETTE_SQ_SZ, UI_PALETTE_SQ_SZ,
            RETROFLAT_FLAGS_FILL );

      }
   }
}

