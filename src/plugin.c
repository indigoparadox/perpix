
#include "perpix.h"

#ifdef RETROFLAT_OS_WIN
#else
#  include <dlfcn.h>
#endif /* RETROFLAT_OS_WIN */

MERROR_RETVAL plugin_load(
   const char* plugin_path, plugin_mod_t* p_mod_exe
) {
   MERROR_RETVAL retval = MERROR_OK;

#if defined( RETROFLAT_OS_UNIX )
   *p_mod_exe = dlopen( plugin_path, RTLD_LAZY );
#elif defined( RETROFLAT_OS_WIN )
   *p_mod_exe = LoadLibrary( plugin_path );
#else
#  error "dlopen undefined!"
#endif /* RETROFLAT_OS_UNIX */

   if( NULL == *p_mod_exe ) {
      error_printf( "unable to load module: %s", plugin_path );
      retval = MERROR_FILE;
   }

   return retval;
}

MERROR_RETVAL plugin_call(
   plugin_mod_t mod_exe, const char* proc_name,
   struct PERPIX_GRID* grid, void* data, size_t data_sz, void* opts
) {
   MERROR_RETVAL retval = MERROR_OK;
   plugin_proc_t plugin_proc = (plugin_proc_t)NULL;
#ifdef RETROFLAT_OS_WIN
   char proc_name_ex[256];
#endif /* RETROFLAT_OS_WIN */

#ifdef RETROFLAT_OS_UNIX
   plugin_proc = dlsym( mod_exe, proc_name );
#elif defined( RETROFLAT_OS_WIN )
   /* Append a _ to the proc_name because Watcom? Windows? */
   maug_snprintf( proc_name_ex, 255, "%s_", proc_name );
   plugin_proc = (plugin_proc_t)GetProcAddress( mod_exe, proc_name_ex );
#else
#  error "dlsym undefined!"
#endif

   if( (plugin_proc_t)NULL == plugin_proc ) {
      goto cleanup;
   }

   retval = plugin_proc( grid, data, data_sz, opts );

cleanup:
   return retval;
}

void plugin_free( plugin_mod_t mod_exe ) {
#ifdef RETROFLAT_OS_UNIX
   dlclose( mod_exe );
#elif defined( RETROFLAT_OS_WIN )
   FreeLibrary( mod_exe );
#else
#  error "dlclose undefined!"
#endif /* RETROFLAT_OS_UNIX || RETROFLAT_OS_WIN */
}

