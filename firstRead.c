#include "assembler.h"

/* ======== Includes ======== */
#include <ctype.h>
#include <stdlib.h>

/* ====== Externs ====== */
extern labelInfo labelArr[MAX_LABELS_NUM]; /* Label array */
extern int labelNum; /* Index label array */
lineInfo *entryLines[MAX_LABELS_NUM]; /* Entry-s array */
extern int entryLabelsNum; /* Index to entery array */
extern int dataArr[MAX_DATA_NUM]; /* Data array- DC-data-counter */

/* ====== Commands List ====== */
const command cmdArr[] =	
{	/* Name | Opcode | NumOfParams */
	{ "mov", 0, 2 } , 
	{ "cmp", 1, 2 } ,
	{ "add", 2, 2 } ,
	{ "sub", 3, 2 } ,
	{ "not", 4, 1 } ,
	{ "clr", 5, 1 } ,
	{ "lea", 6, 2 } ,
	{ "inc", 7, 1 } ,
	{ "dec", 8, 1 } ,
	{ "jmp", 9, 1 } ,
	{ "bne", 10, 1 } ,
	{ "red", 11, 1 } ,
	{ "prn", 12, 1 } ,
	{ "jsr", 13, 1 } ,
	{ "rst", 14, 0 } ,
	{ "stop", 15, 0 } ,
	{ NULL } /* represent the end of the array */
}; 

const directive dircArr[] = 
{	/* Name | Parseing Function */
	{ "data", parseDataDirc } ,
	{ "string", parseStringDirc } ,
	{ "extern", parseExternDirc },
	{ "entry", parseEntryDirc },
	{ NULL } /* represent the end of the array */
};	

void printData(int DC)
{
	int i;
	printf("\nThese are all the data :\n");
		for (i = 0; i < DC; i++)
		{
			printf(" ######## ");
			printf("  %d ",dataArr[i]);
			printf(" ######## \n");



		}
	printf("Lables data\n");

}

int firstFileRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC)
{
	char lineStr[MAX_LINE_LENGTH + 2]; /* +2 for the \n and \0 at the end */
	int errorsFound = 0;

	*linesFound = 0;

	while (!feof(file))
	{
		fgets(lineStr, MAX_LINE_LENGTH + 2, file);
	
		/* Parse a line */
			parseLine(&linesArr[*linesFound], lineStr, *linesFound + 1, IC, DC);

			/* Update errorsFound */
			if (linesArr[*linesFound].isError)
			{
				errorsFound++;
			}

			++*linesFound;

	}
		printLables();
		printData(*DC);
		printf("IC:%d",*IC+FIRST_ADDRESS);

		return errorsFound;

}

void parseLine(lineInfo *line, char *lineStr, int lineNum, int *IC, int *DC)
{
	char *startOfNextPart = lineStr;

	line->lineNum = lineNum;
	line->address = FIRST_ADDRESS + *IC;
	line->originalString = allocString(lineStr);
	line->lineStr = line->originalString;
	line->isError = FALSE;
	line->label = NULL;
	line->commandStr = NULL;
	line->cmd = NULL;

		/* Check if the line is a comment */
	if (isCommentOrEmpty(line))
	{	
		return;
	}

	/* Check if there's a label */

	startOfNextPart = checkLabel(line,*IC);
	if (line->isError)
	{
		return;
	}

		/* Update the line if startOfNextPart isn't NULL */
	if (startOfNextPart)
	{
		line->lineStr = startOfNextPart;
	}

			/* Find the command token */
	line->commandStr = getFirstTok(line->lineStr, &startOfNextPart);
	line->lineStr = startOfNextPart;
	/* Parse the command / directive */
	if (isDirective(line->commandStr))
	{
		line->commandStr++; /* Remove the '.' from the command */
		parseDirective(line, IC, DC);
	}
	else
	{
		parseCommand(line, IC, DC);
	}

	if (line->isError)
	{
		return;
	}
}

