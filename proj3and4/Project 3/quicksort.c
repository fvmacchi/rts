#include <LPC17xx.h>
#include <RTL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "quicksort.h"
#include "array_tools.h"

// You decide what the threshold will be
#define USE_INSERTION_SORT 50

OS_SEM mutex;
int counter;

typedef struct {
	array_t array;
	size_t a;
	size_t c;
} array_interval_t;

typedef struct{
	array_interval_t interval;
	unsigned char priority;
} qsort_task_parameters_t;

/*
void insertion_sort( array_interval_t interval) {
	size_t j, k;
	double tmp;
	array_type *array = interval.array.array;
	
	for ( k = interval.a + 1; k < interval.c; ++k ) {
		tmp = array[k];
		for ( j = k; j > interval.a; --j ) {
			if ( array[j - 1] > tmp ) {
				array[j] = array[j - 1];
			} else {
				array[j] = tmp;
				goto finished;
			}
		}
		array[interval.a] = tmp; // only executed if tmp < array[a]
		finished:
		; // null statement
	}
}*/


void insertion_sort( array_interval_t interval ) {
	
	size_t i,j;
  array_type temp;
  
  for( i = interval.a + 1 ; i <= interval.c; i++){
    j = i;
    while (j > interval.a && interval.array.array[j]<interval.array.array[j-1]){
      temp = interval.array.array[j];
			interval.array.array[j] = interval.array.array[j-1];
			interval.array.array[j-1] = temp;
      j--;
    }
  }
}



__task void quick_sort_task( void* void_ptr){
  // Your implementation here
	array_type pivot, temp;
	int i;
	bool below = 0;
	size_t up , down, pivotIndex;
	qsort_task_parameters_t *task_params = ((qsort_task_parameters_t *) void_ptr);
	qsort_task_parameters_t task_params1;
	qsort_task_parameters_t task_params2;
	array_interval_t interval = task_params->interval;
	unsigned char priority = task_params->priority;
	OS_TID task1, task2;
	
	//printf("\n%d",priority);
	
		for(i = interval.a; i < interval.c;i++)
			{
				//printf("%d ",interval.array.array[i]);
			}
			//printf("\n");
			
	if(interval.c - interval.a < USE_INSERTION_SORT) {
		//printf("insertion sort\n");
		insertion_sort(interval);
		//printf("doneIS");
	}
	else
	{
		pivot = interval.array.array[interval.c-1];

		up = interval.a;
		down = interval.c-2;

		while (true){
			//printf("loop");
			while(up < interval.c-1 && interval.array.array[up] <= pivot){
				//printf("up");
				up++;
			}
			while(down > interval.a && interval.array.array[down] > pivot){
				//printf("down");
				down--;
			}
				
			if(up >= down){
				break;
			}
			
			//Swap values where array stopped
			//printf("swap [%d]=%d with [%d]=%d\n",up,interval.array.array[up],down,interval.array.array[down]);
			temp = interval.array.array[up];
			interval.array.array[up] = interval.array.array[down];
			interval.array.array[down] = temp;
			//printf("\n", priority);
			
			for(i = interval.a; i < interval.c;i++)
			{
				//printf("%d ",interval.array.array[i]);
			}
			//printf("\n");
		}
		//printf("doneQS");
		pivotIndex = up;
		//printf("putting pivot at [%d]=%d\n",pivotIndex,interval.array.array[pivotIndex]);
		interval.array.array[interval.c-1] = interval.array.array[pivotIndex];
		interval.array.array[pivotIndex] = pivot;
		
		
		for(i = interval.a; i < interval.c;i++)
			{
				//printf("%d ",interval.array.array[i]);
			}
			//printf("\n");
		
		//task_params->interval.a = interval.a;
		//task_params->interval.c = pivotIndex;
		//task_params->priority = priority+1;
		
		task_params1.interval.array = interval.array;
		task_params1.interval.a = interval.a;
		task_params1.interval.c = pivotIndex;
		task_params1.priority = priority+1;
		if(task_params1.interval.c - task_params1.interval.a > 1)
		{
			task1 = os_tsk_create_ex( quick_sort_task, priority+1, &task_params1 );	
			if(!task1) {
				//printf("task with priority %d not created",task_params1.priority);
				insertion_sort(task_params1.interval);
			}
		}
		
		task_params2.interval.array = interval.array;
		task_params2.interval.a = pivotIndex+1;
		task_params2.interval.c = interval.c;
		task_params2.priority = priority+1;
		if(task_params2.interval.c - task_params2.interval.a > 1)
		{
			task2 = os_tsk_create_ex( quick_sort_task, priority+1, &task_params2 );	
			if(!task2) {
				//printf("task with priority %d not created",task_params2.priority);
				insertion_sort(task_params1.interval);
			}
		}
		

		//printf("TasksCreated\n");
		//interval.a = pivotIndex+1;
	}
		
	os_tsk_delete_self();
}

