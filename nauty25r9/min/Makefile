CC = icc -g -Wall
LM =

iprogs = ex1

progs = mynautyex1

targets: $(iprogs) $(progs)

$(iprogs) : % : %.c nau0s.h
	$(CC) -o $@ $< $(LM)

comh = nauty.h naurng.h schreier.h sorttemplates.c
comc = nauty.c naurng.c schreier.c nautil.c naugraph.c
deps = $(comh) $(comc) Makefile

$(progs) : % : %.c $(deps)
	$(CC) -DWORDSIZE=32 -o $@ $< $(comc) $(LM)

clean:
	$(RM) -rf $(progs) $(iprogs) *~ a.out bin obj

uncrustify: uncrustify.cfg Makefile
	uncrustify -c $< --replace nau0*.h
	cd ..
	uncrustify -c $< --replace $(comh) $(comc)
