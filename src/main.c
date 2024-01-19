
#define MAUG_C
#define RETROFLT_C
#include "perpix.h"
#include <fcntl.h> /* open() */
#include <sys/mman.h> /* mmap() */
#include <sys/stat.h> /* fstat() */
#include <unistd.h> /* close() */

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
   plugin_mod_t mod_exe = NULL;
   int in_file = 0;
   uint8_t* in_file_bytes = NULL;
   struct stat st;

   assert( NULL != grid );

   in_file = open( filename, O_RDONLY );
   if( 0 >= in_file ) {
      error_printf( "could not open file: %s", filename );
      retval = MERROR_FILE;
      goto cleanup;
   }

   fstat( in_file, &st );
   in_file_bytes = 
      mmap( (caddr_t)0, st.st_size, PROT_READ, MAP_SHARED,  in_file, 0 );

   assert( NULL != in_file_bytes );

   retval = plugin_load( "./perpix_bmp.so", &mod_exe );
   maug_cleanup_if_not_ok();

   retval = plugin_call(
      mod_exe, "bmp_read", grid, in_file_bytes, st.st_size, NULL );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
   }

cleanup:

   if( NULL != in_file_bytes ) {
      munmap( in_file_bytes, st.st_size );
   }

   if( 0 < in_file ) {
      close( in_file );
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

int main( int argc, char** argv ) {
   int retval = 0;
   struct RETROFLAT_ARGS args;
   MAUG_MHANDLE data_h = (MAUG_MHANDLE)NULL;
   struct PERPIX_DATA* data = NULL;
   struct PERPIX_GRID* grid = NULL;
   
   /* === Setup === */

   logging_init();

   memset( &args, '\0', sizeof( struct RETROFLAT_ARGS ) );

   args.title = "perpix";
   args.assets_path = "assets";
   
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
   retval = perpix_open_file( "an2.bmp", grid );
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

