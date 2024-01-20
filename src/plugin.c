
#include "perpix.h"

#ifdef RETROFLAT_OS_WIN
#else
#  include <dlfcn.h>
#endif /* RETROFLAT_OS_WIN */

MERROR_RETVAL plugin_load(
   const char* plugin_basename, plugin_mod_t* p_mod_exe
) {
   MERROR_RETVAL retval = MERROR_OK;
   char plugin_path[RETROFLAT_PATH_MAX + 1];
#ifdef RETROFLAT_OS_WIN
   size_t i = 0;
#endif /* RETROFLAT_OS_WIN */

   memset( plugin_path, '\0', RETROFLAT_PATH_MAX + 1 );

#if defined( RETROFLAT_OS_UNIX )
   maug_snprintf( plugin_path, RETROFLAT_PATH_MAX, "%s.so", plugin_basename );
   *p_mod_exe = dlopen( plugin_path, RTLD_LAZY );
#elif defined( RETROFLAT_OS_WIN )
   maug_snprintf(
      plugin_path, RETROFLAT_PATH_MAX, "%s.dll", plugin_basename );
   for( i = 0 ; RETROFLAT_PATH_MAX > i ; i++ ) {
      if( '/' == plugin_path[i] ) {
         plugin_path[i] = '\\';
      }
   }
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
   char proc_name_ex[RETROFLAT_PATH_MAX + 1];
#endif /* RETROFLAT_OS_WIN */

#ifdef RETROFLAT_OS_UNIX
   plugin_proc = dlsym( mod_exe, proc_name );
#elif defined( RETROFLAT_OS_WIN )
   memset( proc_name_ex, '\0', RETROFLAT_PATH_MAX + 1 );

   /* Append a _ to the proc_name to match calling convention name scheme. */
   maug_snprintf( proc_name_ex, RETROFLAT_PATH_MAX, "%s_", proc_name );
   plugin_proc = (plugin_proc_t)GetProcAddress( mod_exe, proc_name_ex );
#else
#  error "dlsym undefined!"
#endif

   if( (plugin_proc_t)NULL == plugin_proc ) {
      error_printf( "unable to load proc: %s", proc_name );
      retval = MERROR_FILE;
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

