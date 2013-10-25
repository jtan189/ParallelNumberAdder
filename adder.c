/*
  Josh Tan
  CSCI 474
  Project 1: Fork and Pipe
  10/25/13

  Sources:
  http://stackoverflow.com/questions/5248915/execution-time-of-c-program
  http://www.tutorialspoint.com/c_standard_library/c_function_rewind.htm
  http://www-ee.eng.hawaii.edu/~tep/EE160/Book/chap7/section2.1.2.html
  http://stackoverflow.com/questions/5139213/count-number-of-line-using-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

int count_lines(FILE *in_file);
int add_nums(int nums[], int offset, int num_elements);
float time_process(char *filename, int num_proc, int should_print_total);

const int BUFFER_SIZE = 100;

int main(int argc, char *argv[])
{

    // get number of parallel processes to utilize
    int num_proc;
    printf("Enter the number of parallel processes to utilize: (1, 2, or 4)\n");
    scanf("%d", &num_proc);

    int num_tests;
    // exit if not provided with valid command-line parameter(s)
    if (argc == 2 || argc == 3) 
    {
	num_tests = argc == 3 ? atoi(argv[2]) : 1;
    }
    else
    {
	printf("Usage: %s <filename>\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    int i;
    float total_time= 0;
    int should_print_total = 0;
    for (i = 0; i < num_tests; i++)
    {
	if (i == (num_tests - 1))
	{
	    should_print_total = 1;
	}
	total_time += time_process(argv[1], num_proc, should_print_total);
    }

    printf("Execution time (averaged over %d test(s)): %f seconds\n", num_tests, total_time / (float) num_tests);
    exit(EXIT_SUCCESS);
}

float time_process(char *filename, int num_proc, int should_print_total)
{

    FILE *in_file;
    int i;

    // start timer
    clock_t begin, end;
    double time_spent;
    begin = clock();

    // open file for reading
    in_file = fopen(filename, "r");
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
    int line_count = 0;
    line_count = count_lines(in_file);
    /* printf("Line count: %d\n", line_count); */

    int nums[line_count];

    i = 0;
    int proc_id; // proc_id starts at 0

    int lines_per_proc = line_count / num_proc;

    for (proc_id = 0; proc_id < num_proc; proc_id++)
    {
	char line[BUFFER_SIZE];

	int num_lines_to_get;

	// if the last child, then also process any remainder
	// that results from unequal division
	if (proc_id == (num_proc - 1))
	{
	    num_lines_to_get = lines_per_proc + (line_count % num_proc);
	    /* printf("Leftover to get: %d\n", num_lines_to_get); */
	}
	else
	{
	    num_lines_to_get = lines_per_proc;
	}

	// process num_lines_to_get
	int j;
	for (j = 0; j < num_lines_to_get; j++)
	{
	    int index = proc_id * lines_per_proc + j;
	    fgets(line, sizeof(line), in_file);
	    nums[index] = atoi(line);
	}

	// can fork process to work on these
	pid_t fork_pid;
	fork_pid = fork();
	if (fork_pid == 0)
	{
	    int offset = proc_id * lines_per_proc;
	    int num_elements = num_lines_to_get;
	    int child_sum = add_nums(nums, offset, num_elements);
	    /* printf("proc_id %d sum: %d, offest: %d, num_elements: %d\n", proc_id, child_sum, offset, num_elements); */

	    // write subresult to parent
	    write(pipe_to_parent[1], &child_sum, sizeof(int));

	    exit(EXIT_SUCCESS);
	}
    }

    fclose(in_file);
    
    int total_sum;
    total_sum = 0;
    for (i = 0; i < num_proc; i++)
    {
	int child_sum;
	read(pipe_to_parent[0], &child_sum, sizeof(int));
	/* printf("Sum received: %d\n", snum); */
	total_sum += child_sum;
    }

    // if flag set, print total
    if (should_print_total)
    {
	printf("Total sum: %d\n", total_sum);
    }

    // stop timer
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    /* printf("Total time: %f seconds\n", time_spent); */

    return time_spent;
}

int add_nums(int nums[], int offset, int num_elements)
{
    int sum = 0;
    int i;
    for (i = offset; i < offset + num_elements; i++)
    {
	sum += nums[i];
    }
    
    return sum;
}

/*
  Return a count of the number of lines in file. Before returning,
  this function resets the file position to the beginning of the
  file.
 */
int count_lines(FILE* f)
{
    int count;
    char ch;
    count = 0;

    while((ch = fgetc(f)) != EOF)
    {
	if (ch == '\n')
	{
	    count++;
	}
    }

    rewind(f); // reset position within file
    return count;
}
