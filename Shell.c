#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Max command prompt length
#define MAX_LEN 1024
#define ARG_AMOUNT 5

typedef struct List
{
	char** vect;
	struct List* next;
} P_List;												// list of vectorized names and arguements for simple commands

void ParsePrompt();
void ParseIfCommand();
int ParseCommand();
P_List* ParseConveyor();
P_List* ParseSimpleCommand();

int LaunchConveyor();
void CheckExitStatus();

void SignalHandler();

char* ReadPrompt();
char* ExtractCommand();
void PrintList();
int GetProcNum();

// Background process execution
void SignalHandler(int s)
{
	int stat;
	wait(&stat);
	
	if(WIFEXITED(stat))
	{
		printf("Background process finished\n");
	}
}

// DEBUG: Print the list of names and arguements
void PrintList(P_List* list)
{
	int i = 0;
	printf("SIMPLE COMAND VECTOR: ");
	while(list->vect[i] != NULL)
	{
		printf("%d: {%s}; ", i, list->vect[i]);
		i ++;
	}
	printf("%d: {NULL}; \n", i);
	if(list->next != NULL)
	{
		PrintList(list->next);
	}
	return;
}

// Extract a single command from the command prompt
// based on its starting and ending position
char* ExtractCommand(char* Line, int start, int fin)
{
	char* New_Line = malloc(MAX_LEN);
	
	// Remove space in the beginning, if necessary
	if(Line[start] == ' ')
	{
		start++;
	}
	// Remove space in the beginning, if necessary
	if(Line[fin - 1] == ' ')
	{
		fin--;
	}
	// If starting and ending positions are equal, the command is empty
	if(start == fin)
	{
		New_Line = NULL;
	}
	else
	{
		int i = 0;
		int j = start;
		
		while(j < fin)
		{
			New_Line[i] = Line[j];
			i++;
			j++;
		}
	}
	return New_Line;
}

// Read command prompt (removing excessive space where necessary)
char* ReadPrompt(void)
{
	char c1, c2;
	int pos = 0;
	char* buf = malloc(sizeof(char) * MAX_LEN);
	
	c1 = getchar();
	while(c1 != '\n')
	{
		if (c2 == ' ')
		{
			// If there is more than one space, remove excessive
			while((c1 != '\n') && (c1 == ' '))
			{
				c1 = getchar();
			}
		}
		if(c1 != '\n')
		{
			buf[pos] = c1;
			pos ++;
		
			c2 = c1;
			c1 = getchar();
		}
	}
	buf[++pos] = '\0';
	
	// If length of the line exceed max length, report error
	if(pos >= MAX_LEN)
	{
		printf("Error: the command line is too long\n");
		exit(0);
	}
	
	return buf;
}

// Count the number of processes in the conveyor
int GetProcNum(P_List* Vectors_List)
{
	int k = 0;
	if(Vectors_List->next != NULL)
	{
		k = GetProcNum(Vectors_List->next);
	}
	k++;
	return k;
}

// Check process exit status
void CheckExitStatus(int stat, int k)
{
	if (WIFEXITED(stat)) 
	{
		printf("Process %d finished: %d\n", k, WEXITSTATUS(stat));
	}
	else
	{
		printf("Error in process %d\n", k);
	}
	return;
}