__task void quick_sort_task_sem( void* void_ptr){
	// Your implementation here
	array_type pivot, temp;
	size_t up , down, pivotIndex;
	qsort_task_parameters_t *task_params = ((qsort_task_parameters_t *) void_ptr);
	qsort_task_parameters_t *task_params1;
	qsort_task_parameters_t *task_params2;
	array_interval_t interval = task_params->interval;
	unsigned char priority = task_params->priority;
	int i;
	OS_TID task;
	
	free(task_params);
	
	
	if(interval.c - interval.a >= USE_INSERTION_SORT) {
		pivot = interval.array.array[interval.c-1];

		up = interval.a;
		down = interval.c-2;

		while (true){
			//printf("loop");
			while(up < interval.c-1 && interval.array.array[up] <= pivot){
				//printf("up");
				up++;
			}
			while(down > interval.a && interval.array.array[down] > pivot){
				//printf("down");
				down--;
			}
				
			if(up >= down){
				break;
			}
			
			//Swap values where array stopped
			//printf("swap [%d]=%d with [%d]=%d\n",up,interval.array.array[up],down,interval.array.array[down]);
			temp = interval.array.array[up];
			interval.array.array[up] = interval.array.array[down];
			interval.array.array[down] = temp;
			//printf("\n", priority);
			
			for(i = interval.a; i < interval.c;i++)
			{
				//printf("%d ",interval.array.array[i]);
			}
			//printf("\n");
		}
		//printf("doneQS");
		pivotIndex = up;
		//printf("putting pivot at [%d]=%d\n",pivotIndex,interval.array.array[pivotIndex]);
		interval.array.array[interval.c-1] = interval.array.array[pivotIndex];
		interval.array.array[pivotIndex] = pivot;
		
		
		
		
		do {
			task_params1 = (qsort_task_parameters_t *)malloc(sizeof(qsort_task_parameters_t));
			printf("tried allocating \n");
		}while(!task_params1);
		task_params1->interval.array = interval.array;
		task_params1->interval.a = interval.a;
		task_params1->interval.c = pivotIndex;
		task_params1->priority = priority;
		
		os_sem_wait(&mutex, 0xffff);
		counter++;
		printf("\n%d",counter);
		os_sem_send(&mutex);
		
		if(task_params1->interval.c - task_params1->interval.a > 1)
		{
			task = os_tsk_create_ex( quick_sort_task_sem, task_params->priority, task_params1 );
			if(!task) {
				//printf("task with priority %d not created",task_params2.priority);
				insertion_sort(task_params1->interval);
				free(task_params1);
				os_sem_wait(&mutex, 0xffff);
				counter--;
				printf("\n%d",counter);
				os_sem_send(&mutex);
			}
		}
		
		do {
			task_params2 = (qsort_task_parameters_t *)malloc(sizeof(qsort_task_parameters_t));
			printf("tried allocating \n");
		}while(!task_params2);
		task_params2->interval.array = interval.array;
		task_params2->interval.a = pivotIndex + 1;
		task_params2->interval.c = interval.c;
		task_params2->priority = priority;
		
		os_sem_wait(&mutex, 0xffff);
		counter++;
		printf("\n%d",counter);
		os_sem_send(&mutex);
		
		if(task_params2->interval.c - task_params2->interval.a > 1)
		{
			task = os_tsk_create_ex( quick_sort_task_sem, task_params->priority, task_params2 );
			if(!task) {
				//printf("task with priority %d not created",task_params2.priority);
				insertion_sort(task_params2->interval);
				free(task_params2);
				os_sem_wait(&mutex, 0xffff);
				counter--;
				printf("\n%d",counter);
				os_sem_send(&mutex);
			}
		}
	}
	else {
		insertion_sort(interval);
	}
	os_sem_wait(&mutex, 0xffff);
	counter--;
		printf("\n%d",counter);
	os_sem_send(&mutex);
	os_tsk_delete_self();
}

void quicksort( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_t *task_param;
	int i;
	
	// Based on MTE 241 course notes--you can change this if you want
	//  - in the course notes, this sorts from a to c - 1
	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length;
	for(i = interval.a; i < interval.c;i++)
			{
				//printf("%d ",interval.array.array[i]);
			}
			//printf("\n");

	task_param = (qsort_task_parameters_t *)malloc(sizeof(qsort_task_parameters_t));
	task_param->interval = interval;

	// If you are using priorities, you can change this
	task_param->priority = 10;

	//start the quick_sort threading
	os_tsk_create_ex( quick_sort_task, task_param->priority, task_param );
	printf("done");
}

void quicksort_sem( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_t *task_param;

	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length;

	do {
		task_param = (qsort_task_parameters_t *)malloc(sizeof(qsort_task_parameters_t));
	}while(!task_param);
	task_param->interval = interval;

	// If you are using priorities, you can change this
	task_param->priority = 10;

	os_sem_init(&mutex, 1);
	counter = 1;

	//start the quick_sort threading
	os_tsk_create_ex( quick_sort_task_sem, task_param->priority, task_param );

	while(1)
	{
		os_sem_wait(&mutex, 0xffff);
		if(counter <= 0) {
			break;
		}
		os_sem_send(&mutex);
	}
}
