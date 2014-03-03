main: main.o convert.o section.o
	gcc -o main main.o convert.o section.o
main.o: main.c 
	gcc -c main.c
convert.o: convert.c
	gcc -c convert.c
section.o: section.c
	gcc -c section.c
clean:
	rm main *.o
