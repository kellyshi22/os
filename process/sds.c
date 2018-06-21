/********************************************************
 * FILE: sds.c
 * DATE: 4/5/2018
 * UNIT: OS, 2018 S1
 * AUTHOR: Kai Li Shi 19157364
 * PURPOSE: Implements the Readers Writers problem for 
 *          Simple Data Sharing using processes
 *******************************************************/

#include "sds.h"

int main (int argc, char * argv[]) {
    
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
		printf("Error. Command line arguments must be positive\n");
		return -1;
	}

    int parentID = getpid();

    // create shared memory
    int databuffer_fd = shm_open("databuffer", O_CREAT | O_RDWR, 0666);
    int readcount_fd = shm_open("readcount", O_CREAT | O_RDWR, 0666);
    int numDWritten_fd = shm_open("numDWritten", O_CREAT |O_RDWR, 0666);
    int locks_fd = shm_open("locks", O_CREAT | O_RDWR, 0666);
    int countarray_fd = shm_open("countarray", O_CREAT | O_RDWR, 0666);
    int fastestreader_fd = shm_open("fastestreader", O_CREAT | O_RDWR, 0666);
	int sharedarray_fd = shm_open("sharedarray", O_CREAT | O_RDWR, 0666); 

    // error checking
    if (databuffer_fd == -1 || readcount_fd == -1 || locks_fd == -1 || 
			numDWritten_fd == -1 || fastestreader_fd == -1 || sharedarray_fd == -1)
    {
        printf("error creating shared data\n");
        return -1;
    }

    // configure sizes
    error += ftruncate(databuffer_fd, sizeof(int) * b);
    error += ftruncate(readcount_fd, sizeof(int));
    error += ftruncate(numDWritten_fd, sizeof(int));
    error += ftruncate(locks_fd, sizeof(Locks));
    error += ftruncate(countarray_fd, sizeof(int)*b);
    error += ftruncate(fastestreader_fd, sizeof(int));
	error += ftruncate(sharedarray_fd, sizeof(int)*d);

    if (error != 0)
    {
        printf("error ftruncating\n");
        return -1;
    }

    // pointer to the shared data
    int * data_buffer = (int*) mmap(0, sizeof(int) * b, PROT_WRITE | PROT_READ, MAP_SHARED, databuffer_fd, 0);
    int * readcount = (int*)mmap(0, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, readcount_fd, 0);
    int * numDWritten = (int*)mmap(0, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, numDWritten_fd, 0);
    Locks * locks = (Locks*)mmap(0, sizeof(Locks), PROT_WRITE | PROT_READ, MAP_SHARED, locks_fd, 0);
    int * countarray = (int*)mmap(0, sizeof(int)*b, PROT_WRITE | PROT_READ, MAP_SHARED, countarray_fd, 0);
    int * fastestreader = (int*)mmap(0, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, fastestreader_fd, 0);
	int * sharedarray = (int*)mmap(0, sizeof(int)*d, PROT_WRITE | PROT_READ, MAP_SHARED, sharedarray_fd, 0);

    // initialise semaphores 
    error += sem_init(&locks->read, -1, 1);
    error += sem_init(&locks->count, -1, 1);
    error += sem_init(&locks->empty, -1, b);
    error += sem_init(&locks->full, -1, 0);
 
    if (error != 0)
    {
        printf("error initialising semaphores\n");
        return -1;
    }

    // initialise the countarray
    for (int i = 0; i < b; i++)
    {
        countarray[i] = r;
    }
  
	// initialise shared memory
    *readcount = 0;
    *numDWritten = 0;
    *fastestreader = -1;

    // open the shared_data file and add all elements to the sharedarray for writers to access
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
    fclose(fopen("sim_out", "w"));
	

    pid_t pid;

    // create r children processes
    for (int i = 0; i < r; i++)
    {
        if (parentID == getpid())
        {
            pid = fork();
            if (pid == 0)
            {
                reader(locks, data_buffer, readcount, countarray, numDWritten, fastestreader);
                exit(0);
            }
        }
    }

    // create w children processes
    for (int i = 0; i < w; i++)
    {
        if (parentID == getpid())
        {
            pid = fork();
            if (pid == 0)
            {
                writer(locks, data_buffer, sharedarray, numDWritten);
                exit(0);
            }
        }
    }

    // parent waits for children processes
	for (int i = 0; i < r+w; i++)
	{
		wait(NULL);
	}

	// clean up memory
	error += close(databuffer_fd);
	error += close(readcount_fd);
	error += close(numDWritten_fd);
	error += close(locks_fd);
	error += close(countarray_fd);
	error += close(fastestreader_fd);
	error += close(sharedarray_fd);
    error += sem_destroy(&locks->read);
	error += sem_destroy(&locks->count);
	error += sem_destroy(&locks->empty);
	error += sem_destroy(&locks->full);
	error += munmap(data_buffer, sizeof(int));
	error += munmap(readcount, sizeof(int));
	error += munmap(numDWritten, sizeof(int));
	error += munmap(locks, sizeof(Locks));
	error += munmap(countarray, sizeof(int) * b);
	error += munmap(fastestreader, sizeof(int));
	error += munmap(sharedarray, sizeof(int) *d);

	if (error != 0)
	{
		printf("error closing shared memory\n");
		return -1;
	}
   
    return 0;
}

