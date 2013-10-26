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
  http://www.cs.usfca.edu/~wolber/SoftwareDev/C/CStructs.htm
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <libgen.h>

/*
 A portion of the file to be processed. Contains on offset and
 number of elements to process, starting at the offset.
 */
typedef struct
{
    char *filepath;
    int offset;
    int size; // in bytes
} file_portion;

// function prototypes
//int count_lines(FILE *in_file);
int add_nums(file_portion portion);
float time_process(char *filepath, int num_proc, int should_print_total);

const int BUFFER_SIZE = 100;
const int BYTES_PER_LINE = 6; // might be 4 <###\r\n> + 1 (since will append \0)
const char *FILE1 = "file1.dat";
const char *FILE2 = "file2.dat";
const char *FILE3 = "file3.dat";
const char *FILE4 = "file4.dat";

int main(int argc, char *argv[])
{
    // get name of input file
    char *filepath = argv[1];

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
	printf("Usage: %s <filepath>\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    printf("---------------------------------------------------------\n");
    printf("Filename: %s\n", filepath);

    int i;
    float total_time= 0;
    int should_print_total = 0;
    for (i = 0; i < num_tests; i++)
    {
	if (i == (num_tests - 1))
	{
	    should_print_total = 1;
	}
	total_time += time_process(filepath, num_proc, should_print_total);
    }

    if (num_tests == 1)
    {
	printf("Execution time: %f seconds\n", total_time);
    }
    else
    {
	total_time = total_time / (float) num_tests;
	printf("Execution time: %f seconds (averaged over %d tests)\n", total_time / (float) num_tests, num_tests);
    }
    printf("---------------------------------------------------------\n");
    exit(EXIT_SUCCESS);
}

float time_process(char *filepath, int num_proc, int should_print_total)
{

    FILE *in_file;
    int i;
    int total_nums;

    // start timer
    clock_t begin, end;
    double time_spent;
    begin = clock();

    char *filename = basename(filepath);

    // initialize number of lines in file
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
	printf("Error: unknown filename.\nValid filenames are 'file{1,2,3,4}.dat'.");
	exit(EXIT_FAILURE);
    }

    // initialize child pipes
    int pipes_to_child[num_proc][2];
    int pipe_to_parent[num_proc][2];
    for (i = 0; i < num_proc; i++)
    {
	pipe(pipes_to_child[i]);
	pipe(pipe_to_parent[i]);
    }

    // initialize parent pipe
//    int pipe_to_parent[2];
//    pipe(pipe_to_parent);

    int proc_id;
    for (proc_id = 0; proc_id < num_proc; proc_id++)
    {
    	// can fork process to work on these
    	pid_t fork_pid;
    	fork_pid = fork();
    	if (fork_pid == 0)
    	{
    	    // get part of file to work on, from parent
    	    file_portion child_portion;
    	    read(pipes_to_child[proc_id][0], &child_portion, sizeof(file_portion));

    	    // calculate sum of portion
    	    int child_sum = add_nums(child_portion);
    	    /* printf("proc_id %d sum: %d, offest: %d, num_elements: %d\n", proc_id, child_sum, offset, num_elements); */

    	    // send partial sum to parent
    	    write(pipe_to_parent[proc_id][1], &child_sum, sizeof(int));

    	    exit(EXIT_SUCCESS);
    	}
    	else
    	{
    	    file_portion portion;
	    portion.filepath = filepath;
	    portion.size = total_nums  / num_proc;
//	    printf("__fize__: %d", portion.size);
    	    portion.offset = proc_id * portion.size * 4; // STOP HARD CODING FKDLSJFIO#WJRIPO
    	    write(pipes_to_child[proc_id][1], &portion, sizeof(file_portion));
    	}
    }


    /* // count the number of lines in the file */
    /* int line_count = 0; */
    /* line_count = count_lines(in_file); */
    /* /\* printf("Line count: %d\n", line_count); *\/ */

    /* int nums[line_count]; */

    /* i = 0; */
    /* int proc_id; // proc_id starts at 0 */

    /* int lines_per_proc = line_count / num_proc; */

    /* for (proc_id = 0; proc_id < num_proc; proc_id++) */
    /* { */
    /* 	char line[BUFFER_SIZE]; */

    /* 	int total_nums_to_get; */

    /* 	// if the last child, then also process any remainder */
    /* 	// that results from unequal division */
    /* 	if (proc_id == (num_proc - 1)) */
    /* 	{ */
    /* 	    total_nums_to_get = lines_per_proc + (line_count % num_proc); */
    /* 	    /\* printf("Leftover to get: %d\n", total_nums_to_get); *\/ */
    /* 	} */
    /* 	else */
    /* 	{ */
    /* 	    total_nums_to_get = lines_per_proc; */
    /* 	} */

    /* 	// process total_nums_to_get */
    /* 	int j; */
    /* 	for (j = 0; j < total_nums_to_get; j++) */
    /* 	{ */
    /* 	    int index = proc_id * lines_per_proc + j; */
    /* 	    fgets(line, sizeof(line), in_file); */
    /* 	    nums[index] = atoi(line); */
    /* 	} */

    /* 	// can fork process to work on these */
    /* 	pid_t fork_pid; */
    /* 	fork_pid = fork(); */
    /* 	if (fork_pid == 0) */
    /* 	{ */
    /* 	    // get part of file to work on, from parent */
    /* 	    file_portion child_portion; */
    /* 	    read(pipes_to_child[proc_id][0], &child_portion, sizeof(file_portion)); */

    /* 	    // calculate sum of portion */
    /* 	    int child_sum = add_nums(nums, child_portion); */
    /* 	    /\* printf("proc_id %d sum: %d, offest: %d, num_elements: %d\n", proc_id, child_sum, offset, num_elements); *\/ */

    /* 	    // send partial sum to parent */
    /* 	    write(pipe_to_parent[1], &child_sum, sizeof(int)); */

    /* 	    exit(EXIT_SUCCESS); */
    /* 	} */
    /* 	else */
    /* 	{ */
    /* 	    file_portion portion; */
    /* 	    portion.offset = proc_id * lines_per_proc; */
    /* 	    portion.num_elements = total_nums_to_get; */
    /* 	    write(pipes_to_child[proc_id][1], &portion, sizeof(file_portion)); */

    /* 	} */
    /* } */


    
    int total_sum;
    total_sum = 0;
    for (i = 0; i < num_proc; i++)
    {
    	// read partial sum from child
    	int child_sum;
    	read(pipe_to_parent[i][0], &child_sum, sizeof(int));
    	/* printf("Sum received: %d\n", snum); */
    	total_sum += child_sum;
    }

    // if flag set, print total
    if (should_print_total)
    {
	printf("Num lines: %d\n", total_nums);
	printf("Total sum: %d\n", total_sum);
    }

    // stop timer
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
//    printf("Total time: %f seconds\n", time_spent);

    return time_spent;
}

