
#ifndef UI_H
#define UI_H

#define UI_PALETTE_SQ_SZ 10

#define UI_PALETTE_X 10
#define UI_PALETTE_Y 20
#define UI_GRID_X 50
#define UI_GRID_Y 20
#define UI_GRID_ICON_X 10
#define UI_GRID_ICON_Y_BUFFER 10

#define ui_screen_to_grid_y( y ) \
   ((y - UI_GRID_Y) / (g_ui_zoom + 1))

#define ui_screen_to_grid_x( x ) \
   ((x - UI_GRID_X) / (g_ui_zoom + 1))

#define ui_screen_to_palette_y( y ) \
   ((y - UI_PALETTE_Y) / (UI_PALETTE_SQ_SZ))

#define ui_screen_to_palette_x( x ) \
   ((x - UI_PALETTE_X) / (UI_PALETTE_SQ_SZ))

#define ui_palette_height( p_grid ) \
   (((p_grid)->palette_ncolors >> 1) * UI_PALETTE_SQ_SZ)

#define ui_inc_redraws( p_data ) \
   if( !p_data->redraws ) { \
      p_data->redraws += 2; \
      debug_printf( \
         1, "incremented redraws to " UPRINTF_S32_FMT, p_data->redraws ); \
   }

MERROR_RETVAL ui_handle_input_queue( struct PERPIX_DATA* data );

void ui_draw_palette( struct PERPIX_GRID* grid );

void ui_draw_grid( struct PERPIX_GRID* grid );

void ui_draw_layer_icons(
   struct PERPIX_DATA* data, struct PERPIX_GRID_PACK* grid_pack );

void ui_scale_zoom( uint16_t w, uint16_t h, struct PERPIX_GRID* grid );

void ui_click_px(
   struct PERPIX_DATA* data, struct PERPIX_GRID* grid,
   uint16_t mouse_x, uint16_t mouse_y );

void ui_click_palette(
   struct PERPIX_DATA* data, struct PERPIX_GRID* grid,
   uint16_t mouse_x, uint16_t mouse_y );

void ui_click_layer_icons(
   struct PERPIX_DATA* data, struct PERPIX_GRID_PACK* grid_pack,
   uint16_t mouse_x, uint16_t mouse_y );

#endif /* !UI_H */

