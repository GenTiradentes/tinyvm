#include "tvm_file.h"
#include "tvm_program.h"

#define MAX_ARGS 2
#define MAX_TOKENS 4

static void tokenize_line(char** tokens, char* line);
static void parse_labels(program* p, char** tokens);
static void parse_instructions(program* p, char** tokens, memory* pMemory);

program* create_program(char* filename, memory* pMemory)
{
	// Open our file in read-only mode
	FILE* pFile = tvm_openfile(filename, ".vm", "r");

	if(!pFile)
	{
		printf("File was not found, or does not exist. Unable to interpret.\n");
		return NULL;
	}

	// Allocate space for our program
	program* p = (program*)malloc(sizeof(program));

	// Initialize the members of program
	p->start = 0;

	p->instr = 0;
	p->num_instructions = 0;

	p->args = 0;

	p->values = 0;
	p->num_values = 0;

	// Create our label hash table
	p->label_htab = create_htab();

	// Create lookup table
	p->lookup_table = create_htab();

	char** tokens = malloc(sizeof(char*) * MAX_TOKENS);

	int i;
	for(i = 0; i < MAX_TOKENS; i++)
	{
		tokens[i] = malloc(sizeof(char) * 32);
		memset(tokens[i], 0, sizeof(char) * 32);
	}

	char line[128] = {0};

	// Get one line from the source file
	while(fgets(line, 128, pFile))
	{
		p->instr = realloc(p->instr, sizeof(int) * (p->num_instructions + 1));
		p->args = realloc(p->args, sizeof(int**) * (p->num_instructions + 1));
		p->args[p->num_instructions] = malloc(sizeof(int*) * MAX_ARGS);

		tokenize_line(tokens, line);
		/*parse_labels(p, tokens);*/
		/*parse_labels(p, tokens);
		parse_line(p, tokens, pMemory);
	}

	for(i = 0; i < MAX_TOKENS; i++)
		if(tokens[i]) free(tokens[i]);

	if(tokens) free(tokens);

	// Specify the end of the program
	p->instr = realloc(p->instr, sizeof(int) * (p->num_instructions + 1));
	p->instr[p->num_instructions] = END;

	fclose(pFile);
	return p;
}

void destroy_program(program* p)
{
	destroy_htab(p->label_htab);

	int i;
	for(i = 0; i < p->num_values; i++) free(p->values[i]);
	free(p->values);

	for(i = 0; i < p->num_instructions; i++) free(p->args[i]);
	free(p->args);

	free(p->instr);
	free(p);
}

void tokenize_line(char** tokens, char* line)
{
	int token_idx = 0;
	char* pToken;

	pToken = strtok(line, "	 ,");

	while(pToken)
	{
		// Ignore comments
		char* comment_delimiter = strchr(pToken, '#');

		if(comment_delimiter)
			*comment_delimiter = 0;

		if(pToken)
			strcpy(tokens[token_idx], pToken);

		if(comment_delimiter) break;
		else pToken = strtok(NULL, "	 ,");

		++token_idx;
	}
}

void parse_line(program* p, char** tokens, memory *pMemory)
{
   parse_labels(p,tokens);
   parse_instructions(p,tokens,pMemory);
}

void parse_labels(program* p, char** tokens)
{
	int token_idx;

	for(token_idx = 0; token_idx < MAX_TOKENS; token_idx++)
	{
		// Figure out if the token we're dealing with is a label
		char* label_delimiter = strchr(tokens[token_idx], ':');

		if(label_delimiter != NULL)
		{
			// Get rid of the label delimiter
			*label_delimiter = 0;

			// If the label is "start," set the program to begin there.
			if(strcmp(tokens[token_idx], "start") == 0) p->start = p->num_instructions;

			// Add that fucker to the hash table with the corresponding instruction index
			htab_add(p->label_htab, tokens[token_idx], p->num_instructions);

			// Advance to the next token
			++token_idx;
		}
	}
}

void parse_instructions(program* p, char** tokens, memory* pMemory)
{
	int token_idx;
	for(token_idx = 0; token_idx < MAX_TOKENS; token_idx++)
	{
		// Figure out if the token we're dealing with is an opcode
		int valid_opcode = 1;

		if(strcmp(tokens[token_idx], "mov") == 0)	p->instr[p->num_instructions] = MOV;
		else if(strcmp(tokens[token_idx], "push") == 0)	p->instr[p->num_instructions] = PUSH;
		else if(strcmp(tokens[token_idx], "pop") == 0)	p->instr[p->num_instructions] = POP;
		else if(strcmp(tokens[token_idx], "inc") == 0)	p->instr[p->num_instructions] = INC;
		else if(strcmp(tokens[token_idx], "dec") == 0)	p->instr[p->num_instructions] = DEC;
		else if(strcmp(tokens[token_idx], "add") == 0)	p->instr[p->num_instructions] = ADD;
		else if(strcmp(tokens[token_idx], "sub") == 0)	p->instr[p->num_instructions] = SUB;
		else if(strcmp(tokens[token_idx], "mul") == 0)	p->instr[p->num_instructions] = MUL;
		else if(strcmp(tokens[token_idx], "div") == 0)	p->instr[p->num_instructions] = DIV;
		else if(strcmp(tokens[token_idx], "mod") == 0)	p->instr[p->num_instructions] = MOD;
		else if(strcmp(tokens[token_idx], "rem") == 0)	p->instr[p->num_instructions] = REM;
		else if(strcmp(tokens[token_idx], "not") == 0)	p->instr[p->num_instructions] = NOT;
		else if(strcmp(tokens[token_idx], "xor") == 0)	p->instr[p->num_instructions] = XOR;
		else if(strcmp(tokens[token_idx], "or") == 0)	p->instr[p->num_instructions] = OR;
		else if(strcmp(tokens[token_idx], "and") == 0)	p->instr[p->num_instructions] = AND;
		else if(strcmp(tokens[token_idx], "shl") == 0)	p->instr[p->num_instructions] = SHL;
		else if(strcmp(tokens[token_idx], "shr") == 0)	p->instr[p->num_instructions] = SHR;
		else if(strcmp(tokens[token_idx], "cmp") == 0)	p->instr[p->num_instructions] = CMP;
		else if(strcmp(tokens[token_idx], "jmp") == 0)	p->instr[p->num_instructions] = JMP;
		else if(strcmp(tokens[token_idx], "je") == 0)	p->instr[p->num_instructions] = JE;
		else if(strcmp(tokens[token_idx], "jne") == 0)	p->instr[p->num_instructions] = JNE;
		else if(strcmp(tokens[token_idx], "jg") == 0)	p->instr[p->num_instructions] = JG;
		else if(strcmp(tokens[token_idx], "jge") == 0)	p->instr[p->num_instructions] = JGE;
		else if(strcmp(tokens[token_idx], "jl") == 0)	p->instr[p->num_instructions] = JL;
		else if(strcmp(tokens[token_idx], "jle") == 0)	p->instr[p->num_instructions] = JLE;
		else if(strcmp(tokens[token_idx], "pri") == 0)	p->instr[p->num_instructions] = PRI;
		else
			valid_opcode = 0;

		// If it *is* an opcode, parse the arguments
		if(valid_opcode)
		{
			int i, num_instr = p->num_instructions;
			++p->num_instructions;

			for(i = ++token_idx; i < (token_idx + 2); ++i)
			{
				// If the token is empty, do not attempt to parse it
				if(strlen(tokens[i]) <= 0) continue;

				// Remove any trailing newline from the token
				char* newline = strchr(tokens[i], '\n');
				if(newline) *newline = 0;

				// Check to see whether the token specifies an address
				if(tokens[i][0] == '[')
				{
					char* end_symbol = strchr(tokens[i], ']');
					if(end_symbol) *end_symbol = 0;
					int add = parse_value(tokens[i] + 1);
					if(add > pMemory->max)
					{
					     pMemory->max = add;
					}
					p->args[num_instr][i - token_idx] = &pMemory->int32[add];
					continue;
				}
				// If it's not an address, check if the argument is a label
				else
				{
					// Search the hash map for the label
					int instr_idx = htab_find(p->label_htab, tokens[i]);

					// If the label was found, create the argument
					if(instr_idx >= 0)
					{
						p->args[num_instr][i - token_idx] = add_value(p, instr_idx);
						continue;
					}else if((tokens[i][0] > 64 && tokens[i][0] <= 90) 
|| (tokens[i][0] > 96 && tokens[i][0] <=122)){
                                                int* mem = htab_find_var(p->lookup_table,tokens[i],0);
                                                if(mem==NULL){
                                                        htab_add_var(p->lookup_table,tokens[i],&pMemory->int32[pMemory->max],0);
                                                        p->args[num_instr][i - token_idx] = &pMemory->int32[pMemory->max];
                                                        pMemory->max = pMemory->max+1;
                                                }else{
                                                        p->args[num_instr][i - token_idx] = mem;
                                                }
                                                continue;

}
				}

				// If it's not an address, and it's not a label, parse it as a value
				int value = parse_value(tokens[i]);
				p->args[num_instr][i - token_idx] = add_value(p, value);
			}
		}
	}
}

int* add_value(program* p, const int val)
{
	p->values = realloc(p->values, sizeof(int*) * (p->num_values + 1));
	p->values[p->num_values] = malloc(sizeof(int));

	*p->values[p->num_values] = val;

	return p->values[p->num_values++];
}

int parse_value(char* str)
{
	char* delimiter = strchr(str, ':');
	int base = 0;

	if(delimiter)
	{
		char* identifier = delimiter + 1;

		switch(*identifier)
		{
			case 'h':
				base = 16;
				break;
			case 'b':
				base = 2;
				break;
			default:
				base = 0;
				break;
		}
	}

	return strtoul(str, NULL, base);
}
