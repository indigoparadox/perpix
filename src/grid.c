
#include "perpix.h"

MERROR_RETVAL grid_new_h(
   uint32_t w, uint32_t h, uint32_t pal_ncolors, MAUG_MHANDLE* p_grid_h
) {
   MERROR_RETVAL retval = MERROR_OK;
   size_t grid_sz = sizeof( struct PERPIX_GRID );
   uint32_t* p_palette = NULL;
   struct PERPIX_GRID* grid = NULL;
   size_t pal_sz = sizeof( uint32_t ) * pal_ncolors;
   size_t px_sz = w * h;

   debug_printf( 1, "creating grid (" SIZE_T_FMT " bytes)...",
      grid_sz + pal_sz + px_sz );

   /* Allocate new grid. */
   *p_grid_h = maug_malloc( 1, grid_sz + pal_sz + px_sz );
   maug_cleanup_if_null_alloc( MAUG_MHANDLE, *p_grid_h );

   maug_mlock( *p_grid_h, grid );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID*, grid );

   maug_mzero( grid, grid_sz + pal_sz + px_sz );

   /* Setup header. */
   grid->version = 1;
   grid->w = w;
   grid->h = h;
   grid->palette_ncolors = pal_ncolors;
   grid->palette_offset = sizeof( struct PERPIX_GRID );
   grid->px_offset = sizeof( struct PERPIX_GRID ) + pal_sz;

   /* Setup palette. */
   debug_printf( 1, "setting up grid palette..." );

   p_palette = grid_palette( grid );

   #define GRID_PAL_FILL( idx, name, val ) \
      assert( idx < pal_ncolors ); \
      p_palette[idx] = val;

   PERPIX_DEF_COLOR_TABLE( GRID_PAL_FILL );

cleanup:

   if( NULL != grid ) {
      maug_munlock( *p_grid_h, grid );
   }

   if( retval && NULL != *p_grid_h ) {
      error_printf( "cleaning up faulty grid..." );
      maug_mfree( *p_grid_h );
      *p_grid_h = NULL;
   }

   return retval;
}

