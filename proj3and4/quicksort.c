#include <LPC17xx.h>
#include <RTL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "quicksort.h"
#include "array_tools.h"

// You decide what the threshold will be
#define USE_INSERTION_SORT 20

typedef struct {
	array_t array;
	size_t a;
	size_t c;
} array_interval_t;

typedef struct{
	array_interval_t interval;
	unsigned char priority;
} qsort_task_parameters_t;

typedef struct{
	array_interval_t interval;
	unsigned char priority;
	OS_SEM *mutex;
} qsort_task_parameters_sem_t;

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
}


__task void quick_sort_task( void* void_ptr){
  // Your implementation here
	array_type pivot, temp;
	size_t up , down, pivotIndex;
	qsort_task_parameters_t *task_params = ((qsort_task_parameters_t *) void_ptr);
	array_interval_t interval = task_params->interval;
	unsigned char priority = task_params->priority;
	
	while(true) {
		if(interval.c - interval.a < USE_INSERTION_SORT) {
			insertion_sort(interval);
			break;
		}
		pivot = interval.array.array[interval.c-1];

		up = interval.a;
		down = interval.c-2;

		while (true){
			while(interval.array.array[up] <= pivot && up <= down){
				up++;
			}
			while(interval.array.array[down] > pivot && up <= down && down > interval.a){
				down--;
			}

			if(up >= down){
				break;
			}
			//Swap values where array stopped
			temp = interval.array.array[up];
			interval.array.array[up] = interval.array.array[down];
			interval.array.array[down] = temp;
		}
		if(up < interval.c-1){
			interval.array.array[interval.c-1] = interval.array.array[up];
			interval.array.array[up] = pivot;
			pivotIndex = up;
		}
		else {
			pivotIndex = down;
		}

		task_params->interval.c = pivotIndex;

		os_tsk_create_ex( quick_sort_task, priority+1, task_params );

		interval.a = pivotIndex+1;
	}
		
	os_tsk_delete_self();
}

__task void quick_sort_task_sem( void* void_ptr){
	// Your implementation here
	array_type pivot, temp;
	size_t up , down, pivotIndex;
	qsort_task_parameters_sem_t *task_params = ((qsort_task_parameters_sem_t *) void_ptr);
	qsort_task_parameters_sem_t task_params2;
	OS_SEM *mutex = task_params->mutex;
	OS_SEM mutex1, mutex2;
	array_interval_t interval = task_params->interval;
	
	
	if(interval.c - interval.a >= USE_INSERTION_SORT) {
		pivot = interval.array.array[interval.c-1];

		up = interval.a;
		down = interval.c-2;

		while (true){
			while(interval.array.array[up] <= pivot && up <= down){
				up++;
			}
			while(interval.array.array[down] > pivot && up <= down && down > interval.a){
				down--;
			}

			if(up >= down){
				break;
			}
			//Swap values where array stopped
			temp = interval.array.array[up];
			interval.array.array[up] = interval.array.array[down];
			interval.array.array[down] = temp;
		}
		if(up < interval.c-1){
			interval.array.array[interval.c-1] = interval.array.array[up];
			interval.array.array[up] = pivot;
			pivotIndex = up;
		}
		else {
			pivotIndex = down;
		}

		task_params->interval.c = pivotIndex;
		task_params2.interval.a = pivotIndex+1;
		task_params2.interval.c = interval.c;
		
		os_sem_init(&mutex1,1);
		os_sem_init(&mutex2,1);
		task_params->mutex = &mutex1;
		task_params2.mutex = &mutex2;
		
		os_tsk_create_ex( quick_sort_task_sem, task_params->priority, task_params );
		os_tsk_create_ex( quick_sort_task_sem, task_params->priority, &task_params2 );
		
		os_sem_wait(mutex1, 0xffff);
		os_sem_wait(mutex2, 0xffff);
	}
	else {
		insertion_sort(interval);
	}
	os_sem_send(mutex);

	os_tsk_delete_self();
}

void quicksort( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_t task_param;

	// Based on MTE 241 course notes--you can change this if you want
	//  - in the course notes, this sorts from a to c - 1
	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length;

	task_param.interval = interval;

	// If you are using priorities, you can change this
	task_param.priority = 10;

	//start the quick_sort threading
	os_tsk_create_ex( quick_sort_task, task_param.priority, &task_param );
	printf("done");
}

void quicksort_sem( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_sem_t task_param;
	OS_SEM mutex;

	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length;

	task_param.interval = interval;

	// If you are using priorities, you can change this
	task_param.priority = 10;

	os_sem_init(mutex, 1);

	task_param.mutex = &mutex;

	//start the quick_sort threading
	os_tsk_create_ex( quick_sort_task_sem, task_param.priority, &task_param );

	os_sem_wait(&mutex, 0xffff);
}
