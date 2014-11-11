#include <LPC17xx.h>
#include <RTL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "quicksort.h"
#include "array_tools.h"

// You decide what the threshold will be
#define USE_INSERTION_SORT 5

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
	unsigned int *count;
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

	while(true) {
		if(interval.a - interval.c < 20) {
			insertion_sort(interval);
			break;
		}
		pivot = interval.array.array[interval.c-1];

		up = interval.a;
		down = interval.c;

		while (true){
			while(interval.array.array[up] < pivot){
				up++;
			}
			while(interval.array.array[down] > pivot){
				down--;
			}

			if(up > down){
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
		}
		pivotIndex = up;

		task_params->interval.c = pivotIndex;

		os_tsk_create_ex( quick_sort_task, task_params->priority+1, task_params );

		interval.a = pivotIndex+1;
	}
}

__task void quick_sort_task_sem( void* void_ptr){
	// Your implementation here
	array_type pivot, temp;
	size_t up , down, pivotIndex;
	qsort_task_parameters_sem_t *task_params = ((qsort_task_parameters_sem_t *) void_ptr);
	OS_SEM *mutex = task_params->mutex;
	unsigned int *count = task_params->count;
	array_interval_t	interval = task_params->interval;

	while(true) {
		if(interval.a - interval.c < 20) {
			insertion_sort(interval);
			break;
		}
		pivot = interval.array.array[interval.c-1];

		up = interval.a;
		down = interval.c;

		while (true){
			while(interval.array.array[up] < pivot){
				up++;
			}
			while(interval.array.array[down] > pivot){
				down--;
			}

			if(up > down){
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
		}
		pivotIndex = up;

		task_params->interval.c = pivotIndex;

		os_sem_wait(mutex, 0xffff);
		(*count)++;
		os_sem_send(mutex);
		os_tsk_create_ex( quick_sort_task, task_params->priority, task_params );

		interval.a = pivotIndex+1;
	}
	os_sem_wait(mutex, 0xffff);
	(*count)--;
	os_sem_send(mutex);
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
}

void quicksort_sem( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_sem_t task_param;
	OS_SEM counterSem;
	unsigned int counter = 1;

	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length;

	task_param.interval = interval;

	// If you are using priorities, you can change this
	task_param.priority = 10;

	os_sem_init(counterSem, 0);

	task_param.mutex = &counterSem;
	task_param.count = &counter;

	//start the quick_sort threading
	os_tsk_create_ex( quick_sort_task_sem, task_param.priority, &task_param );

	while(true) {
		os_sem_wait(&counterSem, 0xffff);
		if(counter == 0) {
			os_sem_send(&counterSem);
			break;
		}
		os_sem_send(&counterSem);
	}
}