int add_nums(file_portion portion)
{

    // open file for reading
    FILE* in_file;
    in_file = fopen(portion.filepath, "r");
    if (in_file == NULL)
    {
	printf("Error: Could not open file.\n");
	exit(EXIT_FAILURE);
    }

    int sum = 0;
    char num_string[BYTES_PER_LINE + 1]; // + 1???
//    char num_string[BUFFER_SIZE]; // + 1???

    int i;
    fseek(in_file, portion.offset, SEEK_SET);
    for (i = 0; i < portion.size; i++)
    {
	fgets(num_string, BYTES_PER_LINE, in_file);
//	printf("(%d)=num: %d ", portion.offset, atoi(num_string));
    	sum += atoi(num_string);
//	printf("proc w/ %d offset\n", portion.offset);
    }

    fclose(in_file);
//    printf("portion size: %d\n", portion.size);
    return sum;
}

/* /\* */
/*   Return a count of the number of lines in file. Before returning, */
/*   this function resets the file position to the beginning of the */
/*   file. */
/*  *\/ */
/* int count_lines(FILE* f) */
/* { */
/*     int count; */

/*     count = 0; */
/* //    int char_per_line = 0; */

/*     while((ch = fgetc(f)) != EOF) */
/*     { */
/* //	char_per_line++; */
/* 	if (ch == '\n') */
/* 	{ */
/* 	    count++; */
/* //	    printf("char / line: %d", char_per_line); */
/* //	    char_per_line = 0; */
/* 	} */
/*     } */

/*     rewind(f); // reset position within file */
/*     return count; */
/* } */
