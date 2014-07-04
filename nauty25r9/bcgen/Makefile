
CC = gcc -O3 -Wall -Wextra
LM = -lm
LQ = -lquadmath
LMP = -lgmp

exe = gaussf
exe_dq    = $(patsubst %, %_dq,  $(exe))
exe_dmp   = $(patsubst %, %_dmp, $(exe))
exe_l     = $(patsubst %, %_l,   $(exe))
exe_lq    = $(patsubst %, %_lq,  $(exe))
exe_lmp   = $(patsubst %, %_lmp, $(exe))
exe_q     = $(patsubst %, %_q, 	 $(exe))
exe_qmp   = $(patsubst %, %_qmp, $(exe))
exe_mp 	  = $(patsubst %, %_mp,  $(exe))

dep = nau0s.h Makefile

default: $(exe)

all: $(exe) $(exe_dq) $(exe_dmp) $(exe_l) $(exe_lq) $(exe_lmp) $(exe_q) $(exe_qmp) $(exe_mp)

$(exe): %: bcgen.c %.h $(dep)
	$(CC) -DPLUGIN=\"$*.h\" $< -o $@ $(LM)

$(exe_dq): %_dq: bcgen.c %.h $(dep)
	$(CC) -DHP_QUAD -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LQ)

$(exe_dmp): %_dmp: bcgen.c %.h $(dep)
	$(CC) -DHP_MP -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LMP)

$(exe_l): %_l: bcgen.c %.h $(dep)
	$(CC) -DLDBL -DHP_LDBL -DPLUGIN=\"$*.h\" $< -o $@ $(LM)

$(exe_lq): %_lq: bcgen.c %.h $(dep)
	$(CC) -DLDBL -DHP_QUAD -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LQ)

$(exe_lmp): %_lmp: bcgen.c %.h $(dep)
	$(CC) -DLDBL -DHP_MP -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LMP)

$(exe_q): %_q: bcgen.c %.h $(dep)
	$(CC) -DQUAD -DHP_QUAD -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LQ)

$(exe_qmp): %_qmp: bcgen.c %.h $(dep)
	$(CC) -DQUAD -DHP_MP -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LQ) $(LMP)

$(exe_mp): %_mp: bcgen.c %.h $(dep)
	$(CC) -DMP -DHP_MP -DPLUGIN=\"$*.h\" $< -o $@ $(LM) $(LMP)

gaussfmerge: gaussfmerge.c
	$(CC) $< -o $@ $(LM) $(LMP)

ipo:
	rsync -avzL *.[ch] *.py Makefile cz1@129.109.88.204:lwork/vir/gaussf

stampede:
	rsync -avzL *.[ch] *.py oo1@stampede.tacc.utexas.edu:work/vir/gaussf

sync: ipo stampede

clean:
	$(RM) -rf $(exe) $(exe_dq) $(exe_dmp) \
	  $(exe_l) $(exe_lq) $(exe_lmp) \
	  $(exe_q) $(exe_qmp) $(exe_mp) \
	  a.out *~ gaussf*.dat r*hs

