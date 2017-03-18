

#ifndef FUNCTIONDECLARE_H
	#define FUNCTIONDECLARE_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#include "const.h"
#include "dataStructures.h"


/* ======== Methods Declaration ======== */
/* generalFunctions.c methods */
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
void printMemArr(int memoryCounter,int *memoryArr);
int secondFileRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC);
/* main.c methods */
int intToBase16(int num, char *buf, int index);
void fprintfBase16(FILE *file, int num, int strMinWidth);
void createObjectFile(char *name, int IC, int DC, int *memoryArr);
void createEntriesFile(char *name);
void createExternFile(char *name, lineInfo *linesArr, int linesFound);
void clearData(lineInfo *linesArr, int linesFound, int dataCount);

#endif
