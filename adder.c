#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

// TODO: cite sources
/*
  Sources:
  http://stackoverflow.com/questions/5248915/execution-time-of-c-program
  http://www.tutorialspoint.com/c_standard_library/c_function_rewind.htm
 */

int main(int argc, char *argv[])
{

    FILE *in_file;
    int i;

    // start timer
    clock_t begin, end;
    double time_spent;

    begin = clock();


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
    int pipes_to_child[num_proc][2];
    for (i = 0; i < num_proc; i++)
    {
	pipe(pipes_to_child[i]);
    }

    // initialize parent pipe
    int pipe_to_parent[2];
    pipe(pipe_to_parent);

    // count the number of lines in the file
    int count;
    char line[100];

    count = 0;
    i = 0;

    int nums_size;

    rewind(in_file); // reset position within file

    // read file line by line
    while (fgets(line, sizeof(line), in_file) != NULL)
    {
	count++;
    }

    nums_size = count;
    printf("num lines: %d\n", count);
    rewind(in_file);

    int nums[count];

    while (fgets(line, sizeof(line), in_file) != NULL)
    {
	nums[i] = atoi(line);
	i++;
    }

    int num_to_sum;
    num_to_sum =  nums_size / num_proc;
    printf("num lines per child: %d\n", num_to_sum);

    // spawn requested number of processes
    for (i = 0; i < num_proc; i++)
    {
	pid_t child_pid;
	child_pid = fork();
	if (child_pid == 0)
	{
	    // let child do its thing
	    /* int num; */
	    /* read(pipes_to_child[i][0], &num, sizeof(int)); */
	    /* num *= 2; */

	    int child_sum;
	    int j;
	    int child_size;

	    child_sum = 0;
	    read(pipes_to_child[i][0], &child_size, sizeof(int));

	    for (j = 0; j < child_size; j++)
	    {
//		printf("sum before: %d\n", child_sum);
		child_sum += nums[(i * child_size) + j];
//		printf("adding %d\n", nums[(i * child_size) + j]);

	    }
//	    printf("Sending sum: %d", child_sum);
	    write(pipe_to_parent[1], &child_sum, sizeof(int));
	    exit(EXIT_SUCCESS);
	}
	else
	{
	    int pnum;
	    pnum = i;
	    printf("Just forked %d.\n", child_pid);
//	    printf("Sent %d.\n", pnum);
	    write(pipes_to_child[i][1], &num_to_sum, sizeof(int));
	}
    }
    
    int total_sum;
    total_sum = 0;
    for (i = 0; i < num_proc; i++)
    {
	int snum;
	read(pipe_to_parent[0], &snum, sizeof(int));
//	printf("Sum received: %d\n", snum);
	total_sum += snum;
    }

    printf("Total sum: %d\n", total_sum);

    // wait for to children to finish
    /* while (num_proc > 0) */
    /* { */
    /* 	wait(NULL); */
    /* 	num_proc--; */
    /* } */

    /* int sum; */

    /* i = 0; */
    /* sum = 0; */

    // read file line by line
    /* char line[100]; */
    /* while (fgets(line, sizeof(line), in_file) != NULL) */
    /* { */
    /* 	sum += atoi(line); */
    /* } */

    /* for (i = 0; i < nums_size; i++) */
    /* { */
    /* 	sum += nums[i]; */
    /* } */

//    return sum;
//    sum = add_numbers(nums, nums_size);
//    printf("Parent: Sum of numbers in %s: %d\n", argv[1], sum);

    fclose(in_file);
    
    // stop timer
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Total time: %f seconds\n", time_spent);

    exit(EXIT_SUCCESS);
}

int add_numbers(int nums[], int nums_size)
{
    int i;
    int sum;
    
    sum = 0;

    // read file line by line
    /* char line[100]; */
    /* while (fgets(line, sizeof(line), in_file) != NULL) */
    /* { */
    /* 	sum += atoi(line); */
    /* } */

    for (i = 0; i < nums_size; i++)
    {
	sum += nums[i];
    }

    return sum;
}


