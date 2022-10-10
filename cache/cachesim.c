#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>



/*
how tf do i do this

1. find a way to read the given parameters in the command line 
(command line parsing) --> DONE
2. figure out how to read the memory trace files (file parsing) --> DONE
3. model the cache representation (e.g., what data type to use, etc) 
to do the calculations (gna have to use data structs here i think)
    a. make cache struct
    b. process line information (ttttssbb)
4. finally, build the steps to do the real calculations

*/


void help() {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

typedef struct instruct{
    char oper;
    unsigned long long addy;
} instruct;

int instruct_length = 0;

typedef struct cache{
    int valid;
    unsigned long long tag;
    int timestamp;
} cache;



// function that finds [i] of LRU
int LRU(struct cache ** c, int set, int lines) {
    int min = INT_MAX;
    int ret = 0;
    for (int i = 0; i < lines; i++) {
        if (min > c[set][i].timestamp) {
            min = c[set][i].timestamp;
            ret = i;
        }
    }
    return ret;
}


// variables needed when simulating cache
int time = 0;
unsigned int hits;
unsigned int misses;
unsigned int evicts;


// function that simulates the cache
// void simulate(full_cache, input_index, input_tag, E, time);

void simulate(struct cache ** c, int set_param, int tag_param, int line, int time) {
// loop through cache line and check for index
    for (int i = 0; i < line; i++) {
        // if valid bit is 1 and tag matches, it is a hit
        if (c[set_param][i].valid == 1){
            if (c[set_param][i].tag == tag_param){
                c[set_param][i].timestamp = time;
                hits++;
                // USE FOR DEBUGGING
                // printf("%llx, hit ", c[set_param][i].tag);
                break;
            }
            // if valid bit is 1 and tag doesn't match, conflict miss
            // (i == (E-1)) ensures that all lines have been looked through
            else if (i == (line-1)) {
                misses++;
                evicts++;
                int replace = LRU(c, set_param, line);
                c[set_param][replace].tag = tag_param;
                c[set_param][replace].timestamp = time;
                break;
                // USE FOR DEBUGGING
                // printf("%llx, miss eviction ", c[set_param][i].tag);
            }
        // if valid bit is 0, miss and add in values
        } else {
            c[set_param][i].valid = 1;
            c[set_param][i].tag = tag_param;
            c[set_param][i].timestamp = time;
            misses++;
            // USE FOR DEBUGGING
            // printf("%llx, miss ", c[set_param][i].tag);
            break;
        }
    }
}


int main(int argc, char *argv[])
{
    // create file variable
    FILE *f;

    // creating values to store command line argument values in
    // int h = 0;
    // int v = 0;
    int s = 0;
    int E = 0;
    int b = 0;
    int S = 0;
    // int B = 0;
    // char *t = NULL;
    int parse = 0;

    // parse through command line with getopt looping
    while ((parse = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (parse) {
            case 'h':
                help();
                return 0;
            /*
            case 'v':
                v = 1;
                break;
                */
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                f = fopen(optarg, "r");
                break;
            default:
                fprintf(stderr, "wrong argument given!");
                exit(1);
                break;
        }
    }

    // int B = pow(2, b);
    S = pow(2, s);

    char oper_buf;
    unsigned long long int addy_buf;
    int size_buf;
    int count = 0;

    // loop through to find length of array of cachelines
    while (fscanf(f," %c %llx,%d", &oper_buf, &addy_buf, &size_buf) > 0) {
        if (oper_buf == 'I') {
            continue;
        }
        else {
            count++;
        }
    }

    // declare the length of cacheline, then reset counter variable to 0
    // make sure to reset file f so you can fscanf again
    instruct_length = count;
    count = 0;
    rewind(f);


    // malloc array of cachelines, then loop through indiv cacheline to malloc
    struct instruct *instructions;
    instructions = (instruct *) malloc (instruct_length * (sizeof(instruct)));

    // parse the file to input info into the cacheline arrays
    while (fscanf(f," %c %llx,%d", &oper_buf, &addy_buf, &size_buf) > 0) {
        if (oper_buf == 'I') {
            continue;
        }
        instructions[count].oper = oper_buf;
        instructions[count].addy = addy_buf;
        count++;
    }
    // USE WHEN DEBUGGING
    /*
    for (int i = 0; i < instruct_length; i++) {
        printf("%c, %llx \n", instructions[i].oper, instructions[i].addy);
    }
    */
    
    // do i wanna make this a function...
    // now, malloc, then initialize the cache
    // this has to be a 2d array, because it has multiple lines/sets
    struct cache **full_cache;
    // **** DO I MULTIPLY BY E or by S?
    full_cache = (struct cache **) malloc (sizeof(cache*) * S);
    for (int i = 0; i < S; i++) {
        full_cache[i] = (struct cache *) malloc (sizeof(cache) * E);
    }

    // initializing all values in the cache to be 0
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            full_cache[i][j].valid = 0;
            full_cache[i][j].tag = 0;
            full_cache[i][j].timestamp = 0;
            // USE WHEN DEBUGGING
            /*
            printf("cache line %d, %d:\n", i, j);
            printf("valid bit = %d.\n", full_cache[i][j].valid);
            printf("tag = %llx.\n", full_cache[i][j].tag);
            printf("timestamp = %d.\n", full_cache[i][j].timestamp);
            */
        }
    }


    unsigned long long int input_tag = 0;
    unsigned long long int input_index = 0;


    // loop through instruction array and get tag + index of mem access
    for (int step = 0; step < instruct_length; step++) {
        input_tag = (instructions[step].addy >> (b+s));
        input_index = ((instructions[step].addy >> b) & (S-1));
        // USE WHEN DEBUGGING
        /*
        printf("input tag: %llx\n", input_tag);
        printf("input index: %llx\n", input_index);
        */

        if (instructions[step].oper == 'M') {
            //printf("M ");
            time++;
            simulate(full_cache, input_index, input_tag, E, time);
            time++;
            simulate(full_cache, input_index, input_tag, E, time);
            //printf("\n");
        } else {
            //printf("L/S ");
            time++;
            simulate(full_cache, input_index, input_tag, E, time);
            //printf("\n");

        }
    }

    // USE WHEN DEBUGGING
    /*
    printf("hits: %d\n", hits);
    printf("misses: %d\n", misses);
    printf("evicts: %d\n", evicts);
    */



/*
    // USE WHEN DEBUGGING:
    printf("h is %d\n", h);
    printf("v is %d\n", v);
    printf("s is %d\n", s);
    printf("E is %d\n", E);
    printf("b is %d\n", b);
    printf("t is %s\n", t);
    printf("S is %d\n", S);
    printf("B is %d\n", B);
*/

    // USE WHEN DEBUGGING
    //printf("cacheline length is %d\n", instruct_length);

    fclose(f);

    // Freeing instructions
    free(instructions);
 
    // Freeing full_cache struct
    for (int i = 0; i < S; i++) {
        free(full_cache[i]);
    }
    free(full_cache);

    printSummary(hits, misses, evicts);
    return 0;