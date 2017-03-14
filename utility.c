
/* ======== Includes ======== */

#include "const.h"
#include "dataStructures.h"
#include "functionDeclare.h"

/* ====== Methods ====== */
extern const command cmdArr[];
extern labelInfo labelArr[];
extern int labelNum;
lineInfo *entryLines[MAX_LABELS_NUM];
extern int entryLabelsNum;

bool isCommentOrEmpty(lineInfo *line)
{
	char *startOfText = line->lineStr; /* We don't want to change line->lineStr */

	if (*line->lineStr == ';')
	{
		/* Comment */
		return TRUE;
	}

	trimLeftStr(&startOfText);
	if (*startOfText == '\0')
	{
		/* Empty line */
		return TRUE;
	}
	if (*startOfText == ';')
	{
		/* Illegal comment - ';' isn't at the start of the line */
		printf( "ERROR:  %d Comments must start with ';' at the start of the line.",line->lineNum);
		line->isError = TRUE;
		return TRUE;
	}

	/* Not empty or comment */
	return FALSE;
}

/* Returns the same string in a different part of the memory by using malloc. */
char *allocString(const char *str) 
{
	char *newString = (char *)malloc(strlen(str) + 1);
	if (newString) 
	{
		strcpy(newString, str); 
	}

	return newString;
}


/* Removes spaces from start */
void trimLeftStr(char **ptStr)
{
	/* Return if it's NULL */
	if (!ptStr)
	{
		return;
	}

	/* Get ptStr to the start of the actual text */
	while (isspace(**ptStr))
	{
		++*ptStr;
	}
}

