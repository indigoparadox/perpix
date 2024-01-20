
#define MAUG_C
#include <perpix.h>

MPLUG_EXPORT MERROR_RETVAL ico_read( struct PERPIX_PLUG_ENV* plug_env ) {
}

MPLUG_EXPORT MERROR_RETVAL ico_read( struct PERPIX_PLUG_ENV* plug_env ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_PLUG_ENV bmp_env;
   mplug_mod_t mod_exe = NULL;
   uint32_t bmp_offset = 0; /* TODO */
   uint16_t ico_field_res = 0,
      ico_field_type = 0,
      ico_field_num_imgs = 0;

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

   ico_field_num_imgs = perpix_read_lsbf_16( plug_env->buf, 4 );
   debug_printf( 3, "found %u icons in file...", ico_field_num_imgs );

   /* TODO: Select an image. */

   retval = mplug_load( "./perpix_bmp", &mod_exe );
   maug_cleanup_if_not_ok();

   bmp_env.grid = plug_env->grid;
   bmp_env.buf = &(plug_env->buf[bmp_offset]);
   bmp_env.buf_sz = plug_env->buf_sz - bmp_offset;
   retval = mplug_call(
      mod_exe, "bmp_read_header", &plug_env, sizeof( plug_env ) );
   if( MERROR_OK != retval ) {
      error_printf( "plugin returned error: %u", retval );
   }

cleanup:

   if( NULL != mod_exe ) {
      mplug_free( mod_exe );
   }

   return retval;
}

