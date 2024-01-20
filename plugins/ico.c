
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
   size_t icoentry_offset = 
      ICO_FILE_HEADER_SZ + (plug_env->layer_idx * ICO_ENTRY_HEADER_SZ);
   uint32_t bmp_offset = 0;

   bmp_offset = perpix_read_lsbf_32( plug_env->buf,
      icoentry_offset + ICOENTRY_OFFSET_BMP_OFFSET );
   /* ico_entry->bmp_sz = icotools_read_u32( plug_env->buf,
      icoentry_offset + ICOENTRY_OFFSET_BMP_SZ ); */

   plug_env->buf = &(plug_env->buf[bmp_offset]);

   /* TODO: Base this on ICOENTRY_OFFSET_BMP_SZ. */
   plug_env->buf_sz = plug_env->buf_sz - bmp_offset;
}

MPLUG_EXPORT MERROR_RETVAL ico_layers( struct PERPIX_PLUG_ENV* plug_env ) {
   return perpix_read_lsbf_16( plug_env->buf, 4 );
}

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
   uint32_t bmp_offset = 0; /* TODO */
   uint16_t ico_field_res = 0,
      ico_field_type = 0,
      ico_field_num_imgs = 0;
   uint32_t i = 0;

   ico_field_res = perpix_read_lsbf_16( plug_env->buf, 0 );
   if( 0 != ico_field_res ) {
      error_printf( "invalid: field reserved: %u", ico_field_res );
      retval = MERROR_FILE;
      goto cleanup;
   }

   ico_field_type = perpix_read_lsbf_16( plug_env->buf, 2 );
   if( 1 != ico_field_type ) {
      error_printf( "invalid: field type: %u", ico_field_type );
      retval = MERROR_FILE;
      goto cleanup;
   }

   ico_field_num_imgs = ico_layers( plug_env );
   debug_printf( 3, "found %u icons in file...", ico_field_num_imgs );

   /* TODO: Select an image. */

   retval = mplug_load( "./perpix_bmp", &mod_exe );
   maug_cleanup_if_not_ok();

   for( i = 0 ; ico_field_num_imgs > i ; i++ ) {
      if( i >= plug_env->grid_pack->count ) {
         error_printf( "grid has insufficient layers: " UPRINTF_U32_FMT,
            plug_env->grid_pack->count );
         retval = MERROR_OVERFLOW;
         goto cleanup;
      }
      memcpy( &bmp_env, plug_env, sizeof( struct PERPIX_PLUG_ENV ) );
      bmp_env.layer_idx = i;

      /* Get bitmap offset, etc from ICO entry. */
      ico_read_entry( &bmp_env );

      /* Pass icon XOR mask to bitmap reader. */
      retval = mplug_call(
         mod_exe, "bmp_read_info_header", &bmp_env, sizeof( bmp_env ) );
      if( MERROR_OK != retval ) {
         error_printf( "plugin returned error: %u", retval );
         goto cleanup;
      }
   }

cleanup:

   if( NULL != mod_exe ) {
      mplug_free( mod_exe );
   }

   return retval;
}

