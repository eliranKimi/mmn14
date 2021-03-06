/*
General header file for the assembly. 
Contains macros, data structures and methods declaration.

=======================
By Eliran Kimiagarov
ID 305512402
=======================
*/

#ifndef ASSEMBLER_H
	#define ASSEMBLER_H

#include <stdio.h>
#include <string.h>

/* ======== Macros ======== */
/* Utilities */
#define FOREVER				for(;;)
#define BYTE_SIZE			8
#define FALSE				0
#define TRUE				1

/* Given Constants */
#define MAX_DATA_NUM		1000
#define FIRST_ADDRESS		100 
#define MAX_LINE_LENGTH		80
#define MAX_LABEL_LENGTH	30
#define MEMORY_WORD_LENGTH	15
#define MAX_REGISTER_DIGIT	7

/* Defining Constants */
#define MAX_LINES_NUM		700
#define MAX_LABELS_NUM		MAX_LINES_NUM 

/* ======== Data Structures ======== */
typedef unsigned int bool; /* Only get TRUE or FALSE values */

/* === First Read  === */

/* Labels Management */
typedef struct
{
	int address;					/* The address it contains */
	char name[MAX_LABEL_LENGTH];	/* The name of the label */					
	bool isExtern;					/* Extern flag */
	bool isData;					/* Data flag (.data or .string) */
} labelInfo;

/* Directive And Commands */
typedef struct 
{
	char *name;
	void (*parseFunc)();
} directive;

typedef struct
{
	char *name;
	unsigned int opcode : 4;
	int numOfParams;
} command;

/* Operands */
typedef enum { NUMBER = 0, LABEL = 1, INDEX = 2 , REGISTER = 3, INVALID = -1 } opType;

typedef struct
{
	int value;				/* Value */
	int value2;				/*in case of mion2- this will be the value of the second register*/
	char *str;				/* String */
	opType type;			/* Type */
	int address;			/* The adress of the operand in the memory */
} operandInfo;

/* Line */
typedef struct
{
	int lineNum;				/* The number of the line in the file */
	int address;				/* The address of the first word in the line */
	char *originalString;		/* The original pointer, allocated by malloc */
	char *lineStr;				/* The text it contains (changed while using parseLine) */
	bool isError;				/* Represent whether there is an error or not */
	labelInfo *label;			/* A pointer to the lines label in labelArr */

	char *commandStr;			/* The string of the command or directive */

	/* Command line */
	const command *cmd;			/* A pointer to the command in g_cmdArr */
	operandInfo op1;			/* The 1st operand */
	operandInfo op2;			/* The 2nd operand */
} lineInfo;

/* === Second Read  === */

typedef enum { ABSOLUTE = 0, EXTENAL = 1, RELOCATABLE = 2 } eraType;

/* Memory Word */

typedef struct /* 15 bits */
{
	unsigned int era : 2;

	union /* 13 bits */
	{
		/* Commands (only 13 bits) */
		struct
		{
			unsigned int dest : 2;		/* Destination op addressing method ID */
			unsigned int src : 2;		/* Source op addressing method ID */
			unsigned int opcode : 4;	/* Command ID */
			unsigned int group : 2;		/* Number of params */
			unsigned int unused : 3;			/* Unused Bit */
		} cmdBits;

		/* Registers (only 13 bits) */
		struct
		{
			unsigned int destBits : 6;
			unsigned int srcBits : 6;
			unsigned int : 1;			/* Unused Bit */
		} regBits;

		/* Other operands */
		int value : 13; /* (13 bits) */

	} valueBits; /* End of 13 bits union */

} memoryWord;


/* ======== Methods Declaration ======== */
/* utility.c methods */
int getCmdId(char *cmdName);
void printLables();
labelInfo *getLabel(char *labelName);
void trimLeftStr(char **ptStr);
void trimStr(char **ptStr);
char *getFirstTok(char *str, char **endOfTok);
bool isOneWord(char *str);
bool isWhiteSpaces(char *str);
bool isLegalLabel(char *label, int lineNum, bool printErrors);
bool isExistingLabel(char *label);
bool isExistingEntryLabel(char *labelName);
bool isRegister(char *str, int *value);
bool isCommentOrEmpty(lineInfo *line);
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma);
bool isDirective(char *cmd);
bool isLegalStringParam(char **strParam, int lineNum);
bool isLegalNum(char *numStr, int numOfBits, int lineNum, int *value);
bool isIndex(char *line, int *value, int *value2);
/* firstRead.c methods */

/*command methods */
bool areLegalOpTypes(const command *cmd, operandInfo op1, operandInfo op2, int lineNum);
void parseOpInfo(operandInfo *operand, int lineNum);
void parseCmdOperands(lineInfo *line, int *IC, int *DC);
void parseCommand(lineInfo *line, int *IC, int *DC);

char *allocString(const char *str) ;
int firstFileRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC);
void parseLine(lineInfo *line, char *lineStr, int lineNum, int *IC, int *DC);


/* Directive methods */
void parseDirective(lineInfo *line, int *IC, int *DC);
void parseEntryDirc(lineInfo *line);
void parseExternDirc(lineInfo *line);
void parseStringDirc(lineInfo *line, int *IC, int *DC);
void parseDataDirc(lineInfo *line, int *IC, int *DC);

/* add */
bool addStringToData(char *str, int *IC, int *DC, int lineNum);
bool addNumberToData(int num, int *IC, int *DC, int lineNum);

/* misc label */
void removeLastLabel(int lineNum);
char * checkLabel(lineInfo *line, int IC);




/* secondRead.c methods */
void updateDataLabelsAddress(int IC);
int countIllegalEntries();
bool updateLableOpAddress(operandInfo *op, int lineNum);
int getNumFromMemoryWord(memoryWord memory,bool isitcmd);
int getOpTypeId(operandInfo op);
memoryWord getCmdMemoryWord(lineInfo line);
memoryWord getOpMemoryWord(operandInfo op, bool isDest);
void addWordToMemory(int *memoryArr, int *memoryCounter, memoryWord memory,bool isitcmd);
bool addLineToMemory(int *memoryArr, int *memoryCounter, lineInfo *line);
void addDataToMemory(int *memoryArr, int *memoryCounter, int DC);
int secondFileRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC);
/* main.c methods */
int intToBase16(int num, char *buf, int index);
void fprintfBase16(FILE *file, int num, int strMinWidth);
void createObjectFile(char *name, int IC, int DC, int *memoryArr);
void createEntriesFile(char *name);
void createExternFile(char *name, lineInfo *linesArr, int linesFound);
/*void printError(int lineNum, const char *format, ...);*/

#endif
