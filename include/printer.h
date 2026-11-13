/**
 * ============================================================================
 * File: printer.h
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Higher-level printer management engine that coordinates print
 *              jobs, tracks printer hardware simulation status, maintains
 *              statistics, and formats outputs.
 * ============================================================================
 */

#ifndef PRINTER_H
#define PRINTER_H

#include "queue.h"

/**
 * struct PrinterSystem
 * Encapsulates the entire printer subsystem: the pending queue,
 * printing statistics, and the most recently completed print job.
 */
typedef struct {
    PrintQueue queue;                   /* The underlying FIFO print queue */
    PrintJob last_printed_job;          /* Snapshot of the last printed document */
    bool has_last_printed;              /* True if at least one job was printed */
    int total_printed_count;            /* Lifetime count of printed documents */
    int total_cancelled_count;          /* Lifetime count of cancelled documents */
} PrinterSystem;

/* ============================================================================
 * PRINTER MANAGER FUNCTION PROTOTYPES
 * ============================================================================
 */

/**
 * Initialize the printer system and its underlying queue.
 */
void printer_init(PrinterSystem *ps);

/**
 * Add a new print job through the printer system.
 * Validates document name and pages before enqueueing.
 */
bool printer_add_job(PrinterSystem *ps, const char *doc_name, int pages, PrintJob *added_job, char *error_msg);

/**
 * Print the next job in FIFO order.
 * Dequeues from the front of the queue, marks it as last printed,
 * and updates stats.
 */
bool printer_print_next(PrinterSystem *ps, PrintJob *printed_job, char *error_msg);

/**
 * Cancel a specific pending job by its Job ID.
 */
bool printer_cancel_job(PrinterSystem *ps, int job_id, PrintJob *cancelled_job, char *error_msg);

/**
 * Return current count of pending jobs waiting to be printed.
 */
int printer_get_pending_count(const PrinterSystem *ps);

/**
 * Display pending jobs to console in a tabular format.
 */
void printer_display_pending(const PrinterSystem *ps);

/**
 * Display comprehensive printer status and statistics.
 */
void printer_display_status(const PrinterSystem *ps);

/**
 * Serialize the complete printer system state and array queue elements into JSON.
 */
void printer_to_json(const PrinterSystem *ps, char *out_buf, size_t out_max);

#endif /* PRINTER_H */
