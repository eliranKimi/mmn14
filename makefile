main-run: main.o firstRead.o secondRead.o generalFunctions.o
		gcc -g -ansi -Wall main.o firstRead.o secondRead.o generalFunctions.o -o main-run

firstRead.o: firstRead.c
	gcc -c -ansi -Wall firstRead.c -o firstRead.o

secondRead.o: secondRead.c
	gcc -c -ansi -Wall secondRead.c -o secondRead.o	
	
generalFunctions.o: generalFunctions.c
	gcc -c -ansi -Wall generalFunctions.c -o generalFunctions.o

main.o: main.c
	gcc -c -ansi -Wall main.c -o main.o