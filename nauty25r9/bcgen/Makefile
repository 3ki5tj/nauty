CC=icc -Wall -Wremarks
LM=

exe=gaussf
dep=nau0s.h Makefile

$(exe): %: bcgen.c %.h $(dep)
	$(CC) -DPLUGIN=\"$@.h\" $< -o $@ $(LM)

ipo:
	rsync -avzL *.[ch] cz1@129.109.88.204:lwork/vir

clean:
	$(RM) -f $(exe) a.out *~

