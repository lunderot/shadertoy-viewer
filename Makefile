all : shadertoy

MINGW32:=/usr/bin/i686-w64-mingw32

shadertoy : shadertoy.c rawdraw/CNFG.c rawdraw/chew.c
	gcc -o $@ $^  -lGL -lEGL -lm -DEGL_HEADLESS -DCNFGCONTEXTONLY

clean : 
	rm -rf *.o *~ shadertoy.exe shadertoy

