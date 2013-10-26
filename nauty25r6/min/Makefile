CC = icc -g -Wall
LM =

iprogs = ex1

$(iprogs) : % : %.c nau0s.h
	$(CC) -o $@ $< $(LM)



comh = nauty.h naurng.h schreier.h sorttemplates.c
comc = nauty.c naurng.c schreier.c nautil.c naugraph.c
deps = $(comh) $(comc) Makefile

progs = mynautyex1

$(progs) : % : %.c $(deps)
	$(CC) -DWORDSIZE=32 -o $@ $< $(comc) $(LM)

clean:
	$(RM) -rf $(progs) $(iprogs) *~ a.out bin obj
