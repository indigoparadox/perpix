
#define MAUG_C
#define RETROFLT_C
#define MFILE_C
#include "perpix.h"

char g_file_to_open[RETROFLAT_PATH_MAX];
MAUG_CONST char gc_plug_exts[][4] = {"bmp", "ico"};

MERROR_RETVAL perpix_on_resize( uint16_t new_w, uint16_t new_h, void* data ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_DATA* data_p = (struct PERPIX_DATA*)data;
   struct PERPIX_GRID_PACK* grid_pack = NULL;
   struct PERPIX_GRID* grid = NULL;

   /* Resize the virtual buffer and redraw the UI. */

   debug_printf( 1, "resize event triggered..." );

   retroflat_resize_v();

   maug_mlock( data_p->grid_pack_h, grid_pack )
   maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

   grid = grid_get_layer_p( grid_pack, data_p->layer_idx );
   if( NULL == grid ) {
      error_printf( "coult not get grid to resize!" );
      retval = MERROR_ALLOC;
      goto cleanup;
   }
   
   ui_scale_zoom( new_w, new_h, grid );
   maug_munlock( data_p->grid_pack_h, grid_pack )

   ui_inc_redraws( data_p );

cleanup:
   return retval;
}

MERROR_RETVAL perpix_read_file(
   const char* plug_ext, mfile_t* file_in,
   MAUG_MHANDLE* p_grid_pack_h
) {
   MERROR_RETVAL retval = MERROR_OK;
   mplug_mod_t mod_exe = NULL;
   struct PERPIX_PLUG_ENV plug_env;
   char plugin_call_buf[RETROFLAT_PATH_MAX + 1];
   struct PERPIX_GRID test_grid;
   struct PERPIX_GRID_PACK* grid_pack = NULL;

   memset( plugin_call_buf, '\0', RETROFLAT_PATH_MAX + 1 );
   maug_snprintf( plugin_call_buf, RETROFLAT_PATH_MAX, "./perpix_%s",
      plug_ext );
   retval = mplug_load( plugin_call_buf, &mod_exe );
   maug_cleanup_if_not_ok();

   plug_env.layer_idx = 0;
   do {
      plug_env.flags = PERPIX_PLUG_FLAG_HEADER_ONLY;
      plug_env.grid_pack = NULL;
      plug_env.test_grid = &test_grid;
      plug_env.file_offset = 0;
      plug_env.file_sz = mfile_get_sz( &(plug_env.file_in) );
      memcpy( &(plug_env.file_in), file_in, sizeof( mfile_t ) );

      /* Call the plugin to fill out the header. */
      memset( plugin_call_buf, '\0', RETROFLAT_PATH_MAX + 1 );
      maug_snprintf( plugin_call_buf, RETROFLAT_PATH_MAX, "%s_read",
         plug_ext );
      retval = mplug_call(
         mod_exe, plugin_call_buf, &plug_env, sizeof( plug_env ) );
      if( MERROR_OK != retval ) {
         error_printf( "%s plugin returned error: %u", plug_ext, retval );
         goto cleanup;
      }
      debug_printf( 2,
         "plugin returned parameters for: %u x %u grid with %u colors",
         test_grid.w, test_grid.h, test_grid.palette_ncolors );

      /* Create grid for layer. */
      retval = grid_pack_add_layer(
         test_grid.w, test_grid.h, test_grid.palette_ncolors, p_grid_pack_h );
      maug_cleanup_if_not_ok();

      maug_mlock( *p_grid_pack_h, grid_pack )
      maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

      plug_env.flags &= ~PERPIX_PLUG_FLAG_HEADER_ONLY;
      plug_env.grid_pack = grid_pack;
      plug_env.test_grid = NULL;

      /* Read palette and pixels. */
      memset( plugin_call_buf, '\0', RETROFLAT_PATH_MAX + 1 );
      maug_snprintf( plugin_call_buf, RETROFLAT_PATH_MAX, "%s_read",
         plug_ext );
      retval = mplug_call(
         mod_exe, plugin_call_buf, &plug_env, sizeof( plug_env ) );
      if( MERROR_OK != retval ) {
         error_printf( "%s plugin returned error: %u", plug_ext, retval );
         goto cleanup;
      }

      maug_munlock( *p_grid_pack_h, grid_pack )

      debug_printf(
         2, "successfully loaded layer %u...", plug_env.layer_idx );

      plug_env.layer_idx++;
   } while( PERPIX_PLUG_FLAG_MORE_LAYERS ==
      (PERPIX_PLUG_FLAG_MORE_LAYERS & plug_env.flags) );

cleanup:

   if( NULL != mod_exe ) {
      mplug_free( mod_exe );
   }

   return retval;
}

MERROR_RETVAL perpix_open_file(
   const char* filename, MAUG_MHANDLE* p_grid_pack_h
) {
   MERROR_RETVAL retval = MERROR_OK;
   size_t i_plug = 0;
   mfile_t file_in;

   /* TODO: Create if NULL. */
   assert( NULL != *p_grid_pack_h );

   debug_printf( 3, "opening file: %s", filename );
   retval = mfile_open_read( filename, &file_in );
   maug_cleanup_if_not_ok();

   /* Select a plugin based on the first one not to fail. */
   do {
      retval = perpix_read_file(
         gc_plug_exts[i_plug], &file_in, p_grid_pack_h );
      i_plug++;
   } while( i_plug < 2 && MERROR_OK != retval );

   if( MERROR_OK != retval ) {
      goto cleanup;
   }

cleanup:

   mfile_close( &file_in );

   return retval;
}

