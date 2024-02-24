#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LEN 1024													// Max command line length
#define ARG_AMOUNT 5

typedef struct List
{
	char** vect;
	struct List* next;
} P_List;																// Vectorised list of simple commands' names and arguements

// Shell command line analysis
void Shell_Comand_Analysis();
void Comand_If_Analysis();
int Comand_Analysis();
P_List* Conveyor_Analysis();
P_List* Simple_Comand_Analysis();

// Converyor + input/output redirection
int Conveyor_Launch();
void Exit_status();

// Background processes
void Sig_handler();

// Various
char* input_line();
char* split_line();
void print_list();
int Proc_Amount();


void Sig_handler(int s)
{
	int stat;
	wait(&stat);
	
	if(WIFEXITED(stat))
		printf("Background process finished\n");
}


// Vectors list printing  (for debugging)
void print_list(P_List* list)
{
	int i = 0;
	
	printf("SIMPLE COMAND VECTOR: ");
	while(list->vect[i] != NULL)
	{
		printf("%d: {%s}; ", i, list->vect[i]);
		i++;
	}
	printf("%d: {NULL}; \n", i);
	
	if(list->next != NULL)
		print_list(list->next);
		
	return;
}


// Split a single command from command line based on the commands start and end
// (deleting first and final spaces if necessary)
char* split_line(char* Line, int start, int fin)
{
	char* New_Line = malloc(MAX_LEN);
	
	if(Line[start] == ' ')
		start++;
	if(Line[fin-1] == ' ')
		fin--;
	
	if(start == fin)
		New_Line = NULL;
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


// Reafing command line reducing all consecutive spaces to one
char* input_line(void)
{
	char c1;															// Current character of command line
	char c2;															// Previous character of command line
	int pos = 0;
	char* buf = malloc(sizeof(char) * MAX_LEN);
	
	c1 = getchar();
	while(c1 != '\n')
	{
		if (c2 == ' ')
			while((c1 != '\n') && (c1 == ' '))
				c1 = getchar();
		
		if(c1 != '\n')
		{
			buf[pos] = c1;
			pos++;
		
			c2 = c1;
			c1 = getchar();
		}
	}
	buf[++pos] = '\0';
	
	if(pos >= MAX_LEN)
	{
		printf("Error: line is too long\n");
		exit(0);
	}
	
	return buf;
}


// Counting amount of processes in the conveyor
int Proc_Amount(P_List* Vectors_List)
{
	int k = 0;
	if(Vectors_List->next != NULL)
		k = Proc_Amount(Vectors_List->next);
		
	k++;
	return k;
}


// Checking if the program was executed successfully
void Exit_status(int stat, int k)
{
	if (WIFEXITED(stat)) 
		printf("Process %d finished: %d\n", k, WEXITSTATUS(stat));
	else printf("Error in process %d\n", k);
	
	return;
}


int Conveyor_Launch(P_List* Vectors, int n, char* Redir_In, char* Redir_Out, char* Redir_Out_Plus, int Background)
{
	pid_t pid;
	char** vector;
	int stat[n-1];
	int stat_final;
	
	int input_file;
	int output_file;
	int pd[n-1][2];														// n-1 pipes connecting n processes
	
	// printf("Background = %d\n", Background);
	
	for(int i = 0; i < n; i++)
	{
		if(i != n-1)
			pipe(pd[i]);												// Create new pipe the process will write data in (all processes except the last one)

		vector = Vectors->vect;
		Vectors = Vectors->next;
	
		pid = fork();
		
		// Child process
		if(pid == 0)
		{	
			if(i != 0)
			{
				dup2(pd[i-1][0], 0);
			
				close(pd[i-1][0]);
				close(pd[i-1][1]);
			}
			
			else if(Background != 0)
			{
				int nul;
				if((nul = open("/dev/null", O_RDONLY)) == -1)
						printf("Error: File '%s' doesn't exist\n", Redir_In);
				else
					dup2(nul, 0);
			}
			
			else
				if(Redir_In != NULL)									// Redirecting input
				{
					if((input_file = open(Redir_In, O_RDONLY)) == -1)
						printf("Error: File '%s' doesn't exist\n", Redir_In);
					else
						dup2(input_file, 0);
				}
			
			if(i != n-1)
			{
				dup2(pd[i][1], 1);
				
				close(pd[i][1]);
				close(pd[i][0]);
			}
			else
			{
				if(Redir_Out != NULL)									// Redirecting output (overwriting file)
				{
					if((output_file = open(Redir_Out, O_WRONLY | O_TRUNC)) == -1)
						output_file = creat(Redir_Out, S_IWRITE | S_IREAD);
						
					dup2(output_file, 1);
				}
				
				if(Redir_Out_Plus != NULL)								// Redirecting output (writing to the end of file)
				{
					if((output_file = open(Redir_Out_Plus, O_WRONLY | O_APPEND)) == -1)
						output_file = creat(Redir_Out_Plus, S_IWRITE | S_IREAD);
		
					dup2(output_file, 1);
				}
			}
			execvp(vector[0], vector);
		}
		
		// Parent process
		if(i != 0)
		{
			close(pd[i-1][0]);											// Close the pipe that process retreived data from
			close(pd[i-1][1]);											// Close all the pipes that won't be used (all processes except the first one)
		}
		
		if(i == n-1)
			wait(&stat_final);
	}
	
	for (int j = 0; j < n-1; j++)
		wait(&stat[j]);

	return WEXITSTATUS(stat_final);
	
}


P_List* Simple_Comand_Analysis(char* com)
{
	int pos = 0;
	int start = pos;
	int count = 1;
	int amount = ARG_AMOUNT;
	
	P_List* Process = malloc(sizeof(P_List));
	Process->vect = malloc(amount * sizeof(char*));
	Process->next = NULL;
	
	while((com[pos] != ' ') && (com[pos] != '\0'))
		pos++;
	
	Process->vect[0] = split_line(com, start, pos);
	
	start = pos + 1;
		
	if(com[pos] == ' ')
	{
		pos++;
		while(com[pos] != '\0')
		{
			if(com[pos] == ' ')
			{
				if(count >= amount)
				{
					amount += ARG_AMOUNT;
					Process->vect = realloc(Process->vect, amount * sizeof(char*));
				}
				Process->vect[count] = split_line(com, start, pos);
				
				start = pos + 1; 
				count++;
			}
			pos++;
		}
		
		Process->vect[count] = split_line(com, start, pos);
		count++;
	}	
	Process->vect[count] = NULL;

	return Process;
}


// Analyse the conveyor
P_List* Conveyor_Analysis(char* Line)
{	
	P_List* Vectors_List;
	
	char* Simple_Comand;
	char* New_Line;
	int pos = 0;
	int start = pos;
	
	while((Line[pos] != '\0') && (Line[pos] != '|'))
		pos++;
		
	Simple_Comand = split_line(Line, start, pos);
	Vectors_List = Simple_Comand_Analysis(Simple_Comand);
	
	if(Line[pos] == '|')
	{
		start = pos + 1;
		
		while(Line[pos] != '\0')
			pos++;
		
		New_Line = split_line(Line, start, pos);
		Vectors_List->next = Conveyor_Analysis(New_Line);
	}
	
	return Vectors_List;
}


// Analyse command with 'if' condition
void Comand_If_Analysis(char* Line, int Background)
{
	int code;															// Code of the command's end
	char* New_Line;
	int pos = 0;
	int start = pos;
	
	while(Line[pos] != '\0')
	{
		if((Line[pos] == '&') && (Line[pos+1] == '&'))
		{
			New_Line = split_line(Line, start, pos);
			code = Comand_Analysis(New_Line, Background);
			
			start = pos + 2;
			
			if(code != 0)
				return;
		}
		
		if((Line[pos] == '|') && (Line[pos+1] == '|'))
		{
			
			New_Line = split_line(Line, start, pos);
			code = Comand_Analysis(New_Line, Background);
			
			start = pos + 2;
			
			if(code == 0)
				return;
		}
		
		pos++;
	}
	
	New_Line = split_line(Line, start, pos);
	code = Comand_Analysis(New_Line, Background);
	
	return;
}


// Analyse Shell command
void Shell_Comand_Analysis(char* Line, int Background)
{
	int pid;
	int b_m = 1;
	
	char* New_Line;
	int pos = 0;
	int start = pos;
	
	signal(SIGUSR1, Sig_handler);
	
	while(Line[pos] != '\0')
	{		
		if((Line[pos - 1] != '&') && (Line[pos] == '&') && (Line[pos + 1] != '&'))
		{
			New_Line = split_line(Line, start, pos);
			
			pid = fork();
			if(pid == 0)
			{
				printf("Background process [%d]: %d\n", b_m, getpid());
				
				signal(SIGINT, SIG_IGN);
				signal(SIGQUIT, SIG_IGN);
				
				Background++;
				
				Shell_Comand_Analysis(New_Line,Background);
				
				kill(getppid(), SIGUSR1);
				exit(0);
			}
			b_m++;
			
			start = pos + 1;
		}
		
		if(Line[pos] == ';')
		{			
			New_Line = split_line(Line, start, pos);
			Comand_If_Analysis(New_Line, Background);
			
			start = pos + 1;
		}
		
		pos++;
	}
	New_Line = split_line(Line, start, pos);
	if(New_Line != NULL)
		Comand_If_Analysis(New_Line, Background);
		
	return;
}


int Comand_Analysis(char* Line, int Background)
{	
	P_List* Vectors_List;
	
	int code;															// Code of the conveyor's end
	char* New_Line;
	int pos = 0;
	int start = pos;
	int Amount;
	
	char* R_I_Filename = NULL;
	char* R_O_Filename = NULL;
	char* R_O_P_Filename = NULL;
	
	while(Line[pos] != '\0')
	{
		if(Line[pos] =='(')
		{
			start = pos + 1;
			
			while(Line[pos] != ')')
				pos++;
			
			New_Line = split_line(Line, start, pos);
			Shell_Comand_Analysis(New_Line, Background);	
			
			start = pos + 1;
		}
		
		if(Line[pos] == '>')
		{
			New_Line = split_line(Line, start, pos);
			
			start = pos + 1;
			
			pos++;
			if(Line[pos] == '>')
			{
				// Save the output redirection and the file name to use them in the conveyor
				start++;
				pos++;
				if(Line[pos] == ' ')
					pos++;
				
				while((Line[pos] != ' ') && (Line[pos] != '\0'))
				pos++;
				
				R_O_P_Filename = split_line(Line, start, pos);
			}
			
			else
			{
				if(Line[pos] == ' ')
					pos++;
				
				while((Line[pos] != ' ') && (Line[pos] != '\0'))
				pos++;
				
				R_O_Filename = split_line(Line, start, pos);
			}
			start = pos + 1;
			
			if(New_Line != NULL)
			{
				Vectors_List = Conveyor_Analysis(New_Line);
				// print_list(Vectors_List);
				Amount = Proc_Amount(Vectors_List);
				// printf("Amount of processes: %d\n", Amount);
				code = Conveyor_Launch(Vectors_List, Amount, R_I_Filename, R_O_Filename, R_O_P_Filename, Background);
				
				R_I_Filename = NULL;
				R_O_Filename = NULL;
				R_O_P_Filename = NULL;
			}
		}
		
		if(Line[pos] == '<')
		{
			// Save the input redirection and the file name to use them in the conveyor
			start = pos + 1;
			pos++;
			if(Line[pos] == ' ')
				pos++;
			
			while((Line[pos] != ' ') && (Line[pos] != '\0'))
			pos++;
			
			R_I_Filename = split_line(Line, start, pos);
			
			start = pos + 1;
		}
		
		pos++;
	}
	
	New_Line = split_line(Line, start, pos);
	if(New_Line != NULL)
	{
		Vectors_List = Conveyor_Analysis(New_Line);
		 // print_list(Vectors_List);
		Amount = Proc_Amount(Vectors_List);
		// printf("Amount of processes: %d\n", Amount);
		code = Conveyor_Launch(Vectors_List, Amount, R_I_Filename, R_O_Filename, R_O_P_Filename, Background);
		
		R_I_Filename = NULL;
		R_O_Filename = NULL;
		R_O_P_Filename = NULL;
	}
	
	return code;
}


int main(int argc, char **argv)
{
	char* line = NULL;
	
	for (;;)															// infinite loop
	{
		printf("> ");
		line = input_line();											// read command line and delete excessive spaces
		
		if((strcmp(line, "") != 0) && (strcmp(line, " ") != 0))			// if the line is not empty and does not consist entirely of spaces
			Shell_Comand_Analysis(line, 0);								//   analyse command line

	}
	
	return 0;
}
