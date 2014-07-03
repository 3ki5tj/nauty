CC = icc -Wall -Wremarks
LM =

CC_L = icc -Wall -Wremarks -DLDBL
LM_L =

CC_Q = gcc -O3 -Wall -Wextra -DF128
LM_Q = -lquadmath

CC_MP = icc -Wall -Wremarks -DMP
LM_MP = -lgmp

exe = gaussf gcomplx
exe_l = $(patsubst %, %_l, $(exe))
exe_q = $(patsubst %, %_q, $(exe))
exe_mp = $(patsubst %, %_mp, $(exe))

dep = nau0s.h Makefile

default: $(exe)

all: $(exe) $(exe_l) $(exe_q) $(exe_mp)

$(exe): %: bcgen.c %.h $(dep)
	$(CC) -DPLUGIN=\"$*.h\" $< -o $@ $(LM)

$(exe_l): %_l: bcgen.c %.h $(dep)
	$(CC_L) -DPLUGIN=\"$*.h\" $< -o $@ $(LM_L)

$(exe_q): %_q: bcgen.c %.h $(dep)
	$(CC_Q) -DPLUGIN=\"$*.h\" $< -o $@ $(LM_Q)

$(exe_mp): %_mp: bcgen.c %.h $(dep)
	$(CC_MP) -DPLUGIN=\"$*.h\" $< -o $@ $(LM_MP)

ipo:
	rsync -avzL *.[ch] Makefile cz1@129.109.88.204:lwork/vir/gaussf

sync: ipo

clean:
	$(RM) -f $(exe) $(exe_l) $(exe_q) $(exe_mp) a.out *~ gaussf*.dat