labelInfo * addLabelToArr(labelInfo label, lineInfo *line)
{
	/* Check if label is legal */
	if (!isLegalLabel(line->lineStr, line->lineNum, TRUE))
	{
		/* Illegal label name */
		line->isError = TRUE;
		return NULL;
	}

	/* Check if label is legal */
	if (isExistingLabel(line->lineStr))
	{
		printf("%d: Label already exists.",line->lineNum);
		line->isError = TRUE;
		return NULL;
	}

	/* Add the name to the label */
	strcpy(label.name, line->lineStr);

	/* Add the label to g_labelArr and to the lineInfo */
	if (labelNum < MAX_LABELS_NUM)
	{
		labelArr[labelNum] = label;
		return &labelArr[labelNum++];
	}

	/* Too many labels */
	printf("%d: Too many labels - max is %d.",line->lineNum,MAX_LABELS_NUM);
	line->isError = TRUE;
	return NULL;
}

void removeLastLabel(int lineNum)
{
	labelNum--;
	printf("[Warning] At line %d: The assembler ignored the label before the directive.\n", lineNum);
}
/* Adds the number to the g_dataArr and increases DC. Returns if it succeeded. */
bool addNumberToData(int num, int *IC, int *DC, int lineNum)
{
	/* Check if there is enough space in g_dataArr for the data */
	if (*DC + *IC < MAX_DATA_NUM)
	{
		dataArr[(*DC)++] = num;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/* Adds the str to the g_dataArr and increases DC. Returns if it succeeded. */
bool addStringToData(char *str, int *IC, int *DC, int lineNum)
{
	do
	{
		if (!addNumberToData((int)*str, IC, DC, lineNum))
		{
			return FALSE;
		}
	} while (*str++);

	return TRUE;
}
char * checkLabel(lineInfo *line, int IC)
{
	char *labelEnd = strchr(line->lineStr, ':'); /* find location of : , if exists */
	labelInfo label = { 0 };
	label.address = FIRST_ADDRESS + IC;

	/* Find the label (or return NULL if there isn't)- Means if found :, replace by \0 to recognize the label ends after that */
	if (!labelEnd)
	{
		return NULL;
	}
	*labelEnd = '\0';

	/* Check if the sign ':' came after the first word */
	if (!isOneWord(line->lineStr))
	{
		*labelEnd = ':'; /* Fix the change in line->lineStr */
		return NULL;
	}

	/* Check of the label is legal and add it to the labelList */
	line->label = addLabelToArr(label, line);

	return labelEnd + 1; /* +1 to make it point at the next char after the \0 */
}

/* Parses a .data directive. */
void parseDataDirc(lineInfo *line, int *IC, int *DC)
{
	char *operandTok = line->lineStr, *endOfOp = line->lineStr;
	int operandValue;
	bool foundComma;

	/* Make the label a data label (if there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = FIRST_ADDRESS + *DC;
	}

	/* Check if there are params */
	if (isWhiteSpaces(line->lineStr))
	{
		/* No parameters */
		printf("%d:No parameter.",line->lineNum);
		line->isError = TRUE;
		return;
	}

	/* Find all the params and add them to g_dataArr */
	FOREVER
	{
		/* Get next param or break if there isn't */
		if (isWhiteSpaces(line->lineStr))
		{
			break;
		}
		operandTok = getFirstOperand(line->lineStr, &endOfOp, &foundComma);

		/* Add the param to g_dataArr */
		if (isLegalNum(operandTok, MEMORY_WORD_LENGTH, line->lineNum, &operandValue))
		{
			if (!addNumberToData(operandValue, IC, DC, line->lineNum))
			{
				/* Not enough memory */
				line->isError = TRUE;
				return;
			}
		}
		else
		{
			/* Illegal number */
			line->isError = TRUE;
			return;
		}

		/* Change the line to start after the parameter */
		line->lineStr = endOfOp;
	}

	if (foundComma)
	{
		/* Comma after the last param */
		printf("%d:Do not write a comma after the last parameter.",line->lineNum);
		line->isError = TRUE;
		return;
	}
}

/* Parses a .string directive. */
void parseStringDirc(lineInfo *line, int *IC, int *DC)
{
	/* Make the label a data label (if there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = FIRST_ADDRESS + *DC;
	}

	trimStr(&line->lineStr);

	if (isLegalStringParam(&line->lineStr, line->lineNum))
	{
		if (!addStringToData(line->lineStr, IC, DC, line->lineNum))
		{
			/* Not enough memory */
			line->isError = TRUE;
			return;
		}
	}
	else
	{
		/* Illegal string */
		line->isError = TRUE;
		return;
	}
}

