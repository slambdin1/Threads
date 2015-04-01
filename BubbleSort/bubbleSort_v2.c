#include <stdlib.h>
#include <stdio.h> 
#include <string.h>    
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>

struct ThreadRequest {
   int start;
   int end;
   int thread;
};

/* Global Variables */
pthread_mutex_t * border_lock;
pthread_mutex_t * sort_lock;
pthread_cond_t  * thread_cond;
pthread_mutex_t main_lock;
pthread_t * thread_list;
int * num_list;
int * sort_list;
int done;

/* End of global variables */

/* Function declarations */
int inputSanitizing(int argc, char *argv[]);
FILE * openFile(char * file_name);
void closeFile(FILE * file);
int stringToNumber(char * arg);
int readFileHeader(FILE * file);
void readFileBody(FILE * file, int nums[], int array_size);
void * alteredBubbleSort(void * request);
void initializeMutexes(int num_of_threads);
int findThreadSize(int num_ints, int num_threads);
void annihilateMutexes(int num_threads);

int main(int argc, char * argv[]) {

   /* Collecting file data */
   int num_threads = inputSanitizing(argc, argv);
   
   FILE * file;
   file = openFile(argv[2]);

   int num_of_ints = readFileHeader(file);

   int num_array[num_of_ints];

   num_list = malloc(sizeof(int)*num_of_ints);
   num_list = num_array;

   int sort_array[num_of_ints];
   sort_list = malloc(sizeof(int)*num_of_ints);
   sort_list = sort_array;

   readFileBody(file, num_array, num_of_ints);
   /* End of data collection */

   /*Create threads*/
   initializeMutexes(num_threads);
   pthread_t threads[num_threads];

   thread_list = malloc(sizeof(pthread_t)*num_threads);
   thread_list = threads;

   int thread_num = findThreadSize(num_of_ints, num_threads);

   int t;
   for(t = 0; t < num_threads; t++) {
      /* Create array list for each thread */
      int array_size;
      if (t == num_threads - 1) {
         if (((thread_num-1)*t + thread_num) != num_of_ints) {
            array_size = num_of_ints - (thread_num-1)*t;
         }
         else {
            array_size = thread_num;
         }
      }
      else {
         array_size = thread_num;
      }

      int start;
      int end;

      start = (thread_num - 1) * t;
      end = start + array_size - 1;
      /* End of array creation */

      /* Create argument struct for pthread creation*/
      struct ThreadRequest *request;
      request = malloc(sizeof(struct ThreadRequest));

      request->start = start;
      request->end = end;
      request->thread = t;
      /* End of struct creation */

      int rc;
      rc = pthread_create(&threads[t], NULL, alteredBubbleSort, (void *) request);

      if(rc) {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(EXIT_FAILURE);
      }
   }

   // done = 0;
    int i;
    for(i = 0; i < num_threads; i++) {
   //    pthread_mutex_lock(&main_lock);
   //       while(done == 0) {
   //          printf("while\n");
   //          pthread_cond_wait(&thread_cond[i], &main_lock);
   //       }  
   //       int j;
   //       for(j = 0; j < num_threads; j++) {
   //          printf("hi\n");
   //          if(!(*(sort_list + j))) {
   //             done = 0;
   //          }
   //       }
   //       if(done == 1) {
   //          pthread_exit(0);
             pthread_join(threads[i], NULL);
   //       }
   //    pthread_mutex_unlock(&main_lock);
    }
   


   /* Clean up threads and close file */
   annihilateMutexes(num_threads);
   closeFile(file);

      /* Print sorted nums */
   int j;
   for(j = 0; j < num_of_ints; j++) {
      printf("%d ", num_array[j]);
   }
   printf("\n");
   
   //free(num_list);
   // free(sort_list);
   // free(thread_list);

   pthread_exit(NULL);
   
   exit(1);
}

int inputSanitizing(int argc, char * argv[]) {

   if(argc != 3) {
      printf("ERROR: you need to input 1) a valid number 2) a file to read from \n");
      exit(EXIT_FAILURE);
   }

   //Check that the first input is a valid number
   int threads = stringToNumber(argv[1]);

   //Check number is between 1 and 16
   if(threads > 16 || threads < 1) {
      printf("The number of threads must be between 1 and 16 \n");
      exit(EXIT_FAILURE);
   }

   return threads;
}

int stringToNumber(char * argc) {
   char *ptr;
   long ret;
   ret = strtol(argc, &ptr, 10);

   if(ptr != NULL && strcmp(ptr, "") != 0 
      && strcmp(ptr, "\n") != 0 && strcmp(ptr, " ") != 0
      && !isspace(*ptr)) {
      printf("%s is an invalid number. Please sanitize inputs \n", argc);
      exit(EXIT_FAILURE);
   }

   return ret;
}

FILE * openFile(char * file_name) {
   FILE *file = fopen(file_name, "r");
   if (file) {
      return file;
   }
   else {
      printf("Cannot read file. Check file name and read permissions \n");
      exit(EXIT_FAILURE);
   }
}

void closeFile(FILE * file) {
   fclose(file);
}

