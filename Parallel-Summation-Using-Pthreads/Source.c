#include <stdio.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

#define array_size 100
#define no_threads 2
int split_size = array_size / no_threads; // I assume that array_size is divisible by no_threads
int a[array_size];

struct properties { // The goal of this struct is to pack multiple arguments that will be passed to the slave function below
	int tid; // Local thread ID. Each thread will use it to know which part of the array to sum
	int* arr; // Pointer to the array we want to sum
};

int sum = 0;
pthread_mutex_t mutex1;

void *slave(void *threadarg) // Each thread runs this functions
{
	int local_sum = 0;
	struct properties *my_prop;
	my_prop = (struct properties *) threadarg; // Extract thread properties from function argument
	int tid = my_prop->tid; // Local thread ID
	int *arr = my_prop->arr; // Pointer to the array we want to sum
	for (int i = tid*split_size; i < (tid+1)*split_size; i++) // Thread only reads a partition of the array
	{
		local_sum += arr[i]; // Aggregate array elements in local sum
	};
	// Add local sum to global sum:
	pthread_mutex_lock(&mutex1); // Lock the mutex
	sum += local_sum; // Add
	pthread_mutex_unlock(&mutex1); // Release mutex so other threads can use the global variable
}

int main()
{
	int i;
	pthread_t thread[no_threads]; // Array of pthread objects
	struct properties prop[no_threads]; // Array of properties structs. Each one will be passed to a thread
	pthread_mutex_init(&mutex1, NULL); // One mutex that will be accessed by all threads

	for (i = 0; i < array_size; i++) // Initialize  array elements
		a[i] = i + 1;

	for (i = 0; i < no_threads; i++) { // Create threads
		prop[i].tid = i; // This is the local thread ID
		prop[i].arr = a; // A pointer to the array to sum

		if (pthread_create(&thread[i], NULL, &slave, (void*)&prop[i]) != 0) // Create thread
			perror("Pthread create has failed.");
	}
		
	for (i = 0; i < no_threads; i++) // Wait until all threads are complete before the main continues
		if (pthread_join(thread[i], NULL) != 0)
			perror("Pthread join fails");

	printf("The sum of 1 to %i is %d\n", array_size, sum); // Print global sum
	getchar(); // wait for the user to click any button before the console window closes

	return 0;
}