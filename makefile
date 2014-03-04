main: main.o convert.o section.o utils.o version.o symbol.o
	gcc -o main main.o convert.o section.o utils.o version.o symbol.o
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
clean:
	rm main *.o
