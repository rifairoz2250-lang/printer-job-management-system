/**
 * ============================================================================
 * File: queue.c
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Core implementation of the Queue Data Structure using a
 *              fixed-size array with 'front' and 'rear' index pointers.
 *              Strictly enforces FIFO (First-In, First-Out) order.
 * ============================================================================
 */

#include "../include/queue.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/**
 * Helper function: Generates a human-readable current timestamp.
 * Example format: "20:55:12"
 */
static void get_current_time_str(char *buffer, size_t max_len) {
    time_t raw_time;
    struct tm *time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    if (time_info != NULL) {
        strftime(buffer, max_len, "%H:%M:%S", time_info);
    } else {
        snprintf(buffer, max_len, "00:00:00");
    }
}

/**
 * Helper function: Trims leading and trailing whitespace from a string.
 */
static void trim_whitespace(char *str) {
    if (!str) return;

    /* Trim leading space */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    /* Trim trailing space */
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

/**
 * queue_init()
 * Initializes the print queue into a clean, empty state.
 * 
 * In array-based queue theory:
 * - When empty, both 'front' and 'rear' are traditionally set to -1.
 * - 'count' tracks the exact number of elements currently stored.
 * - 'next_job_id' ensures each incoming print job receives a unique ID.
 */
void queue_init(PrintQueue *q) {
    if (!q) return;

    q->front = -1;
    q->rear = -1;
    q->count = 0;
    q->next_job_id = INITIAL_JOB_ID;

    /* Zero out the entire array to prevent garbage memory */
    memset(q->jobs, 0, sizeof(q->jobs));
}

/**
 * queue_is_empty()
 * Checks if the queue has zero pending print jobs.
 * 
 * Returns true if count == 0 (or front == -1), otherwise false.
 */
bool queue_is_empty(const PrintQueue *q) {
    if (!q) return true;
    return (q->count == 0 || q->front == -1);
}

/**
 * queue_is_full()
 * Checks if the queue has reached its maximum defined capacity.
 * 
 * Returns true if count == MAX_QUEUE_SIZE, otherwise false.
 */
bool queue_is_full(const PrintQueue *q) {
    if (!q) return false;
    return (q->count >= MAX_QUEUE_SIZE);
}

/**
 * queue_count()
 * Returns the current number of pending print jobs.
 */
int queue_count(const PrintQueue *q) {
    if (!q) return 0;
    return q->count;
}

/**
 * queue_peek()
 * Allows inspecting the job currently at the front of the queue without removing it.
 * This is the next document scheduled to be printed under FIFO ordering.
 */
bool queue_peek(const PrintQueue *q, PrintJob *next_job) {
    if (!q || queue_is_empty(q)) {
        return false;
    }

    if (next_job) {
        *next_job = q->jobs[q->front];
    }
    return true;
}

/**
 * queue_enqueue()
 * Adds a new print job to the REAR of the queue.
 * 
 * FIFO Demonstration:
 * Every newly submitted document is appended at index 'rear + 1'.
 * Existing jobs waiting in front will always be printed before this new job.
 * 
 * Error Handling:
 * 1. Queue Overflow: If count >= MAX_QUEUE_SIZE, rejection occurs.
 * 2. Input Validation: Document name must not be blank, pages must be >= 1.
 */
bool queue_enqueue(PrintQueue *q, const char *doc_name, int pages, PrintJob *added_job, char *error_msg) {
    if (!q) {
        if (error_msg) snprintf(error_msg, 128, "Error: Queue pointer is NULL.");
        return false;
    }

    /* 1. Check for Queue Overflow condition */
    if (queue_is_full(q)) {
        if (error_msg) {
            snprintf(error_msg, 128, 
                     "Queue Overflow! Printer buffer is full (Maximum %d jobs). Print or cancel jobs first.", 
                     MAX_QUEUE_SIZE);
        }
        return false;
    }

    /* 2. Validate input parameters */
    if (!doc_name || strlen(doc_name) == 0) {
        if (error_msg) snprintf(error_msg, 128, "Validation Error: Document name cannot be empty.");
        return false;
    }

    char clean_name[MAX_DOC_NAME_LEN];
    strncpy(clean_name, doc_name, sizeof(clean_name) - 1);
    clean_name[sizeof(clean_name) - 1] = '\0';
    trim_whitespace(clean_name);

    if (strlen(clean_name) == 0) {
        if (error_msg) snprintf(error_msg, 128, "Validation Error: Document name cannot be whitespace only.");
        return false;
    }

    if (pages <= 0) {
        if (error_msg) snprintf(error_msg, 128, "Validation Error: Number of pages must be at least 1.");
        return false;
    }

    if (pages > 1000) {
        if (error_msg) snprintf(error_msg, 128, "Validation Error: Maximum allowed pages per job is 1000.");
        return false;
    }

    /* 3. Adjust front and rear pointers for insertion */
    if (queue_is_empty(q)) {
        /* If inserting into an empty queue, front moves to index 0 */
        q->front = 0;
        q->rear = 0;
    } else {
        /* Otherwise, rear increments by 1 */
        q->rear++;
    }

    /* 4. Construct the new PrintJob record */
    PrintJob new_job;
    new_job.job_id = q->next_job_id++;
    strncpy(new_job.doc_name, clean_name, sizeof(new_job.doc_name) - 1);
    new_job.doc_name[sizeof(new_job.doc_name) - 1] = '\0';
    new_job.pages = pages;
    get_current_time_str(new_job.submitted_at, sizeof(new_job.submitted_at));

    /* 5. Place the job at the rear position of the array */
    q->jobs[q->rear] = new_job;
    q->count++;

    /* Copy created job back to caller if requested */
    if (added_job) {
        *added_job = new_job;
    }

    return true;
}

/**
 * queue_dequeue()
 * Removes the job at the FRONT of the queue to simulate printing.
 * 
 * FIFO Demonstration:
 * The document that has been waiting the longest (located at index 'front', which is 0)
 * is selected for printing.
 * 
 * Contiguous Array Maintenance:
 * To ensure the queue remains contiguous and array space is never exhausted at the front,
 * remaining jobs shift one slot to the left (from index 1..rear down to 0..rear-1).
 * The 'rear' index is decremented by 1.
 * If the queue becomes empty after dequeue, both front and rear reset to -1.
 */
bool queue_dequeue(PrintQueue *q, PrintJob *printed_job, char *error_msg) {
    if (!q) {
        if (error_msg) snprintf(error_msg, 128, "Error: Queue pointer is NULL.");
        return false;
    }

    /* 1. Check for Queue Underflow condition */
    if (queue_is_empty(q)) {
        if (error_msg) {
            snprintf(error_msg, 128, "Queue Underflow! No pending print jobs. The queue is empty.");
        }
        return false;
    }

    /* 2. Retrieve the front job (FIFO order: front element is processed first) */
    if (printed_job) {
        *printed_job = q->jobs[q->front];
    }

    /* 3. Shift remaining elements forward to preserve contiguous slots */
    for (int i = 0; i < q->rear; i++) {
        q->jobs[i] = q->jobs[i + 1];
    }

    /* Clear the previously occupied rear slot */
    memset(&q->jobs[q->rear], 0, sizeof(PrintJob));

    /* 4. Decrement rear and count */
    q->rear--;
    q->count--;

    /* 5. If queue is now empty, reset pointers to -1 */
    if (q->count == 0) {
        q->front = -1;
        q->rear = -1;
    }

    return true;
}

/**
 * queue_cancel()
 * Allows a user to remove a specific job from the queue by entering its Job ID.
 * 
 * Search & Removal Logic:
 * 1. Linear search through array slots from front to rear.
 * 2. If the Job ID is found at index 'k':
 *    - Save the job details.
 *    - Shift elements from index 'k + 1' to 'rear' left by one position.
 *    - Decrement 'rear' and 'count'.
 *    - If queue becomes empty, reset front and rear to -1.
 * 3. If Job ID is not found, return false with an error message.
 */
bool queue_cancel(PrintQueue *q, int job_id, PrintJob *cancelled_job, char *error_msg) {
    if (!q) {
        if (error_msg) snprintf(error_msg, 128, "Error: Queue pointer is NULL.");
        return false;
    }

    if (queue_is_empty(q)) {
        if (error_msg) {
            snprintf(error_msg, 128, "Cancel Failed: The queue is empty. No jobs to cancel.");
        }
        return false;
    }

    if (job_id <= 0) {
        if (error_msg) {
            snprintf(error_msg, 128, "Validation Error: Job ID must be a positive integer.");
        }
        return false;
    }

    /* Search for the target Job ID */
    int target_idx = -1;
    for (int i = 0; i <= q->rear; i++) {
        if (q->jobs[i].job_id == job_id) {
            target_idx = i;
            break;
        }
    }

    if (target_idx == -1) {
        if (error_msg) {
            snprintf(error_msg, 128, "Cancel Failed: Job ID #%d not found in the queue.", job_id);
        }
        return false;
    }

    /* Capture cancelled job data */
    if (cancelled_job) {
        *cancelled_job = q->jobs[target_idx];
    }

    /* Shift subsequent jobs left to keep contiguous array */
    for (int i = target_idx; i < q->rear; i++) {
        q->jobs[i] = q->jobs[i + 1];
    }

    /* Zero out old rear position */
    memset(&q->jobs[q->rear], 0, sizeof(PrintJob));

    q->rear--;
    q->count--;

    if (q->count == 0) {
        q->front = -1;
        q->rear = -1;
    }

    return true;
}

/**
 * queue_display()
 * Formats and prints all pending jobs currently residing in the queue to standard output.
 * Demonstrates the front-to-rear order clearly.
 */
void queue_display(const PrintQueue *q) {
    if (!q) return;

    printf("\n+-------------------------------------------------------------------------+\n");
    printf("|                         CURRENT PRINT QUEUE STATUS                      |\n");
    printf("+-------------------------------------------------------------------------+\n");
    printf("  Capacity: %d/%d jobs  |  Front Index: %d  |  Rear Index: %d\n",
           q->count, MAX_QUEUE_SIZE, q->front, q->rear);
    printf("+-------+---------+--------------------------------+-------+--------------+\n");
    printf("| Pos   | Job ID  | Document Name                  | Pages | Submitted At |\n");
    printf("+-------+---------+--------------------------------+-------+--------------+\n");

    if (queue_is_empty(q)) {
        printf("|              --- QUEUE IS CURRENTLY EMPTY (0 Pending Jobs) ---           |\n");
        printf("+-------+---------+--------------------------------+-------+--------------+\n");
        return;
    }

    for (int i = 0; i <= q->rear; i++) {
        const PrintJob *j = &q->jobs[i];
        const char *marker = "";
        if (i == 0 && q->rear == 0) {
            marker = "(FRONT/REAR)";
        } else if (i == 0) {
            marker = "(FRONT)";
        } else if (i == q->rear) {
            marker = "(REAR)";
        }

        char pos_str[16];
        snprintf(pos_str, sizeof(pos_str), "#%d %s", i + 1, marker);

        printf("| %-5s | #%-6d | %-30.30s | %-5d | %-12s |\n",
               pos_str, j->job_id, j->doc_name, j->pages, j->submitted_at);
    }
    printf("+-------+---------+--------------------------------+-------+--------------+\n");
    printf("  FIFO Principle: Job at Pos #1 (FRONT) will be printed next.\n\n");
}