void perpix_loop( struct PERPIX_DATA* data ) {
   struct PERPIX_GRID_PACK* grid_pack = NULL;
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_GRID* grid = NULL;

   retroflat_draw_lock( NULL );

   /* Input */

   retval = ui_handle_input_queue( data );
   maug_cleanup_if_not_ok();

   /* Drawing */

   maug_mlock( data->grid_pack_h, grid_pack );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

   if( 0 < data->redraws ) {
      debug_printf( 2, UPRINTF_S32_FMT " redraws...", data->redraws );

      retroflat_rect(
         NULL, RETROFLAT_COLOR_BLACK, 0, 0,
         retroflat_screen_w(), retroflat_screen_h(),
         RETROFLAT_FLAGS_FILL );

      debug_printf( 1, "redrawing UI (layer %u)...", data->layer_idx );

      grid = grid_get_layer_p( grid_pack, data->layer_idx );
      if( NULL == grid ) {
         error_printf( "no grid to redraw!" );
         retval = MERROR_ALLOC;
         goto cleanup;
      }

      ui_draw_palette( grid );

      ui_draw_grid( grid );

      ui_draw_layer_icons( data, grid_pack );

      data->redraws--;
   }

cleanup:

   retroflat_draw_release( NULL );

   if( NULL != grid_pack ) {
      maug_munlock( data->grid_pack_h, grid_pack );
   }

   if( retval ) {
      retroflat_quit( retval );
   }
}

int perpix_cli_o( const char* arg, struct RETROFLAT_ARGS* args ) {
   if( 0 != strncmp( MAUG_CLI_SIGIL "o", arg, MAUG_CLI_SIGIL_SZ + 4 ) ) {
      strncpy( g_file_to_open, arg, RETROFLAT_PATH_MAX );
   }
   return RETROFLAT_OK;
}

int main( int argc, char** argv ) {
   int retval = 0;
   struct RETROFLAT_ARGS args;
   MAUG_MHANDLE data_h = (MAUG_MHANDLE)NULL;
   struct PERPIX_DATA* data = NULL;
   struct PERPIX_GRID_PACK* grid_pack = NULL;
   struct PERPIX_GRID* grid = NULL;
   
   /* === Setup === */

   logging_init();

   memset( g_file_to_open, '\0', RETROFLAT_PATH_MAX );

   memset( &args, '\0', sizeof( struct RETROFLAT_ARGS ) );

   args.title = "perpix";
   args.assets_path = "assets";

   maug_add_arg( MAUG_CLI_SIGIL "o", MAUG_CLI_SIGIL_SZ + 2,
      "Open the given image file", 0,
      (maug_cli_cb)perpix_cli_o, NULL, &args );
   
   retval = retroflat_init( argc, argv, &args );
   if( RETROFLAT_OK != retval ) {
      goto cleanup;
   }

   data_h = maug_malloc( 1, sizeof( struct PERPIX_DATA ) );
   maug_cleanup_if_null_alloc( MAUG_MHANDLE, data_h );
   maug_mlock( data_h, data );
   maug_cleanup_if_null_alloc( struct PERPIX_DATA*, data );
   maug_mzero( data, sizeof( struct PERPIX_DATA ) );

   /* Create an empty new grid_pack. */
   retval = grid_pack_new_h( &(data->grid_pack_h) );
   maug_cleanup_if_not_ok();
   maug_cleanup_if_null_alloc( MAUG_MHANDLE, data->grid_pack_h );

   /* Try to load file. */
   if( '\0' != g_file_to_open[0] ) {
      retval = perpix_open_file( g_file_to_open, &(data->grid_pack_h) );
      maug_cleanup_if_not_ok();
   }

   /* Setup screen zoom. */
   maug_mlock( data->grid_pack_h, grid_pack );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );
   grid = grid_get_layer_p( grid_pack, data->layer_idx );
   ui_scale_zoom( retroflat_screen_w(), retroflat_screen_h(), grid );
   maug_munlock( data->grid_pack_h, grid_pack );
   maug_cleanup_if_not_ok();

   /* Setup data. */
   retroflat_set_proc_resize( perpix_on_resize, data );
   ui_inc_redraws( data );

   /* === Main Loop === */

   retroflat_loop( (retroflat_loop_iter)perpix_loop, NULL, data );

cleanup:

#ifndef RETROFLAT_OS_WASM

   if( NULL != data ) {
      if( NULL != data->grid_pack_h ) {
         maug_mfree( data->grid_pack_h );
      }
      maug_munlock( data_h, data );
   }

   if( NULL != data_h ) {
      maug_mfree( data_h );
   }

   retroflat_shutdown( retval );

   logging_shutdown();

#endif /* !RETROFLAT_OS_WASM */

   return retval;
}
END_OF_MAIN()

