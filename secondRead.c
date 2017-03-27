
#include "const.h"
#include "functionDeclare.h"
#include "dataStructures.h"

/* Second read of the file - works with the data from first read*/

extern labelInfo labelArr[MAX_LABELS_NUM]; /* Label array */
extern int labelNum; /* Index to labels array */
lineInfo *entryLines[MAX_LABELS_NUM]; /* Entry-s array */
extern int entryLabelsNum; /* Index to entry array */
extern int dataArr[MAX_DATA_NUM]; /* Data array- DC-data-counter */




int secondFileRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC)
{
	int errorsFound = 0, memoryCounter = 0, i;

	/* Update the data labels */
	updateDataLabelsAddress(IC);

	/* Check if there are illegal entries */
	errorsFound += countIllegalEntries();

	/* Add each line in linesArr to the memoryArr */
	for (i = 0; i < lineNum; i++)
	{
		if (!addLineToMemory(memoryArr, &memoryCounter, &linesArr[i]))
		{
			/* An error was found while adding the line to the memory */
			errorsFound++;
		}
	}

	/* Add the data from dataArr to the end of memoryArr */
	addDataToMemory(memoryArr, &memoryCounter, DC);

	return errorsFound;
}

/* Updates the addresses of all the data labels in labelArr. */
void updateDataLabelsAddress(int IC)
{
	int i;

	/* Search in the array for label with isData flag */
	for (i = 0; i < labelNum; i++)
	{
		if (labelArr[i].isData)
		{
			/* Increase the address */
			labelArr[i].address += IC;
		}
	}
}

/* Returns if there is an illegal entry line in entryLines. */
int countIllegalEntries()
{
	int i, ret = 0;
	labelInfo *label;

	for (i = 0; i < entryLabelsNum; i++)
	{
		label = getLabel(entryLines[i]->lineStr);
		if (label)
		{
			if (label->isExtern)
			{
				printf("ERROR: %d:The parameter for .entry can't be an external label. \n",entryLines[i]->lineNum);
				ret++;
			}
		}
		else
		{
			printf("ERROR: %d:No such label as \"%s\". \n",entryLines[i]->lineNum,entryLines[i]->lineStr);
			ret++;
		}
	}

	return ret;
}

/* Adds a whole line into the memoryArr, and increase the memory counter. */
bool addLineToMemory(int *memoryArr, int *memoryCounter, lineInfo *line)
{
	bool foundError = FALSE;

	if (!line->isError && line->cmd != NULL)
	{
		/* Update the label operands value */
		if (!updateLableOpAddress(&line->op1, line->lineNum) || !updateLableOpAddress(&line->op2, line->lineNum))
		{
			line->isError = TRUE;
			foundError = TRUE;
		}

		/* Add the command word to the memory */
		addWordToMemory(memoryArr, memoryCounter, getCmdMemoryWord(*line),TRUE);

	if(line->op1.type == INDEX || line->op2.type == INDEX)
	{
	if(line->op1.type == INDEX)
	{
		/* Create the memory word */
		memoryWord memory = { 0 };
		memory.era = (eraType)ABSOLUTE;
		memory.valueBits.regBits.destBits = line->op1.value;
		memory.valueBits.regBits.srcBits = line->op1.value2;

		/* Add the memory to  memory  array */
		addWordToMemory(memoryArr, memoryCounter, memory,FALSE);

	}

	if(line->op2.type == INDEX)
	{
		/* Create the memory word */
		memoryWord memory = { 0 };
		memory.era = (eraType)ABSOLUTE; /* Registers are absolute */
		memory.valueBits.regBits.destBits = line->op2.value;
		memory.valueBits.regBits.srcBits = line->op2.value2;

		/* Add to  memoryArr array */
		addWordToMemory(memoryArr, memoryCounter, memory,FALSE);

	}
	}

	/* If both operands are registers */
 if (line->op1.type == REGISTER && line->op2.type == REGISTER)
	{
		/* Create the memory word */
		memoryWord memory = { 0 };
		memory.era = (eraType)ABSOLUTE; /* Registers are absolute */
		memory.valueBits.regBits.destBits = line->op2.value;
		memory.valueBits.regBits.srcBits = line->op1.value;

		/* Add the memory to the memoryArr array */
		addWordToMemory(memoryArr, memoryCounter, memory,FALSE);
	}
	else
	{
		/* Check if there is a source operand in this line */
		if (line->op1.type != INVALID && line->op1.type != INDEX)
		{
			/* Add the op1 word to the memory */
			line->op1.address = FIRST_ADDRESS + *memoryCounter;
			addWordToMemory(memoryArr, memoryCounter, getOpMemoryWord(line->op1, FALSE),FALSE);
			/* First boolean is to check if operand isDest. second one isCmd */
		}

		/*Check if there is a destination operand in this line */
		if (line->op2.type != INVALID && line->op2.type != INDEX)
		{
			/* Add the op2 word to the memory */
			line->op2.address = FIRST_ADDRESS + *memoryCounter;
			addWordToMemory(memoryArr, memoryCounter, getOpMemoryWord(line->op2, TRUE),FALSE);
			/* First boolean is to check if operand isDest. second one isitCmd */
		}
	}
}

	return !foundError;
}