/* Parses a .extern directive. */
void parseExternDirc(lineInfo *line)
{
	labelInfo label = { 0 }, *labelPointer;

	/* If there is a label in the line, remove the it from labelArr */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

	trimStr(&line->lineStr);
	labelPointer = addLabelToArr(label, line);

	/* Make the label an extern label */
	if (!line->isError)
	{
		labelPointer->address = 0;
		labelPointer->isExtern = TRUE;
	}
}

/* Parses a .entry directive. */
void parseEntryDirc(lineInfo *line)
{
	/* If there is a label in the line, remove the it from labelArr */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

	/* Add the label to the entry labels list */
	trimStr(&line->lineStr);

	if (isLegalLabel(line->lineStr, line->lineNum, TRUE))
	{
		if (isExistingEntryLabel(line->lineStr))
		{
			printf("%d:Label already defined as an entry label.",line->lineNum);
			line->isError = TRUE;
		}
		else if (entryLabelsNum < MAX_LABELS_NUM)
		{
			entryLines[entryLabelsNum++] = line;
		}
	}
}

/* Parses the directive and in a directive line. */
void parseDirective(lineInfo *line, int *IC, int *DC)
{
	int i = 0;
	while (dircArr[i].name)
	{
		if (!strcmp(line->commandStr, dircArr[i].name))
		{
			/* Call the parse function for this type of directive */
			dircArr[i].parseFunc(line, IC, DC);
			return;
		}
		i++;
	}
	
	/* line->commandStr isn't a real directive */
	printf("%d:No such directive as \"%s\".",line->lineNum, line->commandStr);
	line->isError = TRUE;
}

/* Returns if the operands' types are legal (depending on the command). */
bool areLegalOpTypes(const command *cmd, operandInfo op1, operandInfo op2, int lineNum)
{
	/* --- Check First Operand --- */
	/* "lea" command (opcode is 6) can only get a label as the 1st op */
	if (cmd->opcode == 6 && op1.type != LABEL)
	{
		printf("%d:Source operand for \"%s\" command must be a label.\n",lineNum,cmd->name);
		/*printError(lineNum, "Source operand for \"%s\" command must be a label.", cmd->name);*/
		return FALSE;
	}

	/* --- Check Second Operand --- */
	/* 2nd operand can't be random */

	/* 2nd operand can be a number only if the command is "cmp" (opcode is 1) or "prn" (opcode is 12).*/
	if (op2.type == NUMBER && cmd->opcode != 1 && cmd->opcode != 12)
	{
		printf("%d:Destination operand for \"%s\" command can't be a number.\n",lineNum,cmd->name);
		/*printError(lineNum, "Destination operand for \"%s\" command can't be a number.", cmd->name);*/
		return FALSE;
	}

	return TRUE;
}
/* Updates the type and value of operand. */
void parseOpInfo(operandInfo *operand, int lineNum)
{
	int value = 0;
	int value2 = 0;

	if (isWhiteSpaces(operand->str))
	{
		printf("%d:Empty parameter.",lineNum);
		/*printError(lineNum, "Empty parameter.");*/
		operand->type = INVALID;
		return;
	}


	/* Check if the type is NUMBER */
	if (*operand->str == '#')
	{
		operand->str++; /* Remove the '#' */

		/* Check if the number is legal */
		if (isspace(*operand->str)) 
		{
			printf("%d:There is a white space after'#'.",lineNum);
			/*printError(lineNum, "There is a white space afetr the '#'.");*/
			operand->type = INVALID;
		}
		else
		{
			operand->type = isLegalNum(operand->str, MEMORY_WORD_LENGTH - 2, lineNum, &value) ? NUMBER : INVALID;
		}
	}
	/* Check if the type is REGISTER */
	else if (isRegister(operand->str, &value))
	{
		operand->type = REGISTER;
	}
	/* Check if the type is LABEL */
	else if (isLegalLabel(operand->str, lineNum, FALSE))
	{
		operand->type = LABEL;
	}
	/* Check if the type is Addressing method 2 */
	else if ( isIndex(operand->str, &value , &value2) == TRUE)
	{
		if((value%2) !=0 && value2%2 == 0 )
		{
			operand->type = INDEX;
		}
		else
		{
			if(value%2 == 0)
			{
				printf("%d: Addressing mode 2 error. Outer register is not in the group r1,r3,r5,r7\n",lineNum);
			}
			if(value2%2 != 0)
			{
				printf("%d: Addressing mode 2 error. Inner register is not in the group r2,r4,r6,r8\n ",lineNum);
			}

		}

	}
	/* The type is INVALID */
	else
	{
		printf("%d:\"%s\" is an invalid parameter.",lineNum,operand->str);
		/*printError(lineNum, "\"%s\" is an invalid parameter.", operand->str);*/
		operand->type = INVALID;
		value = -1;
	}

	operand->value = value;
	if(operand->type == INDEX)
	{
		operand->value2 = value2;

	}
}