int LaunchConveyor(P_List* Vectors, int n, char* Redir_In, char* Redir_Out, char* Redir_Out_Plus, int Background)
{
	pid_t pid;
	char** vector;
	int stat[n - 1];
	int stat_final;
	
	int input_file;
	int output_file;
	int pd[n - 1][2];													// n - 1 pipes for n processes
	
	for (int i = 0; i < n; i ++)
	{
		if (i != n - 1)
		{
			// Start a new pipe for the process to write data in
			pipe(pd[i]);
		}
		vector = Vectors->vect;
		Vectors = Vectors->next;
	
		pid = fork();
		
		// Child process	
		if (pid == 0)
		{
			if (i != 0)
			{
				dup2(pd[i - 1][0], 0);
			
				close(pd[i - 1][0]);
				close(pd[i - 1][1]);
			}
			else if (Background != 0)
			{
				int nul;
				if ((nul = open("/dev/null", O_RDONLY)) == -1)
				{
						printf("Error: File '%s' doesn't exist\n", Redir_In);
				}
				else
				{
					dup2(nul, 0);
				}
			}
			else
			{
				// Input redirection
				if (Redir_In != NULL)
				{
					if ((input_file = open(Redir_In, O_RDONLY)) == -1)
					{
						printf("Error: File '%s' doesn't exist\n", Redir_In);
					}
					else
					{
						dup2(input_file, 0);
					}
				}
			}
			if (i != n - 1)
			{
				dup2(pd[i][1], 1);
				
				close(pd[i][1]);
				close(pd[i][0]);
			}
			else
			{
				// Output redirection (clear previous data from file before writing)
				if (Redir_Out != NULL)
				{
					if ((output_file = open(Redir_Out, O_WRONLY | O_TRUNC)) == -1)
					{
						output_file = creat(Redir_Out, S_IWRITE | S_IREAD);
					}
					dup2(output_file, 1);
				}
				
				// Output redirection (write to the end of file)
				if (Redir_Out_Plus != NULL)
				{
					if ((output_file = open(Redir_Out_Plus, O_WRONLY | O_APPEND)) == -1)
					{
						output_file = creat(Redir_Out_Plus, S_IWRITE | S_IREAD);
					}
					dup2(output_file, 1);
				}
			}
			execvp(vector[0], vector);
		}
		
		// Parent process
		if(i != 0)
		{
			// Close pipe the process received data from (all except the first process)
			close(pd[i - 1][0]);
			close(pd[i - 1][1]);
		}
		
		if (i == n - 1)
		{
			wait(&stat_final);
		}
	}
	for (int j = 0; j < n - 1; j++)
	{
		wait(&stat[j]);
	}
	return WEXITSTATUS(stat_final);
}


P_List* ParseSimpleCommand(char* com)
{
	int pos = 0;
	int start = pos;
	int count = 1;
	int amount = ARG_AMOUNT;
	
	P_List* Process = malloc(sizeof(P_List));
	Process->vect = malloc(amount * sizeof(char*));
	Process->next = NULL;
	
	while ((com[pos] != ' ') && (com[pos] != '\0'))
	{
		pos++;
	}
	Process->vect[0] = ExtractCommand(com, start, pos);
	
	start = pos + 1;
		
	if (com[pos] == ' ')
	{
		pos++;
		while (com[pos] != '\0')
		{
			if (com[pos] == ' ')
			{
				if (count >= amount)
				{
					amount += ARG_AMOUNT;
					Process->vect = realloc(Process->vect, amount * sizeof(char*));
				}
				Process->vect[count] = ExtractCommand(com, start, pos);
				
				start = pos + 1; 
				count++;
			}
			pos++;
		}
		
		Process->vect[count] = ExtractCommand(com, start, pos);
		count++;
	}	
	Process->vect[count] = NULL;

	return Process;
}

// Parse conveyor
P_List* ParseConveyor(char* Line)
{	
	P_List* Vectors_List;
	
	char* Simple_Comand;
	char* New_Line;
	int pos = 0;
	int start = pos;
	
	while ((Line[pos] != '\0') && (Line[pos] != '|'))
	{
		pos++;
	}	
	Simple_Comand = ExtractCommand(Line, start, pos);
	Vectors_List = ParseSimpleCommand(Simple_Comand);
	
	if (Line[pos] == '|')
	{
		start = pos + 1;
		while (Line[pos] != '\0')
		{
			pos ++;
		}
		New_Line = ExtractCommand(Line, start, pos);
		Vectors_List->next = ParseConveyor(New_Line);
	}
	return Vectors_List;
}

// Parse 'if' command
void ParseIfCommand(char* Line, int Background)
{
	int code;															// execution process exit code
	char* New_Line;
	int pos = 0;
	int start = pos;
	
	while (Line[pos] != '\0')
	{
		if ((Line[pos] == '&') && (Line[pos+1] == '&'))
		{
			New_Line = ExtractCommand(Line, start, pos);
			code = ParseCommand(New_Line, Background);
			
			start = pos + 2;
			
			if(code != 0)
			{
				return;
			}
		}
		if ((Line[pos] == '|') && (Line[pos+1] == '|'))
		{
			
			New_Line = ExtractCommand(Line, start, pos);
			code = ParseCommand(New_Line, Background);
			
			start = pos + 2;
			
			if (code == 0)
			{
				return;
			}
		}
		pos ++;
	}
	New_Line = ExtractCommand(Line, start, pos);
	code = ParseCommand(New_Line, Background);
	
	return;
}

