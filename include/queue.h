/**
 * ============================================================================
 * File: queue.h
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Header file defining the PrintJob structure, the PrintQueue
 *              structure (implemented using a fixed-size array), and all
 *              queue operation prototypes (FIFO order).
 * ============================================================================
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * CONSTANTS & CONFIGURATION
 * ============================================================================
 */
#define MAX_QUEUE_SIZE 10        /* Maximum print jobs the queue can hold */
#define MAX_DOC_NAME_LEN 64      /* Maximum length of document name */
#define MAX_TIME_STR_LEN 32      /* Formatted timestamp string length */
#define INITIAL_JOB_ID 1001      /* Starting unique Job ID */

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================
 */

/**
 * struct PrintJob
 * Represents an individual document submitted to the printer queue.
 */
typedef struct {
    int job_id;                          /* Unique positive identifier (e.g., 1001, 1002) */
    char doc_name[MAX_DOC_NAME_LEN];     /* Name of document (e.g., "DataStructures_Notes.pdf") */
    int pages;                           /* Number of pages to print */
    char submitted_at[MAX_TIME_STR_LEN]; /* Timestamp when the job was enqueued */
} PrintJob;

/**
 * struct PrintQueue
 * Represents the print queue data structure implemented using a contiguous array.
 * Maintains FIFO (First-In, First-Out) discipline using 'front' and 'rear' markers.
 */
typedef struct {
    PrintJob jobs[MAX_QUEUE_SIZE];  /* Contiguous array storing pending print jobs */
    int front;                      /* Index of the job at the front (next to print) */
    int rear;                       /* Index of the job at the rear (most recently added) */
    int count;                      /* Total number of active pending jobs in queue */
    int next_job_id;                /* Internal auto-incrementing counter for unique IDs */
} PrintQueue;

/* ============================================================================
 * QUEUE FUNCTION PROTOTYPES
 * ============================================================================
 */

/**
 * Initialize a PrintQueue instance to its default empty state.
 * Sets front = -1, rear = -1, count = 0, and next_job_id = INITIAL_JOB_ID.
 */
void queue_init(PrintQueue *q);

/**
 * Check if the queue contains no elements.
 * Returns true if empty (count == 0 or front == -1), false otherwise.
 */
bool queue_is_empty(const PrintQueue *q);

/**
 * Check if the queue has reached its maximum capacity.
 * Returns true if full (count == MAX_QUEUE_SIZE), false otherwise.
 */
bool queue_is_full(const PrintQueue *q);

/**
 * Insert a new print job at the rear of the queue (ENQUEUE operation).
 * Demonstrates FIFO: New jobs always join at the rear.
 *
 * Parameters:
 *   q          - Pointer to the PrintQueue
 *   doc_name   - Non-empty document name string
 *   pages      - Page count (must be >= 1)
 *   added_job  - Optional out-pointer to copy the newly created job
 *   error_msg  - Buffer (size >= 128) to store descriptive error on failure
 *
 * Returns:
 *   true on success, false on failure (e.g. queue overflow or invalid input).
 */
bool queue_enqueue(PrintQueue *q, const char *doc_name, int pages, PrintJob *added_job, char *error_msg);

/**
 * Remove and return the job at the front of the queue (DEQUEUE operation).
 * Demonstrates FIFO: The job waiting longest at 'front' is printed first.
 * Shifts subsequent jobs forward to maintain contiguous array indexing.
 *
 * Parameters:
 *   q           - Pointer to the PrintQueue
 *   printed_job - Out-pointer to store the dequeued job details
 *   error_msg   - Buffer (size >= 128) to store descriptive error on failure
 *
 * Returns:
 *   true on success, false on failure (e.g. queue underflow).
 */
bool queue_dequeue(PrintQueue *q, PrintJob *printed_job, char *error_msg);

/**
 * Cancel and remove a specific job from anywhere in the queue by its Job ID.
 * Preserves the FIFO order of all remaining jobs by shifting subsequent elements.
 *
 * Parameters:
 *   q             - Pointer to the PrintQueue
 *   job_id        - Unique ID of the job to cancel
 *   cancelled_job - Out-pointer to store the removed job details
 *   error_msg     - Buffer (size >= 128) to store descriptive error on failure
 *
 * Returns:
 *   true on success, false if the Job ID is not found or queue is empty.
 */
bool queue_cancel(PrintQueue *q, int job_id, PrintJob *cancelled_job, char *error_msg);

/**
 * Return the total number of pending jobs in the queue.
 */
int queue_count(const PrintQueue *q);

/**
 * Inspect the job currently at the front of the queue without removing it (PEEK).
 *
 * Parameters:
 *   q        - Pointer to the PrintQueue
 *   next_job - Out-pointer to store the front job details
 *
 * Returns:
 *   true if a front job exists, false if queue is empty.
 */
bool queue_peek(const PrintQueue *q, PrintJob *next_job);

/**
 * Print a formatted text representation of the queue to standard output (CLI).
 */
void queue_display(const PrintQueue *q);

#endif /* QUEUE_H */
