all : shadertoy

MINGW32:=/usr/bin/i686-w64-mingw32

shadertoy : shadertoy.c rawdraw/CNFG.c rawdraw/chew.c
	gcc -o $@ $^  -lX11 -lXinerama -lGL   -DCNFGOGL -lm

clean : 
	rm -rf *.o *~ shadertoy.exe shadertoy

