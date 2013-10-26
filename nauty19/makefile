CFLAGS=-c -O
SMALL=-DMAXN=WORDSIZE
MEDIUM=-DMAXN=1024
LARGE=-DMAXN=8192

dreadnaut: dreadnaut.o naututil.o nauty.o nautil.o nautinv.o
	cc -o dreadnaut dreadnaut.o naututil.o nauty.o nautil.o nautinv.o
dreadnaut.o: nauty.h naututil.h dreadnaut.c
	cc ${CFLAGS} ${MEDIUM} dreadnaut.c
naututil.o: nauty.h naututil.h naututil.c
	cc ${CFLAGS} ${MEDIUM} naututil.c
nautil.o: nauty.h nautil.c
	cc ${CFLAGS} ${MEDIUM} nautil.c
nauty.o: nauty.h nauty.c
	cc ${CFLAGS} ${MEDIUM} nauty.c
nautinv.o: nauty.h naututil.h nautinv.c
	cc ${CFLAGS} ${MEDIUM} nautinv.c
nautaux.o: nautaux.h nauty.h naututil.h nautaux.c
	cc ${CFLAGS} ${MEDIUM} nautaux.c

dreadnaut1: dreadnaut1.o naututil1.o nauty1.o nautil1.o nautinv1.o
	cc -o dreadnaut1 dreadnaut1.o naututil1.o nauty1.o nautil1.o nautinv1.o
dreadnaut1.o: nauty.h naututil.h dreadnaut.c
	cc ${CFLAGS} ${SMALL} -o dreadnaut1.o dreadnaut.c
naututil1.o: nauty.h naututil.h naututil.c
	cc ${CFLAGS} ${SMALL} -o naututil1.o naututil.c
nautil1.o: nauty.h nautil.c
	cc ${CFLAGS} ${SMALL} -o nautil1.o nautil.c
nauty1.o: nauty.h nauty.c
	cc ${CFLAGS} ${SMALL} -o nauty1.o nauty.c
nautinv1.o: nauty.h naututil.h nautinv.c
	cc ${CFLAGS} ${SMALL} -o nautinv1.o nautinv.c
nautaux1.o: nautaux.h nauty.h naututil.h nautaux.c
	cc ${CFLAGS} ${SMALL} -o nautaux1.o nautaux.c

dreadnautx: dreadnautx.o naututilx.o nautyx.o nautilx.o nautinvx.o
	cc -o dreadnautx dreadnautx.o naututilx.o nautyx.o nautilx.o nautinvx.o
dreadnautx.o: nauty.h naututil.h dreadnaut.c
	cc ${CFLAGS} ${LARGE} -o dreadnautx.o dreadnaut.c
naututilx.o: nauty.h naututil.h naututil.c
	cc ${CFLAGS} ${LARGE} -o naututilx.o naututil.c
nautilx.o: nauty.h nautil.c
	cc ${CFLAGS} ${LARGE} -o nautilx.o nautil.c
nautyx.o: nauty.h nauty.c
	cc ${CFLAGS} ${LARGE} -o nautyx.o nauty.c
nautinvx.o: nauty.h naututil.h nautinv.c
	cc ${CFLAGS} ${LARGE} -o nautinvx.o nautinv.c
nautauxx.o: nautaux.h nauty.h naututil.h nautaux.c
	cc ${CFLAGS} ${LARGE} -o nautauxx.o nautaux.c

rancubic.o : nauty.h rancubic.c
	cc ${CFLAGS} rancubic.c

cif3.o : nauty.h naututil.h cif3.c
	cc ${CFLAGS} cif3.c
cif3 : cif3.o rancubic.o naututil.o nautilx.o
	cc -o cif3 cif3.o rancubic.o naututilx.o nautilx.o

ranc : ranc.c rancubic.o nauty.h
	cc -o ranc -O ranc.c rancubic.o

makeg.c : ../r44/makeg.c
	cp ../r44/makeg.c .

makebg.c : ../r44/makebg.c
	cp ../r44/makebg.c .

makeg.o:        naututil.h nauty.h makeg.c
	cc -c -O makeg.c
 
makeg:  makeg.o nauty1.o nautil1.o
	cc -o makeg makeg.o nauty1.o nautil1.o
 
makebg.o:       naututil.h nauty.h makebg.c
	cc -c -O makebg.c
 
makebg:  makebg.o nauty.o nautil.o
	cc -o makebg makebg.o nauty1.o nautil1.o
 
archive : makeg.c makebg.c
	tar cvf - dreadnaut.c nautaux.h nauty.h makebg.c nautil.c \
                  nautyex.c makefile nautinv.c nug.alw makefile_lattice \
                  naututil.c nautyex.c read.me makeg.c naututil.h nautaux.c \
                  nauty.c | compress >nauty19p.tar.Z