int readFileHeader(FILE * file) {
   char line[100];
   if (fgets(line, sizeof (line), file)) {
      return stringToNumber(line);    
   }
   else {
      printf("File has erroneous data");
      exit(EXIT_FAILURE);
   }
}

/*Used for reading unsorted numbers and validated data*/
void readFileBody(FILE * file, int nums[], int array_size) {
   int i = 0;
   char buf[100];
   while(fgets(buf, sizeof(buf), file) != NULL && i < array_size) {
      char * line = buf;
      if(!isspace(* line)) {
         nums[i] = stringToNumber(buf);
         i++;
      }
   }

   if(i < array_size - 1) {
      printf("File does not have the indicated number of values \n");
      closeFile(file);
      exit(EXIT_FAILURE);
   }
}

void * alteredBubbleSort(void * thread_request) {
   struct ThreadRequest *request = (struct ThreadRequest *) thread_request;
    
   *(sort_list + request->thread) = 0;
   while (!(*(sort_list + request->thread))) {
      int start = request->start;
      int stop = request->end;
      
      // Send the large number down.
      pthread_mutex_lock(&sort_lock[request->thread]);
         *(sort_list + request->thread) = 1;
      pthread_mutex_unlock(&sort_lock[request->thread]);

      int x;
      for (x = start; x < stop; x++) {
         if (*(num_list + x) > *(num_list + x + 1)) {
            
            if(x == start) {
               pthread_mutex_lock(&border_lock[request->thread]);
            }
            else if (x == (stop - 1)) {
               pthread_mutex_lock(&border_lock[request->thread + 1]);
            }

            int previous = *(num_list + x);
            *(num_list + x) = *(num_list + x + 1);
            *(num_list + x + 1) = previous;

            *(sort_list + request->thread) = 0;

            if(x == start) {
               if(request->thread != 0) {
                  *(sort_list + request->thread - 1) = 0;
               }
               pthread_mutex_unlock(&border_lock[request->thread]);
            }
            else if (x == (stop - 1)) {
               *(sort_list + request->thread + 1) = 0;
               pthread_mutex_unlock(&border_lock[request->thread + 1]);
            }
         }
      }
 
      if (*(sort_list + request->thread)) {
         break;
      }
 
      // Bring the small number up.
      pthread_mutex_lock(&sort_lock[request->thread]);
         *(sort_list + request->thread) = 1;
      pthread_mutex_unlock(&sort_lock[request->thread]);
      for (x = stop; x > start ; x--) {
         if (*(num_list + (x-1)) > *(num_list + x)) {
            
            if((x - 1) == start) {
               pthread_mutex_lock(&border_lock[request->thread]);
            }
            else if (x == stop) {
               pthread_mutex_lock(&border_lock[request->thread + 1]);
            }
            
            int previous = *(num_list + (x - 1));
            *(num_list + (x - 1)) = *(num_list + x);
            *(num_list + x) = previous;

            *(sort_list + request->thread) = 0;
            
            if((x - 1) == start) {
               if(request->thread != 0) {
                  *(sort_list + request->thread - 1) = 0;
               }
               pthread_mutex_unlock(&border_lock[request->thread]);
            }
            else if ( x == stop) {
               *(sort_list + request->thread + 1) = 0;
               pthread_mutex_unlock(&border_lock[request->thread + 1]);
            }
         }
      }
   }

   // pthread_mutex_lock(&main_lock);
   //    if (*(sort_list + request->thread)) {
   //       printf("here\n");
   //       done = 1;
   //       pthread_cond_signal(&thread_cond[request->thread]);
   //       printf("after\n");
   //       // break;
   //    }
   // pthread_mutex_unlock(&main_lock);

   free(request);
   pthread_exit(0);
}

void initializeMutexes(int num_threads) {
   // Allocate NumThreads mutexes and conditions.
   border_lock = malloc(sizeof(pthread_mutex_t)*num_threads);
   pthread_mutex_t border[num_threads];
   border_lock = border;

   sort_lock = malloc(sizeof(pthread_mutex_t)*num_threads);
   pthread_mutex_t sort[num_threads];
   sort_lock = sort;

   thread_cond = malloc(sizeof(pthread_cond_t)*num_threads);
   pthread_cond_t cond[num_threads];
   thread_cond = cond;

   pthread_mutex_init(&main_lock, NULL);

   //Initialize created mutexes and threads.
   int i;
   for (i = 0; i < num_threads; i++) {
      pthread_mutex_init(&border_lock[i], NULL);
      pthread_mutex_init(&sort_lock[i], NULL);
      pthread_cond_init(&thread_cond[i], NULL);
   }
}

void annihilateMutexes(int num_threads) {
   int i;
   for(i = 0; i < num_threads; i++) {
      //pthread_mutex_unlock(&border_lock[i]); 
      // pthread_mutex_unlock(&sort_lock[i]); 
      //pthread_mutex_destroy(&border_lock[i]);
   }
}

int findThreadSize(int num_ints, int num_threads) {
   int size;
   if(num_ints/num_threads == ceil(num_ints/num_threads)) {
      size = ceil(num_ints/num_threads) + 1;
   }
   else {
      size = ceil(num_ints/num_threads);
   }
   return size;
}






