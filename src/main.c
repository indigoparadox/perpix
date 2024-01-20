
#define MAUG_C
#define RETROFLT_C
#define RETROFIL_C
#include "perpix.h"

char g_file_to_open[RETROFLAT_PATH_MAX];

MERROR_RETVAL perpix_on_resize( uint16_t new_w, uint16_t new_h, void* data ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_DATA* data_p = (struct PERPIX_DATA*)data;
   struct PERPIX_GRID* grid = NULL;

   /* Resize the virtual buffer and redraw the UI. */

   retroflat_resize_v();

   maug_mlock( data_p->grid_h, grid )
   ui_scale_zoom( new_w, new_h, grid );
   maug_munlock( data_p->grid_h, grid )

   data_p->flags |= PERPIX_FLAG_REDRAW_UI;

   return retval;
}

MERROR_RETVAL perpix_open_file(
   const char* filename, struct PERPIX_GRID* grid
) {
   MERROR_RETVAL retval = MERROR_OK;
   mplug_mod_t mod_exe = NULL;
   MAUG_MHANDLE in_file_bytes_h = NULL;
   uint8_t* in_file_bytes = NULL;
   size_t in_file_sz = 0;
   struct PERPIX_PLUG_ENV plug_env;

   assert( NULL != grid );

   debug_printf( 3, "opening file: %s", filename );
   retval = retrofil_open_read( filename, &in_file_bytes_h, &in_file_sz );
   maug_cleanup_if_not_ok();

   retval = mplug_load( "./perpix_bmp", &mod_exe );
   maug_cleanup_if_not_ok();

   maug_mlock( in_file_bytes_h, in_file_bytes );
   maug_cleanup_if_null_alloc( uint8_t*, in_file_bytes );

   plug_env.grid = grid;
   plug_env.buf = in_file_bytes;
   plug_env.buf_sz = in_file_sz;
   retval = mplug_call( mod_exe, "bmp_read", &plug_env, sizeof( plug_env ) );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
   }

cleanup:

   if( NULL != in_file_bytes ) {
      maug_munlock( in_file_bytes_h, in_file_bytes );
   }

   if( NULL != in_file_bytes_h ) {
      retrofil_close_read( in_file_bytes, in_file_sz );
   }

   return retval;
}

void perpix_loop( struct PERPIX_DATA* data ) {
   struct PERPIX_GRID* grid = NULL;
   MERROR_RETVAL retval = MERROR_OK;

   retroflat_draw_lock( NULL );

   /* Input */

   retval = ui_handle_input_queue( data );
   maug_cleanup_if_not_ok();

   /* Drawing */

   maug_mlock( data->grid_h, grid );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID*, grid );

   if( PERPIX_FLAG_REDRAW_UI == (data->flags & PERPIX_FLAG_REDRAW_UI) ) {
      retroflat_rect(
         NULL, RETROFLAT_COLOR_BLACK, 0, 0,
         retroflat_screen_w(), retroflat_screen_h(),
         RETROFLAT_FLAGS_FILL );

      ui_draw_palette( grid );

      ui_draw_grid( grid );

      data->flags &= ~PERPIX_FLAG_REDRAW_UI;
   }

cleanup:

   retroflat_draw_release( NULL );

   if( NULL != grid ) {
      maug_munlock( data->grid_h, grid );
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

   /* Create an empty new grid. */
   retval = grid_new_h( 16, 16, 16, &(data->grid_h) );
   maug_cleanup_if_not_ok();
   maug_cleanup_if_null_alloc( MAUG_MHANDLE, data->grid_h );

   /* Try to load file. */
   maug_mlock( data->grid_h, grid );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID*, grid );
   if( '\0' != g_file_to_open[0] ) {
      retval = perpix_open_file( g_file_to_open, grid );
   }
   ui_scale_zoom( retroflat_screen_w(), retroflat_screen_h(), grid );
   maug_munlock( data->grid_h, grid );
   maug_cleanup_if_not_ok();

   /* Setup data. */
   retroflat_set_proc_resize( perpix_on_resize, data );
   data->flags |= PERPIX_FLAG_REDRAW_UI;

   /* === Main Loop === */

   retroflat_loop( (retroflat_loop_iter)perpix_loop, NULL, data );

cleanup:

#ifndef RETROFLAT_OS_WASM

   if( NULL != data ) {
      if( NULL != data->grid_h ) {
         maug_mfree( data->grid_h );
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

