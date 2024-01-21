
#include "perpix.h"

static uint8_t g_ui_zoom = 10;

MERROR_RETVAL ui_handle_input_queue( struct PERPIX_DATA* data ) {
   struct RETROFLAT_INPUT input_evt;
   RETROFLAT_IN_KEY input = 0;
   struct PERPIX_GRID_PACK* grid_pack = NULL;
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_GRID* grid = NULL;

   /* Process input events until none remain. */
   do {
      input = retroflat_poll_input( &input_evt );

      switch( input ) {
      case RETROFLAT_KEY_ESC:
         retroflat_quit( 0 );
         break;

      case RETROFLAT_MOUSE_B_LEFT:
         /* Lock the grid specially to draw this pixel. */
         maug_mlock( data->grid_pack_h, grid_pack );
         maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

         grid = grid_get_layer_p( grid_pack, data->layer_idx );
         if( UI_GRID_X < input_evt.mouse_x ) {
            ui_click_px( data, grid, input_evt.mouse_x, input_evt.mouse_y );
         } else if( UI_GRID_Y < input_evt.mouse_y ) {
            ui_click_palette(
               data, grid, input_evt.mouse_x, input_evt.mouse_y );
         }
         
         maug_munlock( data->grid_pack_h, grid_pack );
         break;
      }

   } while( input );

cleanup:
   return retval;
}

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
   uint8_t grid_px = 0;

   p_px = grid_px( grid );

   debug_printf( 1, "redrawing grid: " UPRINTF_U32_FMT "x" UPRINTF_U32_FMT,
      grid->w, grid->h );
   
   for( y_iter = 0 ; grid->h > y_iter ; y_iter++ ) {
      for( x_iter = 0 ; grid->w > x_iter ; x_iter++ ) {
         
         grid_px = grid_px_at( grid, p_px, x_iter, y_iter );
         
         if( PERPIX_MASK_PX_TRANS == (PERPIX_MASK_PX_TRANS & grid_px) ) {
            /* TODO: Draw transparent color. */

            grid_px &= ~PERPIX_MASK_PX_TRANS;
         }

         if( 16 <= grid_px ) {
            error_printf( "invalid pixel at %ux%u: 0x%02x",
               x_iter, y_iter, grid_px );
            continue;
         }

         /* Don't bother with the palette... ui_draw_palette() should've
          * taken care of it!
          */

         retroflat_rect(
            NULL,
            grid_px,
            UI_GRID_X + (x_iter * (g_ui_zoom + 1)),
            UI_GRID_Y + (y_iter * (g_ui_zoom + 1)),
            g_ui_zoom, g_ui_zoom,
            RETROFLAT_FLAGS_FILL );

      }
   }
}

void ui_draw_layer_icons( struct PERPIX_GRID_PACK* grid_pack ) {
   uint32_t grid_y_iter = UI_GRID_ICON_Y,
      px_x = 0,
      px_y = 0,
      layer_idx = 0;
   uint8_t* p_px = NULL;
   uint8_t grid_px = 0;
   struct PERPIX_GRID* grid = NULL;

   for( layer_idx = 0 ; grid_pack->count > layer_idx ; layer_idx++ ) {
      grid = grid_get_layer_p( grid_pack, layer_idx );
      p_px = grid_px( grid );
      for( px_y = 0 ; grid->h > px_y ; px_y++ ) {
         for( px_x = 0 ; grid->w > px_x ; px_x++ ) {

            grid_px = grid_px_at( grid, p_px, px_x, px_y );

            grid_px &= ~PERPIX_MASK_PX_TRANS;
            if( 16 <= grid_px ) {
               grid_px = 15;
            }

            retroflat_px( NULL, grid_px,
               UI_GRID_ICON_X + px_x, grid_y_iter + px_y, 0 );
         }
      }
      grid_y_iter += 64;
   }
}

void ui_scale_zoom( uint16_t w, uint16_t h, struct PERPIX_GRID* grid ) {
   if( NULL != grid && 0 < grid->h ) {
      g_ui_zoom = (h - (2 * UI_GRID_Y)) / grid->h;
   }
}

void ui_click_px(
   struct PERPIX_DATA* data, struct PERPIX_GRID* grid,
   uint16_t mouse_x, uint16_t mouse_y
) {
   int16_t g_x = 0,
      g_y = 0;
   uint8_t* p_px = NULL;

   p_px = grid_px( grid );

   g_x = ui_screen_to_grid_x( mouse_x );
   g_y = ui_screen_to_grid_y( mouse_y );

   /* TODO: Implement multiple tools with callbacks. */

   if( 0 <= g_x && grid->w > g_x && 0 <= g_y && grid->h > g_y ) { 
      grid_px_at( grid, p_px, g_x, g_y ) = data->fg_idx;

      retroflat_rect(
         NULL,
         grid_px_at( grid, p_px, g_x, g_y ),
         UI_GRID_X + (g_x * (g_ui_zoom + 1)),
         UI_GRID_Y + (g_y * (g_ui_zoom + 1)),
         g_ui_zoom, g_ui_zoom,
         RETROFLAT_FLAGS_FILL );
   }
}

void ui_click_palette(
   struct PERPIX_DATA* data, struct PERPIX_GRID* grid,
   uint16_t mouse_x, uint16_t mouse_y
) {
   int16_t g_x = 0,
      g_y = 0;

   g_x = ui_screen_to_palette_x( mouse_x );
   g_y = ui_screen_to_palette_y( mouse_y );

   /* Translate palette X/Y into color index. */
   if( 0 <= g_x && 2 > g_x && 0 <= g_y && grid->palette_ncolors / 2 > g_y ) { 
      data->fg_idx = g_y;
      if( 0 < g_x ) {
         /* In right-hand column, so add left-hand column count. */
         data->fg_idx += (grid->palette_ncolors / 2);
      }
   }
}

