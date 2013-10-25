#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char *argv[])
{

    FILE *in_file;
    int i;

    // exit if not provided with exactly one command-line parameter
    if (argc != 2) 
    {
	printf("Usage: %s <filename>\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    // get number of parellel processes to utilize
    int num_proc;
    printf("Enter the number of parallel processes to utilize: (1, 2, or 4)\n");
    scanf("%d", &num_proc);

    // open file for reading
    in_file = fopen(argv[1], "r");
    if (in_file == NULL)
    {
	printf("Error: Could not open file.\n");
	exit(EXIT_FAILURE);
    }

    // initialize child pipes
    int pipe_fds[num_proc][2];
    for (i = 0; i < num_proc; i++)
    {
	pipe(pipe_fds[i]);
    }

    // initialize parent pipe
    int parent_pipe[2];
    pipe(parent_pipe);

    // spawn requested number of processes
    for (i = 0; i < num_proc; i++)
    {
	pid_t child_pid;
	child_pid = fork();
	if (child_pid == 0)
	{
	    // let child do its thing
	    int num;
	    read(pipe_fds[i][0], &num, sizeof(int));
	    num *= 2;
	    write(parent_pipe[1], &num, sizeof(int));
	    exit(EXIT_SUCCESS);
	}
	else
	{
	    int pnum;
	    pnum = i + 1;
	    printf("Just forked %d.\n", child_pid);
	    printf("Sent %d.\n", pnum);
	    write(pipe_fds[i][1], &pnum, sizeof(int));
	}
    }

    for (i = 0; i < num_proc; i++)
    {
	int snum;
	read(parent_pipe[0], &snum, sizeof(int));
	printf("Received: %d\n", snum);
    }


    // wait for to children to finish
    while (num_proc > 0)
    {
	wait(NULL);
	num_proc--;
    }

    int sum;
    sum = add_numbers(in_file);
    printf("Parent: Sum of numbers in %s: %d\n", argv[1], sum);

    fclose(in_file);
    exit(EXIT_SUCCESS);
}

int add_numbers(FILE* in_file)
{
    int sum = 0;

    // read file line by line
    char line[100];
    while (fgets(line, sizeof(line), in_file) != NULL)
    {
	sum += atoi(line);
    }

    return sum;
}
