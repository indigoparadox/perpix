
#define MAUG_C
#include <perpix.h>

#define ICO_FILE_HEADER_SZ 6
#define ICO_ENTRY_HEADER_SZ 16

#define ICOENTRY_OFFSET_WIDTH 0
#define ICOENTRY_OFFSET_HEIGHT 1
#define ICOENTRY_OFFSET_PAL_SZ 2
#define ICOENTRY_OFFSET_COLOR_PLANES 4
#define ICOENTRY_OFFSET_BPP 6
#define ICOENTRY_OFFSET_BMP_SZ 8
#define ICOENTRY_OFFSET_BMP_OFFSET 12

MPLUG_EXPORT MERROR_RETVAL ico_read_entry(
   struct PERPIX_PLUG_ENV* plug_env
) {
   MERROR_RETVAL retval = MERROR_OK;
   size_t icoentry_offset = 
      ICO_FILE_HEADER_SZ + (plug_env->layer_idx * ICO_ENTRY_HEADER_SZ);
   uint32_t bmp_offset = 0;
   uint32_t bmp_sz = 0;
   uint16_t bmp_bpp = 0;

   /* bmp_offset = perpix_read_lsbf_32( plug_env->buf,
      icoentry_offset + ICOENTRY_OFFSET_BMP_OFFSET );
   bmp_sz = perpix_read_lsbf_32( plug_env->buf,
      icoentry_offset + ICOENTRY_OFFSET_BMP_SZ ); */
   mfile_u32read_lsbf_at( &(plug_env->file_in), &bmp_offset,
      plug_env->file_offset + icoentry_offset + 
         ICOENTRY_OFFSET_BMP_OFFSET );
   mfile_u32read_lsbf_at( &(plug_env->file_in), &bmp_sz,
      plug_env->file_offset + icoentry_offset + ICOENTRY_OFFSET_BMP_SZ );
   if( bmp_offset + bmp_sz > plug_env->file_sz ) {
      error_printf(
         "bitmap data offset is beyond file ending! (" UPRINTF_U32_FMT
         " bytes at " UPRINTF_U32_FMT ", beyond " UPRINTF_U32_FMT " bytes)",
         bmp_sz, bmp_offset, plug_env->file_sz );
      retval = MERROR_OVERFLOW;
      goto cleanup;
   }

   /* bmp_bpp = perpix_read_lsbf_16( plug_env->buf,
      icoentry_offset + ICOENTRY_OFFSET_BPP ); */
   mfile_u16read_lsbf_at( &(plug_env->file_in), &bmp_bpp,
      plug_env->file_offset + icoentry_offset + ICOENTRY_OFFSET_BPP );
   if( 8 < bmp_bpp ) {
      error_printf( "bitmap bpp is too high: %u", bmp_bpp );
      retval = MERROR_OVERFLOW;
      goto cleanup;
   }

   debug_printf( 1,
      "bitmap data is located at: %u bytes, %u bytes long",
      bmp_offset, bmp_sz );

   /* plug_env->buf = &(plug_env->buf[bmp_offset]);
   plug_env->buf_sz = bmp_sz; */
   plug_env->file_offset += bmp_offset;
   plug_env->file_sz -= bmp_sz;

cleanup:
   return retval;
}

/*
MPLUG_EXPORT MERROR_RETVAL ico_layers( struct PERPIX_PLUG_ENV* plug_env ) {
   return perpix_read_lsbf_16( plug_env->buf, 4 );
}
*/

#if 0
MPLUG_EXPORT MERROR_RETVAL ico_layer_sz( struct PERPIX_PLUG_ENV* plug_env ) {
   struct PERPIX_PLUG_ENV bmp_env;
   MERROR_RETVAL retval = MERROR_OK;
   mplug_mod_t mod_exe = NULL;

   memcpy( &bmp_env, plug_env, sizeof( struct PERPIX_PLUG_ENV ) );
   retval = ico_read_entry( &bmp_env );
   maug_cleanup_if_not_ok();

   retval = mplug_load( "./perpix_bmp", &mod_exe );
   maug_cleanup_if_not_ok();

   retval = mplug_call(
      mod_exe, "bmp_read_info_header", &bmp_env, sizeof( bmp_env ) );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
      goto cleanup;
   }

   plug_env->layer_sz =

