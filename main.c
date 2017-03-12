#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"

/* ====== Global Data Structures ====== */

/* Labels */
labelInfo labelArr[MAX_LABELS_NUM]; 
int labelNum = 0;

/* Entry Lines */
lineInfo *entryLines[MAX_LABELS_NUM]; 
int entryLabelsNum = 0;

/* Data */
int dataArr[MAX_DATA_NUM];

void printError(int lineNum, const char *format, ...);
FILE *openFile(char *name, char *ending, const char *mode);
void parseFile(char *fileName);


int main(int argc, char* argv[])
{
	int i;


	if(argc==1)
    {
        printf("No files entered\n");
        return 0;
    }
		
	for (i = 1; i < argc; i++)
	{
		parseFile(argv[i]);
		printf("\n");
	}

	return 0;

}


/* Print an error with the line number. */
void printError(int lineNum, const char *format, ...)
{
	return;
}
FILE *openFile(char *name, char *ending, const char *mode)
{
	FILE *file;
	char *mallocStr = (char *)malloc(strlen(name) + strlen(ending) + 1), *fileName = mallocStr;
	sprintf(fileName, "%s%s", name, ending);

	file = fopen(fileName, mode);
	free(mallocStr);

	return file;
}

void parseFile(char *fileName)
{
	
	lineInfo linesArr[MAX_LINES_NUM]; 
	int memoryArr[MAX_DATA_NUM] = { 0 };
	int IC = 0;
	int DC = 0;
	int numOfErrors = 0;
	int linesFound = 0;

	/*char line[MAX_LINES_NUM];*/

	FILE *file = openFile(fileName,".as", "r");
	
		/* Open File */
	if (file == NULL)
	{
		printf("[Info] Can't open the file \"%s.as\".\n", fileName);
		return;
	}
	printf("[Info] Successfully opened the file \"%s.as\".\n", fileName);


	/* First Read */
	numOfErrors += firstFileRead(file, linesArr, &linesFound, &IC, &DC);
	/* Second Read */
	numOfErrors += secondFileRead(memoryArr, linesArr, linesFound, IC, DC);
/* Create Output Files */
	if (numOfErrors == 0)
	{
		/* Create all the output files */
		createObjectFile(fileName, IC, DC, memoryArr);
		createExternFile(fileName, linesArr, linesFound); 
		createEntriesFile(fileName);
		printf("[Info] Created output files for the file \"%s.as\".\n", fileName);
	}
	else
	{
		/* print the number of errors. */
		printf("[Info] A total of %d error%s found throughout \"%s.as\".\n", numOfErrors, (numOfErrors > 1) ? "s were" : " was", fileName);
	}

}

/* Puts in the given buffer a base 16 representation of num. */
int intToBase16(int num, char *buf, int index)
{
	const int base = 16;
	const char digits[] = "0123456789ABCDEF";

	if (num)
	{		
		index = intToBase16(num / base, buf, index);
		buf[index] = digits[num % base];
		return ++index;
	}

	return 0;
}

/* Prints a number in base 16 in the file. */
void fprintfBase16(FILE *file, int num, int strMinWidth)
{
	int numOfZeros, i;
	/* 2^15 = 32 ^ 3, So 3 chars are enough to represent 15 bits in base 32, and the last char is \0. */
	char buf[4] = { 0 };

	intToBase16(num, buf, 0);

	/* Add zeros first, to make the length at least strMinWidth */
	numOfZeros = strMinWidth - strlen(buf);
	for (i = 0; i < numOfZeros; i++)
	{
		fprintf(file, "0");
	}
	fprintf(file, "%s", buf);
}

/* Creates the .obj file, which contains the assembled lines in base 16. */
void createObjectFile(char *name, int IC, int DC, int *memoryArr)
{
	int i;
	FILE *file;
	file = openFile(name, ".ob", "w");

	/* Print IC and DC */
	fprintfBase16(file, IC, 1);
	fprintf(file, "\t\t");
	fprintfBase16(file, DC, 1);

	/* Print all of memoryArr */
	for (i = 0; i < IC + DC; i++)
	{
		fprintf(file, "\n");
		fprintfBase16(file, FIRST_ADDRESS + i, 4);
		fprintf(file, "\t\t");
		fprintfBase16(file, memoryArr[i], 4);
	}

	fclose(file);
}

void createEntriesFile(char *name)
{
	int i;
	FILE *file;

	/* Don't create the entries file if there aren't entry lines */
	if (!entryLabelsNum)
	{
		return;
	}

	file = openFile(name, ".ent", "w");

	for (i = 0; i < entryLabelsNum; i++)
	{
		fprintf(file, "%s\t\t", entryLines[i]->lineStr);
		fprintfBase16(file, getLabel(entryLines[i]->lineStr)->address, 1);

		if (i != entryLabelsNum - 1)
		{
			fprintf(file, "\n");
		}
	}

	fclose(file);
}

/* Creates the .ext file, which contains the addresses for the extern labels operands in base 32. */
void createExternFile(char *name, lineInfo *linesArr, int linesFound)
{
	int i;
	labelInfo *label;
	bool firstPrint = TRUE; /* This bool meant to prevent the creation of the file if there aren't any externs */
	FILE *file = NULL;

	for (i = 0; i < linesFound; i++)
	{
		/* Check if the 1st operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 2 && linesArr[i].op1.type == LABEL)
		{
			label = getLabel(linesArr[i].op1.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintfBase16(file, linesArr[i].op1.address, 1);
				firstPrint = FALSE;
			}
		}

		/* Check if the 2nd operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].op2.type == LABEL)
		{
			label = getLabel(linesArr[i].op2.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintfBase16(file, linesArr[i].op2.address, 1);
				firstPrint = FALSE;
			}
		}
	}

	if (file)
	{
		fclose(file);
	}
}