/* Checks If the op is a label, this method is updating the value of the it to be the address of the label. */

bool updateLableOpAddress(operandInfo *op, int lineNum)
{
	if (op->type == LABEL)
	{
	
			labelInfo *label = getLabel(op->str);

			/* Check if op.str is a real label name */
			if (label == NULL)
			{
				/* Print errors (legal name is illegal or not exists yet) */
				if (isLegalLabel(op->str, lineNum, TRUE))
				{
					printf("ERROR: No such label as \"%s\"",op->str);
				}
				return FALSE;
			}

			op->value = label->address;
		
	}

	return TRUE;
}

/* Returns int value of a memory word. */
int getNumFromMemoryWord(memoryWord memory,bool isitcmd)
{
	/* Create an int of "MEMORY_WORD_LENGTH" times '1', and all the rest are '0' */
	unsigned int mask = ~0;
	mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LENGTH);

/* Checks if the word is a cmd word */
	if ( isitcmd == TRUE)
	{
	/* Turn on unused bits */
	return UNUSED_BITS_WORD | (mask & ((memory.valueBits.value << 2) + memory.era));
	}
	else
	{
		return (mask & ((memory.valueBits.value << 2) + memory.era));
	}
}

/* Returns the id of the addressing method of the operand */
int getOpTypeId(operandInfo op)
{
	/* Check if the operand have legal type */
	if (op.type != INVALID)
	{
		/* Value according to opType (enum defined at datastrucers.h )*/
		return (op.value2) ? 2 : (int)op.type;
	}

	return 0;
}

/* Returns a memory word which represents the command in a line. */
memoryWord getCmdMemoryWord(lineInfo line)
{
	memoryWord memory = { 0 };

	/* Update all the bits in the command word */
	memory.era = (eraType)ABSOLUTE; /* Commands are absolute */
	memory.valueBits.cmdBits.dest = getOpTypeId(line.op2);
	memory.valueBits.cmdBits.src = getOpTypeId(line.op1);
	memory.valueBits.cmdBits.opcode = line.cmd->opcode;
	memory.valueBits.cmdBits.group = line.cmd->numOfParams;

	return memory;
}

/* Returns a memory word which represents the operand (assuming it's a valid operand). */
memoryWord getOpMemoryWord(operandInfo op, bool isDest)
{
	memoryWord memory = { 0 };

	/* Check if it's a register or not */
	if (op.type == REGISTER)
	{
		memory.era = (eraType)ABSOLUTE; /* Registers are absolute */

		/* Check if it's the dest or src */
		if (isDest)
		{
			memory.valueBits.regBits.destBits = op.value;
		}
		else
		{
			memory.valueBits.regBits.srcBits = op.value;
		}
	}
	else
	{
		labelInfo *label = getLabel(op.str);

		/* Set era */	
		if (op.type == LABEL && label && label->isExtern)
		{
			memory.era = EXTENAL;
		}
		else
		{
			memory.era = (op.type == NUMBER) ? (eraType)ABSOLUTE : (eraType)RELOCATABLE;
		}

		memory.valueBits.value = op.value;
	}

	return memory;
}

/* Adds the value of memory word to the memoryArr, and increase the memory counter. */
void addWordToMemory(int *memoryArr, int *memoryCounter, memoryWord memory,bool isitcmd)
{
	/* Check if memoryArr isn't full yet */
	if (*memoryCounter < MAX_DATA_NUM)
	{
		/* Add the memory word and increase memoryCounter */
		memoryArr[(*memoryCounter)++] = getNumFromMemoryWord(memory,isitcmd);
	}
}


/* Adds the data from dataArr to the end of memoryArr. */
void addDataToMemory(int *memoryArr, int *memoryCounter, int DC)
{
	int i;
	/* Create an int of "MEMORY_WORD_LENGTH" times '1', and all the rest are '0' */
	unsigned int mask = ~0;
	mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LENGTH);

	/* Add each int from dataArr to the end of memoryArr */
	for (i = 0; i < DC; i++)
	{
		if (*memoryCounter < MAX_DATA_NUM)
		{
			/* The mask makes sure we only use the first "MEMORY_WORD_LENGTH" bits */
			memoryArr[(*memoryCounter)++] = mask & dataArr[i];
		}
		else
		{
			/* No more space in memoryArr */
			return;
		}
	}
}

/* Prints memory array - Used for checking */
void printMemArr(int memoryCounter,int *memoryArr)
{

	int i;
	int lineCount=0;
		printf("\nMemory :\n");
			for (i= 0; i < memoryCounter; i++)
			{lineCount++;
				printf(" ######## ");
				printf("  %d ",memoryArr[i]);
				printf(" ######## \n");

			}
		printf("count:%d\n",lineCount);

}