/* METHOD writer
 * IMPORTS counter (int), locks (Locks*), data_buffer (int*), f (int*), numDWritten (int*)
 * EXPORTS none
 * PURPOSE allows writers to writer to the data_buffer one at a time*/
void writer(Locks * locks, int * data_buffer, int * f, int * numDWritten)
{
	int counter = 0;
	int exit = FALSE;
    while (!exit) //(*numDWritten < d)
    {
		// writer decrements empty
		sem_wait(&locks->empty);
		// writer decrements read
        sem_wait(&locks->read);

        // exit if 100 numbers already written
	    if (*numDWritten == d)
		{
			exit = TRUE;
		}
		else // write 1 number to the buffer
		{
			data_buffer[*numDWritten % b] = f[*numDWritten];
            (*numDWritten)++;
            counter++;
		}

		// writer increments read
        sem_post(&locks->read);
		//writer increments full
        sem_post(&locks->full); 
        sleep(t2);
    }
    sem_post(&locks->full);
	
	// writes out message to sim_out
    char message[300];
	sprintf(message, "writer-%d has finished writing %d pieces of data to the data_buffer\n", getpid(), counter);
	writeMessage(message);
}

/* METHOD: reader
 * IMPORTS: locks (Locks *), data_buffer (int*), readcount (int*), 
 *          countarray (int*), numDWritten (int*), fastestreader (int*)
 * EXPORTS: none
 * PURPOSE:  reads elements from the data_buffer while there are elements to read. 
 *           the last reader of each index will call sem_post().
 *           the first reader of each index will call sem_wait().*/
void reader(Locks * locks, int * data_buffer, int * readcount, int* countarray, int * numDWritten, int * fastestreader)
{
	int counter = 0;
    while (counter < d)
    {
		// the fastest reader will call sem_wait(&locks->full)
        if ((countarray[counter%b] == r) && *fastestreader < counter)
		{
			*fastestreader = counter;
			sem_wait(&locks->full);
		}

		// lock the count to update readcount
        sem_wait(&locks->count);
        (*readcount)++;
        if (*readcount == 1)
        {
			// lock the read semaphore if it is the first reader
            sem_wait(&locks->read);
        }

        sem_post(&locks->count);

		// keep reading while there are values in the data_buffer
        while (counter < *numDWritten)
        {
            // read the value at data_buffer[counter%b]
            countarray[counter%b]--;
            if (countarray[counter%b] == 0)
            {
				// if it is the last reader for the index
				// then reset the counter and let the 
				// writer know that it can overwrite the value
				countarray[counter%b] = r;
                sem_post(&locks->empty); 
            }
            counter++;
        }

		// update readcount 
        sem_wait(&locks->count);
        (*readcount)--;
        if (*readcount == 0)
        {
			// unlock read semaphore if it is the last reader
            sem_post(&locks->read);
        }
        sem_post(&locks->count);
        sleep(t1);
    }

	// writes message to sim_out
	char message[300];
	sprintf(message, "reader-%d has finished reading %d pieces of data from the data_buffer\n", getpid(), counter);
	writeMessage(message);
}


/* METHOD: writeMessage
 * IMPORTS: message (string)
 * EXPORTS: none 
 * PURPOSE: writes message string to the "sim_out" file */
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
