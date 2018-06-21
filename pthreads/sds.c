/******************************************************
 * FILE: sds.c
 * DATE: 4/5/2018
 * UNIT: OS, 2018 S1
 * AUTHOR: Kai Li Shi 19157364
 * PURPOSE: Implements the Readers Writers problem for 
 *          Simple Data Sharing using pthreads 
 *****************************************************/

#include "sds.h"

int main (int argc, char * argv[])
{
    // check that all arguments are entered
    if (argc != 5) {
        printf("Usage: ./sds r w, t1 t2\n");
        return -1;
    }

	// rename argument names
    r = atoi(argv[1]);
    w = atoi(argv[2]);
    t1 = atoi(argv[3]);
    t2 = atoi(argv[4]);

	// validate arguments
	if (r < 1 || w < 1 || t1 < 0 || t2 < 0)
	{
		printf("Error. Invalid command line arguments\n");
		return -1;
	}

    // open shared_data file and add all elements to the sharedarray
	// for writers to access
    FILE * fp = fopen("shared_data", "r");
	if (fp == NULL)
	{
		printf("Error opening shared_data file\n");
		return -1;
	}
	for (int i = 0; i < d; i++)
	{
		int number;
		fscanf(fp, "%d,", &number);
		sharedarray[i] = number;
	}
	fclose(fp);

	fclose(fopen("sim_out","w"));

	// initialise countarray
    for (int i = 0; i < b; i++)
    {
        countarray[i] = r;
    }

    // launch threads. array of reader and writer threads
    pthread_t tidReader[r];
    pthread_t tidWriter[w];

    // create reader threads
    for (int i = 0; i < r; i++) 
	{
		pthread_create(&tidReader[i], NULL, reader, NULL);
    }

    // create writer threads
    for (int i = 0; i < w; i++)
    {
        pthread_create(&tidWriter[i], NULL, writer, NULL);
    }

    // wait for reader threads
    for (int i = 0; i < r; i++)
    {
        pthread_join(tidReader[i], NULL);
    }

    // wait for writer threads
    for (int i = 0; i < w; i++)
    {
        pthread_join(tidWriter[i], NULL);
    }

	// free all memory
	pthread_mutex_destroy(&readmutex);
	pthread_mutex_destroy(&count);
	pthread_cond_destroy(&empty);
	pthread_cond_destroy(&full);

    return 0;
}

/* METHOD writer
 * IMPORTS ptr (void*)
 * EXPORTS void*
 * PURPOSE allows writers to write to the data_buffer one at a time */
void * writer ()
{
    int counter = 0;
	int exit = FALSE;
    while (!exit) 
	{
		// lock the read mutex
        pthread_mutex_lock(&readmutex);

		// no space in the buffer to write
		// writer waits for the readers to finish reading
		while (emptycount == 0)
		{
			pthread_cond_wait(&empty, &readmutex);
		}

		// writer writes 1 number to the data_buffer
		if (numDWritten < d )
		{
			data_buffer[numDWritten%b] = sharedarray[numDWritten];
			numDWritten++;
			counter++;
			fullcount++;
			emptycount--;
		}
		else
		{
			exit = TRUE;
		}

		// signals to readers that they can read another number
		pthread_cond_signal(&full); 
		//unlocks the read mutex
		pthread_mutex_unlock(&readmutex);
        sleep(t2);
    }

	// writes out message to sim_out
	char message[300];
	sprintf(message, "writer-%d has finished writing %d pieces of data to the data_buffer\n", (int)pthread_self(), counter);
    writeMessage(message);
    pthread_exit(0);
}

/* METHOD reader
 * IMPORTS none
 * EXPORTS void* 
 * PURPOSE reads elements from the data_buffer while there are elements to read */
void * reader() 
{
	int counter=0;

    while (counter < d) {
		// lock the count mutex, to update readcount
        pthread_mutex_lock(&count);
        readcount++;
        if (readcount == 1) // if it is the first reader then lock resource from writer
        {
			pthread_mutex_lock(&readmutex);
        }
 
		// reader has nothing to read. waits for the writer to write
        while (fullcount == 0)
		{
			pthread_cond_wait(&full, &readmutex);
			fullcount--;
		}

	    pthread_mutex_unlock(&count);

		// continues reading while there are unread elements 
        while (counter < numDWritten)
        {
			// read the element at data_buffer[counter%b]'
            countarray[counter%b]--; // decrease countarray
			// the last reader will reset the countarray for the index
			// signals to the writer that they can write
            if (countarray[counter%b] == 0)
            {
                countarray[counter%b] = r;
				emptycount++;
                pthread_cond_signal(&empty);
            }
            counter++;
        }

		// lock count mutex to update readcount
        pthread_mutex_lock(&count);
        readcount--;
        if (readcount == 0) // if it is the last reader then unlock resource from writer
        {
            pthread_mutex_unlock(&readmutex);
        }
        pthread_mutex_unlock(&count);
        sleep(t1);
    }

    // writes message to sim_out
	char message[300];
	sprintf(message, "reader-%d has finished reading %d pieces of data from the data_buffer\n", (int)pthread_self(), counter);
    writeMessage(message);
    pthread_exit(0);
}


/* METHOD writeMessage 
 * IMPORTS message (string)
 * EXPORTS none
 * PURPOSE writes message string to the "sim_out" file */
void writeMessage(char * message)
{
    FILE* fp = fopen("sim_out", "a");
    if (fp == NULL) {
        printf("error opening sim_out: %s\n", message);
        fclose(fp);
    }
    else
    {
        int error = fprintf(fp, message);
        if (error == EOF)
        {
            printf("error opening sim_out: %s\n", message);
        }
        fclose(fp);
    }
}