/* Parses the operands in a command line. */
void parseCmdOperands(lineInfo *line, int *IC, int *DC)
{
	char *startOfNextPart = line->lineStr;
	bool foundComma = FALSE;
	int numOfOpsFound = 0;

	/* Reset the op types */
	line->op1.type = INVALID;

	line->op2.type = INVALID;


	/* Get the parameters */
	FOREVER
	{
		/* If both of the operands are registers, they will only take 1 memory word (instead of 2) */
		if (!(line->op1.type == REGISTER && line->op2.type == REGISTER))
		{
			/* Check if there is enough memory */
			if (*IC + *DC < MAX_DATA_NUM)
			{
				++*IC; /* Count the last command word or operand. */
			}
			else
			{
				line->isError = TRUE;
				return;
			}
		}

		/* Check if there are still more operands to read */
		if (isWhiteSpaces(line->lineStr) || numOfOpsFound > 2)
		{
			/* If there are more than 2 operands it's already illegal */
			break;
		}

		/* If there are 2 ops, make the destination become the source op */
		if (numOfOpsFound == 1)
		{
			line->op1 = line->op2;
			/* Reset op2 */
			if ( line->op2.type == INDEX )
			{
				line->op2.value2=0;
			}
			line->op2.type = INVALID;


		}

		/* Parse the opernad*/
		line->op2.str = getFirstOperand(line->lineStr, &startOfNextPart, &foundComma);
		parseOpInfo(&line->op2, line->lineNum);

		if (line->op2.type == INVALID)
		{
			line->isError = TRUE;
			return;
		}

		numOfOpsFound++;
		line->lineStr = startOfNextPart;
	} /* End of while */

	/* Check if there are enough operands */
	if (numOfOpsFound != line->cmd->numOfParams) 
	{
		/* There are more/less operands than needed */
		if (numOfOpsFound <  line->cmd->numOfParams)
		{
			printf("%d: Not enough operands for %s\n", line->lineNum,line->commandStr);
			/*printError(line->lineNum, "Not enough operands.", line->commandStr);*/
		}
		else
		{	printf("%d: Too many operands for %s\n", line->lineNum,line->commandStr);
			/*printError(line->lineNum, "Too many operands.", line->commandStr);*/
		}

		line->isError = TRUE;
		return;
	}

	/* Check if there is a comma after the last param */
	if (foundComma)
	{
		printf("%d:Comma error\n",line->lineNum);
	/*	printError(line->lineNum, "Don't write a comma after the last parameter.");*/
		line->isError = TRUE;
		return;
	}
	/* Check if the operands' types are legal */
	if (!areLegalOpTypes(line->cmd, line->op1, line->op2, line->lineNum))
	{
		line->isError = TRUE;
		return;
	}
}
/* Parses the command in a command line. */
void parseCommand(lineInfo *line, int *IC, int *DC)
{
	int cmdId = getCmdId(line->commandStr);

	if (cmdId == -1)
	{
		line->cmd = NULL;
		if (*line->commandStr == '\0')
		{
			/* The command is empty, but the line isn't empty so it's only a label. */
			printf("%d:Can't write a label to an empty line. %s \n",line->lineNum,line->commandStr);
			/*printError(line->lineNum, "Can't write a label to an empty line.", line->commandStr);*/
		}
		else
		{
			/* Illegal command. */
			printf("%d:No such command as \"%s\".\n",line->lineNum,line->commandStr);

			/*printError(line->lineNum, "No such command as \"%s\".", line->commandStr);*/
		}
		line->isError = TRUE;
		return;
	}

	line->cmd = &cmdArr[cmdId];
	parseCmdOperands(line, IC, DC);
}
