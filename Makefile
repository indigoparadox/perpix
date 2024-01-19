
# vim: ft=make noexpandtab

#WING := 1
RETROFLAT_DOS_MEM_LARGE := 1
C_FILES := src/main.c src/ui.c src/grid.c
GLOBAL_DEFINES := -DRETROFLAT_DEFAULT_SCREEN_W=640 -DRETROFLAT_DEFAULT_SCREEN_H=480

include maug/Makefile.inc

# Target-specific options.
.PHONY: clean

all: perpix.ale perpix.sdl perpixd.exe perpixw.exe perpixnt.exe perpix.html

# Unix (Allegro)

$(eval $(call TGTUNIXALE,perpix))

# Unix (SDL)

$(eval $(call TGTUNIXSDL,perpix))

# WASM

$(eval $(call TGTWASMSDL,perpix))

# DOS

$(eval $(call TGTDOSALE,perpix))

$(eval $(call TGTDOSBIOS,perpix))

# WinNT

$(eval $(call TGTWINNT,perpix))

$(eval $(call TGTWINSDL,perpix))

# Win386

$(eval $(call TGTWIN16,perpix))

$(eval $(call TGTWIN386,perpix))

# Clean

clean:
	rm -rf $(CLEAN_TARGETS)

