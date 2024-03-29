/*
  Josh Tan
  CSCI 474
  Project 1: Fork and Pipe
  10/25/13

  Resources:
  http://www.cs.usfca.edu/~wolber/SoftwareDev/C/CStructs.htm
  http://rabbit.eng.miami.edu/info/functions/time.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>     // provides strcmp( )
#include <sys/types.h>  // provides pid_t
#include <sys/time.h>   // provides gettimeofday( )
#include <libgen.h>     // provides basename( )
#include <ctype.h>      // provides isdigit( )

/*
 A portion of the file to be processed. Contains on offset and
 number of elements to process, starting at the offset.
 */
typedef struct
{
    char *filepath;
    int offset; // in bytes
    int num_lines;
} file_portion;

// function prototypes
int add_nums(file_portion portion);
double time_summation(char *filepath, int num_proc, int should_print);
int is_num(char *str);

// constant variables
const int BYTES_PER_LINE = 5; // 3 digits + '\r' + '\n'
const char *FILE1 = "file1.dat";
const char *FILE2 = "file2.dat";
const char *FILE3 = "file3.dat";
const char *FILE4 = "file4.dat";

int main(int argc, char *argv[])
{
    // exit if not provided with valid command-line parameters
    if (argc > 3 || argc == 1) 
    {
        printf("Usage: %s <filepath> [<num executions>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // get name of input file
    char *filepath = argv[1];

    // get number of parallel processes to utilize
    int num_proc;
    printf("Enter the number of parallel processes to utilize (1, 2, or 4):\n");
    scanf("%d", &num_proc);

    // get number of times to execute summation logic
    int num_tests;
    if (argc == 3)
    {
	if (is_num(argv[2]) == 0)
	{
	    // execution times parameter is non-numeric
	    printf("Invalid number of executions specified.\n");
	    exit(EXIT_FAILURE);
	}
	else
	{
	    num_tests = atoi(argv[2]);
	}
    }
    else
    {
	num_tests = 1; // default to single execution
    }

    printf("---------------------------------------------------------\n");
    printf("Filename: %s\n", filepath);

    int i;
    double total_time = 0;
    int should_print = 0; // flag used to print summation info only once per program invocation
    for (i = 0; i < num_tests; i++)
    {
        if (i == (num_tests - 1))
        {
	    // only print summation info on last execution
            should_print = 1;
        }
	// add the times for each execution
        total_time += time_summation(filepath, num_proc, should_print);
    }

    // logic to adjust output based on number of executions
    if (num_tests == 1)
    {
        printf("Execution time: %f seconds\n", total_time);
    }
    else
    {
        total_time = total_time / (double) num_tests;
        printf("Execution time: %f seconds (averaged over %d tests)\n", total_time, num_tests);
    }

    printf("---------------------------------------------------------\n");
    exit(EXIT_SUCCESS);
}

/*
  Sum the numbers contained in the given file, using the specified number
  of processes. If should_print is 1, then print the summation info
  to standard output. Return the time taken to perform the summation.
 */
double time_summation(char *filepath, int num_proc, int should_print)
{
    // start timer
    struct timeval t1;
    struct timeval t2;
    gettimeofday(&t1, NULL);

    char *filename = basename(filepath);

    // determine number of lines in file (hardcoded)
    int total_nums;
    if (strcmp(filename, FILE1) == 0)
    {
        total_nums = 1000;
    }
    else if (strcmp(filename, FILE2) == 0)
    {
        total_nums = 10000;
    }
    else if (strcmp(filename, FILE3) == 0)
    {
        total_nums = 100000;
    }
    else if (strcmp(filename, FILE4) == 0)
    {
        total_nums = 1000000;
    }
    else
    {
        printf("Error: unknown filename.\nValid filenames are 'file{1,2,3,4}.dat'.\n");
        exit(EXIT_FAILURE);
    }

    // initialize child pipes
    int pipes_to_child[num_proc][2];
    int pipe_to_parent[num_proc][2];

    int i;
    for (i = 0; i < num_proc; i++)
    {
        pipe(pipes_to_child[i]);
        pipe(pipe_to_parent[i]);
    }

    /*
      Fork the specified number of processes to calculate partial sums for the
      file. The i'th process calculates the sum for the i'th section of the file,
      where file sections are divided equally. Each processes send the partial
      sum back to the parent.
     */
    int proc_id;
    for (proc_id = 0; proc_id < num_proc; proc_id++)
    {
        // create a child process to compute the partial sum
        pid_t fork_pid;
        fork_pid = fork();
        if (fork_pid == 0) // is child process
        {
            // retrieve part of file to work on from parent
            file_portion child_portion;
            read(pipes_to_child[proc_id][0], &child_portion, sizeof(file_portion));

            // calculate partial sum
            int child_sum = add_nums(child_portion);

            // send partial sum to parent
            write(pipe_to_parent[proc_id][1], &child_sum, sizeof(int));

            exit(EXIT_SUCCESS);
        }
        else // is parent process
        {
	    // encapsulate file portion info in a struct
            file_portion portion;
            portion.filepath = filepath;
            portion.num_lines = total_nums  / num_proc;
            portion.offset = proc_id * portion.num_lines * BYTES_PER_LINE;

	    // send info to appropriate child
            write(pipes_to_child[proc_id][1], &portion, sizeof(file_portion));
        }
    }
    
    // add up all partial sums
    int total_sum = 0;
    for (i = 0; i < num_proc; i++)
    {
        // read partial sum from child
        int child_sum;
        read(pipe_to_parent[i][0], &child_sum, sizeof(int));
        total_sum += child_sum;
    }

    // if flag set, print num lines processed and total sum
    if (should_print)
    {
        printf("Num lines: %d\n", total_nums);
        printf("Total sum: %d\n", total_sum);
    }

    // stop timer and calculate elapsed time
    gettimeofday(&t2, NULL);
    double t1_s = t1.tv_sec + (t1.tv_usec / 1000000.0);
    double t2_s = t2.tv_sec + (t2.tv_usec / 1000000.0);
    double elapsed_time = t2_s - t1_s;

    return elapsed_time;
}

/* 
   Add all the numbers contained in the specified portion
   of the file. Returns the sum.
 */
int add_nums(file_portion portion)
{
    // open file for reading, checking for errors
    FILE* in_file;
    in_file = fopen(portion.filepath, "r");
    if (in_file == NULL)
    {
        printf("Error: Could not open file.\n");
        exit(EXIT_FAILURE);
    }

    // seek forward to the starting point in the file
    fseek(in_file, portion.offset, SEEK_SET);

    // for each line in the file portion, convert the text to an int and add to total
    char buffer[BYTES_PER_LINE + 1]; // bytes/line + room for '\0'
    int sum = 0;
    int i;
    for (i = 0; i < portion.num_lines; i++)
    {
        fgets(buffer, BYTES_PER_LINE + 1, in_file); // fgets() reads in at most one less than size characters
        sum += atoi(buffer);
    }

    // close file
    fclose(in_file);

    return sum;
}

/* Check if string can be parsed to a number. This is
   done by checking if the first character in the string
   is a digit. (atoi will strip off non-numeric chars
   after initial sequence of digits.)
*/
int is_num(char *str)
{
    return isdigit(str[0]);
}
