main: main.o convert.o section.o utils.o version.o symbol.o relocation.o
	gcc -o main main.o convert.o section.o utils.o version.o symbol.o relocation.o -Wl,--hash-style=both -lm
main.o: main.c
	gcc -c main.c
convert.o: convert.c 
	gcc -c convert.c
section.o: section.c
	gcc -c section.c
utils.o: utils.c
	gcc -c utils.c
version.o: version.c
	gcc -c version.c
symbol.o: symbol.c
	gcc -c symbol.c
relocation.o: relocation.c
	gcc -c relocation.c
clean:
	rm -rf main *.o *~