// Parse command prompt
void ParsePrompt(char* Line, int Background)
{
	int pid;
	int b_m = 1;
	
	char* New_Line;
	int pos = 0;
	int start = pos;
	
	signal(SIGUSR1, SignalHandler);
	
	while (Line[pos] != '\0')
	{		
		if ((Line[pos - 1] != '&') && (Line[pos] == '&') && (Line[pos + 1] != '&'))
		{
			New_Line = ExtractCommand(Line, start, pos);
			
			pid = fork();
			if(pid == 0)
			{
				printf("Background process [%d]: %d\n", b_m, getpid());
				
				signal(SIGINT, SIG_IGN);
				signal(SIGQUIT, SIG_IGN);
				
				Background++;
				
				ParsePrompt(New_Line, Background);
				
				kill(getppid(), SIGUSR1);
				exit(0);
			}
			b_m++;
			start = pos + 1;
		}
		
		if (Line[pos] == ';')
		{			
			New_Line = ExtractCommand(Line, start, pos);
			ParseIfCommand(New_Line, Background);
			start = pos + 1;
		}
		pos++;
	}
	New_Line = ExtractCommand(Line, start, pos);
	if (New_Line != NULL)
	{
		ParseIfCommand(New_Line, Background);
	}	
	return;
}

// Parse command
int ParseCommand(char* Line, int Background)
{	
	P_List* Vectors_List;
	
	int code;															// Conveyor exit code
	char* New_Line;
	int pos = 0;
	int start = pos;
	int Amount;
	
	char* R_I_Filename = NULL;
	char* R_O_Filename = NULL;
	char* R_O_P_Filename = NULL;
	
	while (Line[pos] != '\0')
	{
		if (Line[pos] =='(')
		{
			start = pos + 1;
			
			while (Line[pos] != ')')
			{
				pos ++;
			}
			New_Line = ExtractCommand(Line, start, pos);
			ParsePrompt(New_Line, Background);	
			
			start = pos + 1;
		}
		
		if (Line[pos] == '>')
		{
			New_Line = ExtractCommand(Line, start, pos);
			
			start = pos + 1;
			pos++;
			if (Line[pos] == '>')
			{
				// Save input redirection type and file name to be used in the conveyor
				start++;
				pos++;
				if (Line[pos] == ' ')
				{
					pos++;
				}
				while ((Line[pos] != ' ') && (Line[pos] != '\0'))
				{
					pos++;
				}
				R_O_P_Filename = ExtractCommand(Line, start, pos);
			}
			else
			{
				if (Line[pos] == ' ')
				{
					pos++;
				}
				while ((Line[pos] != ' ') && (Line[pos] != '\0'))
				{
					pos++;
				}
				R_O_Filename = ExtractCommand(Line, start, pos);
			}
			start = pos + 1;
			
			if (New_Line != NULL)
			{
				Vectors_List = ParseConveyor(New_Line);
//				PrintList(Vectors_List);
				Amount = GetProcNum(Vectors_List);
//				printf("Amount of processes: %d\n", Amount);
				code = LaunchConveyor(Vectors_List, Amount, R_I_Filename, R_O_Filename, R_O_P_Filename, Background);
				
				R_I_Filename = NULL;
				R_O_Filename = NULL;
				R_O_P_Filename = NULL;
			}
		}
		
		if (Line[pos] == '<')
		{
			// Save output redirection type and file name to be used in the conveyor
			start = pos + 1;
			pos++;
			if (Line[pos] == ' ')
			{
				pos++;
			}
			while ((Line[pos] != ' ') && (Line[pos] != '\0'))
			{
				pos++;
			}
			R_I_Filename = ExtractCommand(Line, start, pos);
			
			start = pos + 1;
		}
		pos++;
	}
	
	New_Line = ExtractCommand(Line, start, pos);
	if (New_Line != NULL)
	{
		Vectors_List = ParseConveyor(New_Line);
//		PrintList(Vectors_List);
		Amount = GetProcNum(Vectors_List);
//		printf("Amount of processes: %d\n", Amount);
		code = LaunchConveyor(Vectors_List, Amount, R_I_Filename, R_O_Filename, R_O_P_Filename, Background);
		
		R_I_Filename = NULL;
		R_O_Filename = NULL;
		R_O_P_Filename = NULL;
	}
	return code;
}


int main(int argc, char **argv)
{
	char* prompt = NULL;
	
	// Infinite loop
	for (;;)
	{

		printf("> ");													// Print command prompt starter
		prompt = ReadPrompt();											// Read command prompt and remove exessive space
		
		if((strcmp(prompt, "") != 0) && (strcmp(prompt, " ") != 0))		// If a command prompt is not an empty line or spaces:
		{
			ParsePrompt(prompt, 0);										//   Parse command prompt
		}
	}
	
	return 0;
}
