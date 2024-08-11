
# vim: ft=make noexpandtab

#WING := 1
RETROFLAT_DOS_MEM_LARGE := 1
C_FILES := src/main.c src/ui.c src/grid.c
GLOBAL_DEFINES := -DRETROFLAT_DEFAULT_SCREEN_W=640 -DRETROFLAT_DEFAULT_SCREEN_H=480 -Isrc
LDFLAGS_GCC_UNIX := -ldl
PLUGIN_DEFINES := -DDEBUG_LOG -DDEBUG_THRESHOLD=1 -DPERPIX_PLUGIN -DMAUG_NO_RETRO -DMFMT_TRACE_BMP_LVL=1
C_MPLUG_OVLS := plugins/bmp.c

include maug/Makefile.inc

# Target-specific options.
.PHONY: clean

all: perpix.ale perpix.sdl perpixd.exe perpixw.exe perpixnt.exe perpix.html perpix_bmp.so perpix_ico.so

# Unix (Allegro)

$(eval $(call TGTUNIXALE,perpix))

# Unix (SDL)

$(eval $(call TGTUNIXSDL,perpix))

# WASM

$(eval $(call TGTWASMSDL,perpix))

# DOS

$(eval $(call TGTDOSALE,perpix))

$(eval $(call TGTDOSBIOS,perpix))

$(eval $(call TGTDOSGAL,perpix))

# WinNT

$(eval $(call TGTWINNT,perpix))

$(eval $(call TGTWINSDL,perpix))

# Win386

$(eval $(call TGTWIN16,perpix))

$(eval $(call TGTWIN386,perpix))

# Plugins

$(eval $(call TGT_WATCOM_WIN32_PLUG,perpix_bmp,plugins/bmp.c))

$(eval $(call TGT_WATCOM_WIN32_PLUG,perpix_ico,plugins/ico.c))

$(eval $(call TGT_GCC_UNIX_PLUG,perpix_bmp,plugins/bmp.c))

$(eval $(call TGT_GCC_UNIX_PLUG,perpix_ico,plugins/ico.c))

# Clean

clean:
	rm -rf $(CLEAN_TARGETS)

