
#include "perpix.h"

MERROR_RETVAL grid_pack_new_h( MAUG_MHANDLE* p_grid_pack_h ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_GRID_PACK* grid_pack = NULL;

   debug_printf( 1, "creating empty grid pack (" SIZE_T_FMT " bytes)...",
      sizeof( struct PERPIX_GRID_PACK ) );

   /* Allocate new grid. */
   *p_grid_pack_h = maug_malloc( 1, sizeof( struct PERPIX_GRID_PACK ) );
   maug_cleanup_if_null_alloc( MAUG_MHANDLE, *p_grid_pack_h );

   maug_mlock( *p_grid_pack_h, grid_pack );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

   maug_mzero( grid_pack, sizeof( struct PERPIX_GRID_PACK ) );

   /* Setup header. */
   grid_pack->version = 1;
   grid_pack->sz = sizeof( struct PERPIX_GRID_PACK );
   grid_pack->count = 0;

cleanup:

   if( NULL != grid_pack ) {
      maug_munlock( *p_grid_pack_h, grid_pack );
   }

   if( retval && NULL != *p_grid_pack_h ) {
      error_printf( "cleaning up faulty grid..." );
      maug_mfree( *p_grid_pack_h );
      *p_grid_pack_h = NULL;
   }

   return retval;
}

MERROR_RETVAL grid_pack_add_layer(
   uint32_t w, uint32_t h, uint32_t pal_ncolors, MAUG_MHANDLE* p_grid_pack_h
) {
   struct PERPIX_GRID_PACK* grid_pack = NULL;
   MAUG_MHANDLE grid_pack_new_h = NULL;
   MERROR_RETVAL retval = MERROR_OK;
   size_t grid_new_sz = 0;
   size_t grid_pack_new_sz = 0;
   size_t pal_sz = sizeof( uint32_t ) * pal_ncolors;
   uint32_t* p_palette = NULL;
   struct PERPIX_GRID* grid_new = NULL;

   maug_mlock( *p_grid_pack_h, grid_pack );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

   grid_new_sz =
      sizeof( struct PERPIX_GRID ) + /* Header */
      (sizeof( uint32_t ) * pal_ncolors) + /* Palette */
      (w * h); /* Pixels */
   grid_pack_new_sz = grid_pack->sz + grid_new_sz;

   maug_munlock( *p_grid_pack_h, grid_pack );
   
   debug_printf( 2,
      "adding %ux%u-pixel %u-color layer (" SIZE_T_FMT
      " bytes) to grid pack...", w, h, pal_ncolors, grid_new_sz );

   /* Allocate space for the new layer. */
   grid_pack_new_h = maug_mrealloc( *p_grid_pack_h, 1, grid_pack_new_sz );
   maug_cleanup_if_null_alloc( MAUG_MHANDLE, grid_pack_new_h );
   *p_grid_pack_h = grid_pack_new_h;

   maug_mlock( *p_grid_pack_h, grid_pack );
   maug_cleanup_if_null_alloc( struct PERPIX_GRID_PACK*, grid_pack );

   /* Configure the grid pack to account for new layer. */
   grid_pack->sz = grid_pack_new_sz;
   grid_pack->count++;

   grid_new = grid_get_layer_p( grid_pack, grid_pack->count - 1 );

   maug_mzero( grid_new, grid_new_sz );

   grid_new->version = 1;
   grid_new->sz = grid_new_sz;
   grid_new->data_sz = w * h;
   grid_new->w = w;
   grid_new->h = h;
   grid_new->palette_ncolors = pal_ncolors;
   grid_new->palette_offset = sizeof( struct PERPIX_GRID );
   grid_new->px_offset = sizeof( struct PERPIX_GRID ) + pal_sz;

   /* Setup palette. */
   debug_printf( 1, "setting up grid palette..." );

   p_palette = grid_palette( grid_new );

   #define GRID_PAL_FILL( idx, name, val ) \
      assert( idx < pal_ncolors ); \
      p_palette[idx] = val;

   PERPIX_DEF_COLOR_TABLE( GRID_PAL_FILL );

cleanup:

   if( NULL != grid_pack ) {
      maug_munlock( *p_grid_pack_h, grid_pack );
   }

   return retval;
}