cleanup:

   if( NULL != mod_exe ) {
      mplug_free( mod_exe );
   }

   return retval;
}
#endif

MPLUG_EXPORT MERROR_RETVAL ico_read( struct PERPIX_PLUG_ENV* plug_env ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_PLUG_ENV bmp_env;
   mplug_mod_t mod_exe = NULL;
   uint16_t ico_field_res = 0,
      ico_field_type = 0,
      ico_field_num_imgs = 0;
   struct PERPIX_GRID* p_grid = NULL;

   debug_printf( 3, "started reading at offset " UPRINTF_U32_FMT "...",
      plug_env->file_offset );

   mfile_u16read_lsbf_at( &(plug_env->file_in), &ico_field_res,
      plug_env->file_offset );
   if( 0 != ico_field_res ) {
      error_printf( "invalid: field reserved: %u", ico_field_res );
      retval = MERROR_FILE;
      goto cleanup;
   }

   mfile_u16read_lsbf_at( &(plug_env->file_in), &ico_field_type,
      plug_env->file_offset + 2 );
   if( 1 != ico_field_type ) {
      error_printf( "invalid: field type: %u", ico_field_type );
      retval = MERROR_FILE;
      goto cleanup;
   }

   /* Figure out the number of layers remaining. */
   mfile_u16read_lsbf_at( &(plug_env->file_in), &ico_field_num_imgs,
      plug_env->file_offset + 4 );
   debug_printf( 2, "found %u icons in file...", ico_field_num_imgs );
   if( ico_field_num_imgs > plug_env->layer_idx + 1 ) {
      debug_printf( 2, "processing layer: %u, more to follow...",
         plug_env->layer_idx );
      plug_env->flags |= PERPIX_PLUG_FLAG_MORE_LAYERS;
   } else {
      debug_printf( 2, "processing final layer: %u", plug_env->layer_idx );
      plug_env->flags &= ~PERPIX_PLUG_FLAG_MORE_LAYERS;
   }

   /* TODO: Select an image. */

   retval = mplug_load( "./perpix_bmp", &mod_exe );
   maug_cleanup_if_not_ok();

   memcpy( &bmp_env, plug_env, sizeof( struct PERPIX_PLUG_ENV ) );

   /* Get bitmap offset, etc from ICO entry. */
   retval = ico_read_entry( &bmp_env );
   maug_cleanup_if_not_ok();

   /* Pass icon XOR mask to bitmap reader. */
   retval = mplug_call(
      mod_exe, "bmp_read_info_header", &bmp_env, sizeof( bmp_env ) );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
      goto cleanup;
   }

   /* If this is the test pass, cut height in half. */
   if( NULL != plug_env->test_grid ) {
      plug_env->test_grid->h /= 2;
      goto cleanup;
   }

   /* This isn't the test pass, so read the image. */
   p_grid = grid_get_layer_p( bmp_env.grid_pack, bmp_env.layer_idx );
   debug_printf( 2, "reading layer " UPRINTF_U32_FMT " palette data...",
      bmp_env.layer_idx );
   /* bmp_env.buf = &(bmp_env.buf[40]); */
   bmp_env.file_offset += 40;
   retval = mplug_call(
      mod_exe, "bmp_read_palette", &bmp_env, sizeof( bmp_env ) );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
      goto cleanup;
   }

   p_grid = grid_get_layer_p( bmp_env.grid_pack, bmp_env.layer_idx );
   /* bmp_env.buf = &(bmp_env.buf[p_grid->palette_ncolors * 4]); */
   /* TODO: Figure out why ICO parser messes up size. */
   bmp_env.file_offset += p_grid->palette_ncolors * 4;
   debug_printf( 2, "reading layer " UPRINTF_U32_FMT " pixel data...",
      bmp_env.layer_idx );
   retval = mplug_call(
      mod_exe, "bmp_read_px", &bmp_env, sizeof( bmp_env ) );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
      goto cleanup;
   }

   debug_printf( 2, "layer load completed successfully!" );

cleanup:

   if( NULL != mod_exe ) {
      mplug_free( mod_exe );
   }

   return retval;
}

