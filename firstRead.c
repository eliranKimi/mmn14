
#include "const.h"
#include "functionDeclare.h"
#include "dataStructures.h"

/* First read of the file */

extern labelInfo labelArr[MAX_LABELS_NUM]; /* Label array */
extern int labelNum; /* Index label array */
lineInfo *entryLines[MAX_LABELS_NUM]; /* Entry-s array */
extern int entryLabelsNum; /* Index to entery array */
extern int dataArr[MAX_DATA_NUM]; /* Data array- DC-data-counter */

/* Commands Array */
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
	{ NULL }
}; 

void printData(int DC)
{
	int i;
	printf("\nData array:\n");
		for (i = 0; i < DC; i++)
		{
			printf(" ######## ");
			printf("  %d ",dataArr[i]);
			printf(" ######## \n");



		}
	printf("End Data\n");

}

int firstFileRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC)
{
	char lineStr[MAX_LINE_LENGTH];
	int errorsFound = 0;

	*linesFound = 0;

	while (!feof(file))
	{
		fgets(lineStr, MAX_LINE_LENGTH, file);
	
			parseLine(&linesArr[*linesFound], lineStr, *linesFound + 1, IC, DC);

			/* Update errorsFound */
			if (linesArr[*linesFound].isError)
			{
				errorsFound++;
			}

			++*linesFound;

	}

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

			/* Find the command  */
	line->commandStr = getFirstTok(line->lineStr, &startOfNextPart);
	line->lineStr = startOfNextPart;
	/* Parse the command / directive */
	if (isDirective(line->commandStr))
	{
		line->commandStr++; /* Remove'.' from command */
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
		line->isError = TRUE;
		return NULL;
	}

	if (isExistingLabel(line->lineStr))
	{
		printf("ERROR: %d: Label already exists.",line->lineNum);
		line->isError = TRUE;
		return NULL;
	}

	strcpy(label.name, line->lineStr);

	if (labelNum < MAX_LABELS_NUM)
	{
		labelArr[labelNum] = label;
		return &labelArr[labelNum++];
	}

	printf("ERROR: %d: Too many labels - max is %d.",line->lineNum,MAX_LABELS_NUM);
	line->isError = TRUE;
	return NULL;
}

void removeLastLabel(int lineNum)
{
	labelNum--;
	printf("WARN: At line %d: The assembler ignored the label before the directive.\n", lineNum);
}
/* Adds the number to the dataArr and increases data counter */
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

/* Adds  str to the dataArr and increases data counter */
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
	char *labelEnd = strchr(line->lineStr, ':'); /* finds ':' */
	labelInfo label = { 0 };
	label.address = FIRST_ADDRESS + IC;

	/* Find the label and place '\0\' at the end */
	if (!labelEnd)
	{
		return NULL;
	}
	*labelEnd = '\0';

	/* Check if the sign ':' came after the first word */
	if (!isOneWord(line->lineStr))
	{
		*labelEnd = ':';
		return NULL;
	}

	/* Check of the label& add it to the label array */
	line->label = addLabelToArr(label, line);

	return labelEnd + 1; /* Increase pointer to next character */
}


void parseDataDirc(lineInfo *line, int *IC, int *DC)
{
	char *operandTok = line->lineStr, *endOfOp = line->lineStr;
	int operandValue;
	bool foundComma;

	/* if data, turn on isData flag) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = FIRST_ADDRESS + *DC;
	}

	/* Check for parameters */
	if (isWhiteSpaces(line->lineStr))
	{
		printf("ERROR: %d:No parameter.",line->lineNum);
		line->isError = TRUE;
		return;
	}

	/* Find  parameters and add them data array */
	FOREVER
	{
		/* Get next parameter */
		if (isWhiteSpaces(line->lineStr))
		{
			break;
		}
		operandTok = getFirstOperand(line->lineStr, &endOfOp, &foundComma);

		/* Add the parameter to data array */
		if (isLegalNum(operandTok, MEMORY_WORD_LENGTH, line->lineNum, &operandValue))
		{
			if (!addNumberToData(operandValue, IC, DC, line->lineNum))
			{
				line->isError = TRUE;
				return;
			}
		}
		else
		{
			line->isError = TRUE;
			return;
		}

		/* Increase pointer to str after parameter */
		line->lineStr = endOfOp;
	}

	if (foundComma)
	{

		printf("ERROR: %d:Do not write a comma after the last parameter.",line->lineNum);
		line->isError = TRUE;
		return;
	}
}

void parseStringDirc(lineInfo *line, int *IC, int *DC)
{
	/* if data, turn on isData flag) */
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

			line->isError = TRUE;
			return;
		}
	}
	else
	{
		line->isError = TRUE;
		return;
	}
}

