/*
 * data_s.h
 *
 *  Created on: Mar 14, 2017
 *      Author: eliran
 */

#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_


#include <stdio.h>
#include <string.h>
#include "const.h"


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


#endif /* DATASTRUCTURES_H_ */
