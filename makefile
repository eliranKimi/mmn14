main-run: main.o firstRead.o secondRead.o utility.o
		gcc -g -ansi -Wall main.o firstRead.o secondRead.o utility.o -o main-run

firstRead.o: firstRead.c
	gcc -c -ansi -Wall firstRead.c -o firstRead.o

secondRead.o: secondRead.c
	gcc -c -ansi -Wall secondRead.c -o secondRead.o	
	
utility.o: utility.c
	gcc -c -ansi -Wall utility.c -o utility.o

main.o: main.c
	gcc -c -ansi -Wall main.c -o main.o