void parseExternDirc(lineInfo *line)
{
	labelInfo label = { 0 }, *labelPointer;

	/* If label is a duplicate , remove it from label array */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

	trimStr(&line->lineStr);
	labelPointer = addLabelToArr(label, line);

	/* if extern, turn on isExtern flag) */
	if (!line->isError)
	{
		labelPointer->address = 0;
		labelPointer->isExtern = TRUE;
	}
}

void parseEntryDirc(lineInfo *line)
{
	/* If label is a duplicate , remove it from label array */
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
			printf("ERROR: %d:Label already defined as an entry label.",line->lineNum);
			line->isError = TRUE;
		}
		else if (entryLabelsNum < MAX_LABELS_NUM)
		{
			entryLines[entryLabelsNum++] = line;
		}
	}
}

void parseDirective(lineInfo *line, int *IC, int *DC)
{

	if ( !strcmp(line->commandStr,"data")) /* if data directive */
	{
		parseDataDirc(line,IC,DC);
	}
	else if (!strcmp(line->commandStr,"string")) /* if string directive */
	{
		parseStringDirc(line,IC,DC);
	}
	else if (!strcmp(line->commandStr,"extern")) /* if extern directive */
	{
		parseExternDirc(line);
	}
	else if (!strcmp(line->commandStr,"entry")) /* if entry directive */
	{
		parseEntryDirc(line);
	}
	else
	{
		printf("ERROR: %d:No such directive as \"%s\".",line->lineNum, line->commandStr);
		line->isError = TRUE;

	}


}

/* Check if the operands' types are legal*/
bool areLegalOpTypes(const command *cmd, operandInfo op1, operandInfo op2, int lineNum)
{
	/* Check First Operand */
	/* "lea" command (opcode is 6) can only get a label as the 1st op */
	if (cmd->opcode == 6 && op1.type != LABEL)
	{
		printf("ERROR: %d:Source operand for \"%s\" command must be a label.\n",lineNum,cmd->name);
		return FALSE;
	}

	/*Check Second Operand*/

	/* 2nd operand can be a number only if the command is "cmp" (opcode is 1) or "prn" (opcode is 12).*/
	if (op2.type == NUMBER && cmd->opcode != 1 && cmd->opcode != 12)
	{
		printf("ERROR: %d:Destination operand for \"%s\" command can't be a number.\n",lineNum,cmd->name);
		return FALSE;
	}

	return TRUE;
}
/* Updates the type and value of operand. */
void parseOpInfo(operandInfo *operand, int lineNum)
{
	int value = 0;
	int value2 = 0; /* Used in case of addressing method 2 */

	if (isWhiteSpaces(operand->str))
	{
		printf("ERROR: %d:Empty parameter.",lineNum);
		operand->type = INVALID;
		return;
	}


	/* Check if the type is NUMBER */
	if (*operand->str == '#')
	{
		operand->str++; /* Skip '#' */

		/* Check if the number is legal */
		if (isspace(*operand->str)) 
		{
			printf("ERROR: %d:There is a white space after'#'.",lineNum);
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
	/* Check if the type is INDEX (Addressing method 2) */
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
				printf("ERROR: %d: Addressing mode 2 error. Outer register is not in the group r1,r3,r5,r7\n",lineNum);
			}
			if(value2%2 != 0)
			{
				printf("ERROR: %d: Addressing mode 2 error. Inner register is not in the group r2,r4,r6,r8\n ",lineNum);
			}

		}

	}
	/* The type is INVALID */
	else
	{
		printf("ERROR: %d:\"%s\" is an invalid parameter.",lineNum,operand->str);
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
			printf("ERROR: %d: Not enough operands for %s\n", line->lineNum,line->commandStr);
		}
		else
		{	printf("ERROR: %d: Too many operands for %s\n", line->lineNum,line->commandStr);
		}

		line->isError = TRUE;
		return;
	}

	/* Check if there is a comma after the last parameter */
	if (foundComma)
	{
		printf("ERROR: %d:Comma error\n",line->lineNum);
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
void parseCommand(lineInfo *line, int *IC, int *DC)
{
	int cmdId = getCmdId(line->commandStr);

	if (cmdId == -1)
	{
		line->cmd = NULL;
		if (*line->commandStr == '\0')
		{
			/* The command is empty, but the line isn't empty so it's only a label. */
			printf("ERROR: %d:Can't write a label to an empty line. %s \n",line->lineNum,line->commandStr);
		}
		else
		{
			printf("ERROR: %d:No such command as \"%s\".\n",line->lineNum,line->commandStr);

		}
		line->isError = TRUE;
		return;
	}

	line->cmd = &cmdArr[cmdId];
	parseCmdOperands(line, IC, DC);
}
