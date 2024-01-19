
#ifndef PLUGIN_H
#define PLUGIN_H

typedef MERROR_RETVAL (*plugin_proc_t)(
   struct PERPIX_GRID* grid, void* data, size_t data_sz, void* opts );

#ifdef RETROFLAT_OS_WIN
typedef HMODULE plugin_mod_t;
#else
typedef void* plugin_mod_t;
#endif /* RETROFLAT_OS_WIN */

MERROR_RETVAL plugin_load(
   const char* plugin_path, plugin_mod_t* p_mod_exe );

MERROR_RETVAL plugin_call(
   plugin_mod_t mod_exe, const char* proc_name,
   struct PERPIX_GRID* grid, void* data, size_t data_sz, void* opts );

#endif /* !PLUGIN_H */