/* Returns if str contains only white spaces. */
bool isWhiteSpaces(char *str)
{
	while (*str)
	{
		if (!isspace(*str++))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/* Returns if str contains only one word. */
bool isOneWord(char *str)
{
	trimLeftStr(&str);							/* Skip the spaces at the start */
	while (!isspace(*str) && *str) { str++; }	/* Skip the text at the middle */					

	/* Return if it's the end of the text or not. */
	return isWhiteSpaces(str);
}



/* Returns a pointer to the label with 'labelName' name in labelArr or NULL if there isn't such label. */
labelInfo *getLabel(char *labelName)
{
	int i = 0;

	if (labelName)
	{
		for (i = 0; i < labelNum; i++)
		{
			if (strcmp(labelName, labelArr[i].name) == 0)
			{
				return &labelArr[i];
			}
			
		}
	}
	return NULL;
}

void printLables()
{
	int i;
	printf("\nThese are all the lables :\n");
		for (i = 0; i < labelNum; i++)
		{
			printf(" ######## ");
			printf("  %s ",labelArr[i].name);
			printf("   %d ",labelArr[i].address);
			printf(" ######## \n");


			
		}
	printf("Lables end\n");	

}





/* Returns the ID of the command with 'cmdName' name in cmdArr or -1 if there isn't such command. */
int getCmdId(char *cmdName)
{
	int i = 0;

	while (cmdArr[i].name)
	{
		if (strcmp(cmdName, cmdArr[i].name) == 0)
		{
			return i;
		}

		i++;
	}
	return -1;
}



/* Removes all the spaces from the edges of the string ptStr is pointing to. */
void trimStr(char **ptStr)
{
	char *eos;

	/* Return if it's NULL or empty string */
	if (!ptStr || **ptStr == '\0')
	{
		return;
	}

	trimLeftStr(ptStr);

	/* eos is pointing to the last char in str, before '\0' */
	eos = *ptStr + strlen(*ptStr) - 1;

	/* Remove spaces from the end */
	while (isspace(*eos) && eos != *ptStr)
	{
		*eos-- = '\0';
	}
}

/* Returns a pointer to the start of first token. */
/* Also makes *endOfTok (if it's not NULL) to point at the last char after the token. */
char *getFirstTok(char *str, char **endOfTok)
{
	char *tokStart = str;
	char *tokEnd = NULL;

	/* Trim the start */
	trimLeftStr(&tokStart);

	/* Find the end of the first word */
	tokEnd = tokStart;
	while (*tokEnd != '\0' && !isspace(*tokEnd))
	{
		tokEnd++;
	}

	/* Add \0 at the end if needed */
	if (*tokEnd != '\0')
	{
		*tokEnd = '\0';
		tokEnd++;
	}

	/* Make *endOfTok (if it's not NULL) to point at the last char after the token */
	if (endOfTok)
	{
		*endOfTok = tokEnd;
	}
	

	return tokStart;
}


/* Returns if labelStr is a legal MION2 operand */
bool isIndex(char *line, int *value, int *value2)
{ 

	char *starts = strchr(line, '[');
	char *ends;
	

	if (starts != NULL)
	{
		*starts = '\0';
		starts++;
		if (isRegister(line,(int *) *&value))
		{
			
			ends = strchr(starts, ']');

			if (ends != NULL)
			{
				*ends = '\0';

				if (isRegister(starts,(int *) *&value2))
				{
					return TRUE;
				}
			}

		}
	}
	return FALSE;

}

/* Returns if labelStr is a legal label name. */
bool isLegalLabel(char *labelStr, int lineNum, bool printErrors)
{
	int labelLength = strlen(labelStr), i;

	/* Check if the label is short enough */
	if (strlen(labelStr) > MAX_LABEL_LENGTH)
	{
		if (printErrors) printf("ERROR: %d:Label is too long. Max label name length is %d.",lineNum, MAX_LABEL_LENGTH);
		return FALSE;
	}

	/* Check if the label isn't an empty string */
	if (*labelStr == '\0')
	{
		if (printErrors) printf("ERROR: %d: Label name is empty.",lineNum);
		return FALSE;
	}

	/* Check if the 1st char is a letter. */
	if (isspace(*labelStr))
	{
		if (printErrors) printf("ERROR: %d:Label must start at the start of the line.",lineNum);
		return FALSE;
	}

	/* Check if it's chars only. */
	for (i = 1; i < labelLength; i++)
	{
		if (!isalnum(labelStr[i]))
		{
			if (printErrors) printf("ERROR: %d:\"%s\" is illegal label - use letters and numbers only.",lineNum, labelStr);
			return FALSE;
		}
	}

	/* Check if the 1st char is a letter. */
	if (!isalpha(*labelStr))
	{
		if (printErrors) printf("ERROR: %d:\"%s\" is illegal label - first char must be a letter.",lineNum, labelStr);
		return FALSE;
	}

	/* Check if it's not a name of a register */
	if (isRegister(labelStr, NULL)) /* NULL since we don't have to save the register number */
	{
		if (printErrors) printf("ERROR: %d:\"%s\" is illegal label - don't use a name of a register.",lineNum, labelStr);
		return FALSE;
	}

	/* Check if it's not a name of a command */
	if (getCmdId(labelStr) != -1)
	{
		if (printErrors) printf("ERROR: %d:\"%s\" is illegal label - don't use a name of a command.",lineNum, labelStr);
		return FALSE;
	}

	return TRUE;
}

/* Returns if the label exists. */
bool isExistingLabel(char *label)
{
	if (getLabel(label))
	{
		return TRUE;
	}

	return FALSE;
}

/* Returns if the label is already in the entry lines array. */
bool isExistingEntryLabel(char *labelName)
{
	int i = 0;

	if (labelName)
	{
		for (i = 0; i < entryLabelsNum; i++)
		{
			if (strcmp(labelName, entryLines[i]->lineStr) == 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/* Returns if str is a register name, and update value to be the register value. */
bool isRegister(char *str, int *value)
{
	if (str[0] == 'r'  && str[1] >= '0' && str[1] - '0' <= MAX_REGISTER_DIGIT && str[2] == '\0') 
	{
		/* Update value if it's not NULL */
		if (value)
		{
			*value = str[1] - '0'; /* -'0' To get the actual number the char represents */
		}
		return TRUE;
	}

	return FALSE;
}


/* Returns a pointer to the start of the first operand in 'line' and change the end of it to '\0'. */
/* Also makes *endOfOp (if it's not NULL) point at the next char after the operand. */
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma)
{
	if (!isWhiteSpaces(line))
	{
		/* Find the first comma */
		char *end = strchr(line, ',');
		if (end)
		{
			*foundComma = TRUE;
			*end = '\0';
			end++;
		}
		else
		{
			*foundComma = FALSE;
		}

		/* Set endOfOp (if it's not NULL) to point at the next char after the operand
		(Or at the end of it if it's the end of the line) */
		if (endOfOp)
		{
			if (end)
			{
				*endOfOp = end;
			}
			else
			{
				*endOfOp = strchr(line, '\0');
			}
		}
	}

	trimStr(&line);
	return line;
}

/* Returns if the cmd is a directive. */
bool isDirective(char *cmd)
{
	return (*cmd == '.') ? TRUE : FALSE;
}

/* Returns if the strParam is a legal string param (enclosed in quotes), and remove the quotes. */
bool isLegalStringParam(char **strParam, int lineNum)
{
	/* check if the string param is enclosed in quotes */
	if ((*strParam)[0] == '"' && (*strParam)[strlen(*strParam) - 1] == '"')
	{
		/* remove the quotes */
		(*strParam)[strlen(*strParam) - 1] = '\0';
		++*strParam;
		return TRUE;
	}

	if (**strParam == '\0')
	{
		printf("ERROR: %d:No parameter.",lineNum);
	}
	else
	{
		printf("ERROR: %d:The parameter for .string must be enclosed in quotes.",lineNum);
	}
	return FALSE;
}

/* Returns if the num is a legal number param, and save it's value in *value. */
bool isLegalNum(char *numStr, int numOfBits, int lineNum, int *value)
{
	char *endOfNum;
	/* maxNum is the max number you can represent with (MAX_LABEL_LENGTH - 1) bits 
	 (-1 for the negative/positive bit) */
	int maxNum = (1 << numOfBits) - 1;

	if (isWhiteSpaces(numStr))
	{
		printf("ERROR: %d:Empty parameter.",lineNum);
		return FALSE;
	}

	*value = strtol(numStr, &endOfNum, 0);

	/* Check if endOfNum is at the end of the string */
	if (*endOfNum)
	{
		printf("ERROR: %d:\"%s\" isn't a valid number.",lineNum, numStr);
		return FALSE;
	}

	/* Check if the number is small enough to fit into 1 memory word 
	(if the absolute value of number is smaller than 'maxNum' */
	if (*value > maxNum || *value < -maxNum)
	{
		printf("ERROR: %d:\"%s\" is too %s, must be between %d and %d.",lineNum, numStr, (*value > 0) ? "big" : "small", -maxNum, maxNum);
		return FALSE;
	}

	return TRUE;
}
