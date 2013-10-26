CC = icc -O2 -Wall
LM =

comh = nauty.h naurng.h schreier.h sorttemplates.c
comc = nauty.c naurng.c schreier.c nautil.c naugraph.c

progs = nautyex1

$(progs) : % : %.c $(deps)
	$(CC) -o $@ $< $(comc) $(LM)

clean:
	$(RM) $(progs) *~
