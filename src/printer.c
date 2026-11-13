/**
 * ============================================================================
 * File: printer.c
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Implementation of printer manager operations: delegates queue
 *              manipulations to queue.c while tracking high-level print state
 *              and formatting output messages.
 * ============================================================================
 */

#include "../include/printer.h"
#include <stdio.h>
#include <string.h>

/**
 * printer_init()
 * Sets up printer data structures and resets all counters.
 */
void printer_init(PrinterSystem *ps) {
    if (!ps) return;
    queue_init(&ps->queue);
    memset(&ps->last_printed_job, 0, sizeof(PrintJob));
    ps->has_last_printed = false;
    ps->total_printed_count = 0;
    ps->total_cancelled_count = 0;
}

/**
 * printer_add_job()
 * Submits a document to be queued for printing.
 */
bool printer_add_job(PrinterSystem *ps, const char *doc_name, int pages, PrintJob *added_job, char *error_msg) {
    if (!ps) {
        if (error_msg) snprintf(error_msg, 128, "Error: Printer system pointer is NULL.");
        return false;
    }
    return queue_enqueue(&ps->queue, doc_name, pages, added_job, error_msg);
}

/**
 * printer_print_next()
 * Simulates printing the frontmost pending job under FIFO rules.
 */
bool printer_print_next(PrinterSystem *ps, PrintJob *printed_job, char *error_msg) {
    if (!ps) {
        if (error_msg) snprintf(error_msg, 128, "Error: Printer system pointer is NULL.");
        return false;
    }

    PrintJob job;
    bool success = queue_dequeue(&ps->queue, &job, error_msg);
    if (success) {
        ps->last_printed_job = job;
        ps->has_last_printed = true;
        ps->total_printed_count++;
        if (printed_job) {
            *printed_job = job;
        }
    }
    return success;
}

/**
 * printer_cancel_job()
 * Removes a job from the queue before it prints.
 */
bool printer_cancel_job(PrinterSystem *ps, int job_id, PrintJob *cancelled_job, char *error_msg) {
    if (!ps) {
        if (error_msg) snprintf(error_msg, 128, "Error: Printer system pointer is NULL.");
        return false;
    }

    bool success = queue_cancel(&ps->queue, job_id, cancelled_job, error_msg);
    if (success) {
        ps->total_cancelled_count++;
    }
    return success;
}

/**
 * printer_get_pending_count()
 * Returns the count of jobs currently waiting in the queue.
 */
int printer_get_pending_count(const PrinterSystem *ps) {
    if (!ps) return 0;
    return queue_count(&ps->queue);
}

/**
 * printer_display_pending()
 * Formats and prints the pending jobs table.
 */
void printer_display_pending(const PrinterSystem *ps) {
    if (!ps) return;
    queue_display(&ps->queue);
}

/**
 * printer_display_status()
 * Displays overall printer dashboard summary in CLI mode.
 */
void printer_display_status(const PrinterSystem *ps) {
    if (!ps) return;

    printf("\n=======================================================\n");
    printf("              PRINTER DASHBOARD METRICS                \n");
    printf("=======================================================\n");
    printf("  Pending Jobs in Queue : %d (Capacity: %d)\n", 
           ps->queue.count, MAX_QUEUE_SIZE);
    printf("  Total Jobs Printed    : %d\n", ps->total_printed_count);
    printf("  Total Jobs Cancelled  : %d\n", ps->total_cancelled_count);
    
    if (ps->has_last_printed) {
        printf("  Last Printed Document : #%d - \"%s\" (%d pages)\n",
               ps->last_printed_job.job_id,
               ps->last_printed_job.doc_name,
               ps->last_printed_job.pages);
    } else {
        printf("  Last Printed Document : None yet\n");
    }

    PrintJob next_job;
    if (queue_peek(&ps->queue, &next_job)) {
        printf("  Next in Line (FRONT)  : #%d - \"%s\" (%d pages)\n",
               next_job.job_id, next_job.doc_name, next_job.pages);
    } else {
        printf("  Next in Line (FRONT)  : None (Queue is empty)\n");
    }
    printf("=======================================================\n\n");
}

/**
 * Helper: Escapes string for JSON output safely.
 */
static void json_escape_str(const char *src, char *dest, size_t dest_len) {
    if (!dest || dest_len == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }

    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dest_len - 2; i++) {
        if (src[i] == '"' || src[i] == '\\') {
            if (j < dest_len - 3) {
                dest[j++] = '\\';
                dest[j++] = src[i];
            }
        } else if (src[i] == '\n') {
            if (j < dest_len - 3) {
                dest[j++] = '\\';
                dest[j++] = 'n';
            }
        } else if (src[i] == '\r') {
            if (j < dest_len - 3) {
                dest[j++] = '\\';
                dest[j++] = 'r';
            }
        } else if (src[i] == '\t') {
            if (j < dest_len - 3) {
                dest[j++] = '\\';
                dest[j++] = 't';
            }
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}

/**
 * printer_to_json()
 * Serializes the complete printer queue state into a JSON string.
 */
void printer_to_json(const PrinterSystem *ps, char *out_buf, size_t out_max) {
    if (!ps || !out_buf || out_max == 0) return;

    const PrintQueue *q = &ps->queue;
    char jobs_array[4096] = "[";
    size_t cur_len = strlen(jobs_array);

    for (int i = 0; i <= q->rear && i < MAX_QUEUE_SIZE; i++) {
        const PrintJob *j = &q->jobs[i];
        char escaped_doc[128];
        json_escape_str(j->doc_name, escaped_doc, sizeof(escaped_doc));

        char item[512];
        snprintf(item, sizeof(item),
            "%s{\"job_id\": %d, \"doc_name\": \"%s\", \"pages\": %d, \"submitted_at\": \"%s\", \"pos\": %d, \"is_front\": %s, \"is_rear\": %s}",
            (i > 0 ? ", " : ""),
            j->job_id,
            escaped_doc,
            j->pages,
            j->submitted_at,
            i + 1,
            (i == 0 ? "true" : "false"),
            (i == q->rear ? "true" : "false"));

        if (cur_len + strlen(item) < sizeof(jobs_array) - 2) {
            strcat(jobs_array, item);
            cur_len += strlen(item);
        }
    }
    strcat(jobs_array, "]");

    char last_printed_json[512] = "null";
    if (ps->has_last_printed) {
        char escaped_last[128];
        json_escape_str(ps->last_printed_job.doc_name, escaped_last, sizeof(escaped_last));
        snprintf(last_printed_json, sizeof(last_printed_json),
            "{\"job_id\": %d, \"doc_name\": \"%s\", \"pages\": %d, \"submitted_at\": \"%s\"}",
            ps->last_printed_job.job_id,
            escaped_last,
            ps->last_printed_job.pages,
            ps->last_printed_job.submitted_at);
    }

    snprintf(out_buf, out_max,
        "{"
        "\"count\": %d, "
        "\"capacity\": %d, "
        "\"front\": %d, "
        "\"rear\": %d, "
        "\"is_empty\": %s, "
        "\"is_full\": %s, "
        "\"total_printed\": %d, "
        "\"total_cancelled\": %d, "
        "\"last_printed\": %s, "
        "\"jobs\": %s"
        "}",
        q->count,
        MAX_QUEUE_SIZE,
        q->front,
        q->rear,
        queue_is_empty(q) ? "true" : "false",
        queue_is_full(q) ? "true" : "false",
        ps->total_printed_count,
        ps->total_cancelled_count,
        last_printed_json,
        jobs_array);
}
