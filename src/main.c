/**
 * ============================================================================
 * File: main.c
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Main entry point providing an Interactive Terminal CLI mode,
 *              a Web Server mode (running the browser dashboard), and an
 *              automated test runner.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/printer.h"
#include "../include/server.h"

/* Forward declaration of external test runner function */
extern int run_all_queue_tests(void);


/**
 * Helper: Safely reads a non-empty string line from stdin.
 */
static bool read_input_string(const char *prompt, char *buffer, size_t max_len) {
    printf("%s", prompt);
    if (!fgets(buffer, (int)max_len, stdin)) {
        return false;
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return true;
}

/**
 * Helper: Safely reads an integer from stdin with input validation.
 */
static bool read_input_int(const char *prompt, int *out_val) {
    char temp[64];
    printf("%s", prompt);
    if (!fgets(temp, sizeof(temp), stdin)) {
        return false;
    }

    char *endptr;
    long val = strtol(temp, &endptr, 10);
    if (endptr == temp || (*endptr != '\n' && *endptr != '\0')) {
        return false;
    }

    *out_val = (int)val;
    return true;
}

/**
 * Interactive Terminal CLI Loop
 */
static void run_cli_mode(PrinterSystem *ps) {
    int choice = 0;

    printf("\n====================================================================\n");
    printf("     🖨️  PRINTER JOB MANAGEMENT SYSTEM - TERMINAL CLI MODE          \n");
    printf("        Data Structure: Linear Queue (Array with Front & Rear)      \n");
    printf("====================================================================\n");

    while (1) {
        printf("\n----------------------- MAIN MENU -----------------------\n");
        printf("  [1] Add Print Job (Enqueue)\n");
        printf("  [2] Display Pending Jobs in Queue\n");
        printf("  [3] Print Next Job (Dequeue - FIFO Demonstration)\n");
        printf("  [4] Cancel Print Job (Remove by Job ID)\n");
        printf("  [5] Count Pending Jobs\n");
        printf("  [6] Display Printer Dashboard & Statistics\n");
        printf("  [7] Exit Application\n");
        printf("---------------------------------------------------------\n");

        if (!read_input_int("Enter your choice (1-7): ", &choice)) {
            printf("\n⚠️  Invalid input! Please enter a number between 1 and 7.\n");
            continue;
        }

        switch (choice) {
            case 1: {
                /* Feature 1: Add Print Job (Enqueue) */
                char doc_name[MAX_DOC_NAME_LEN];
                int pages = 1;

                printf("\n--- ADD PRINT JOB (ENQUEUE) ---\n");
                if (!read_input_string("Enter Document Name (e.g., Assignment.pdf): ", doc_name, sizeof(doc_name))) {
                    printf("⚠️  Error reading document name.\n");
                    break;
                }

                if (!read_input_int("Enter Number of Pages (e.g., 5): ", &pages)) {
                    printf("⚠️  Invalid page count! Please enter a valid positive number.\n");
                    break;
                }

                PrintJob added_job;
                char error_msg[128] = {0};
                if (printer_add_job(ps, doc_name, pages, &added_job, error_msg)) {
                    printf("\n✅ SUCCESS: Print Job Added!\n");
                    printf("   Job ID        : #%d\n", added_job.job_id);
                    printf("   Document      : %s\n", added_job.doc_name);
                    printf("   Pages         : %d\n", added_job.pages);
                    printf("   Submitted At  : %s\n", added_job.submitted_at);
                    printf("   Queue Position: #%d (Rear Index: %d)\n", ps->queue.count, ps->queue.rear);
                    printf("   FIFO Note     : Job joined at the rear and will wait its turn.\n");
                } else {
                    printf("\n❌ FAILED: %s\n", error_msg);
                }
                break;
            }

            case 2: {
                /* Feature 2: Display Pending Jobs */
                printer_display_pending(ps);
                break;
            }

            case 3: {
                /* Feature 3: Print Next Job (Dequeue - FIFO) */
                printf("\n--- PRINT NEXT JOB (DEQUEUE) ---\n");
                PrintJob printed_job;
                char error_msg[128] = {0};

                if (printer_print_next(ps, &printed_job, error_msg)) {
                    printf("\n🖨️  NOW PRINTING (FIFO EXECUTION):\n");
                    printf("   ==========================================\n");
                    printf("   Job ID        : #%d\n", printed_job.job_id);
                    printf("   Document Name : %s\n", printed_job.doc_name);
                    printf("   Pages         : %d pages\n", printed_job.pages);
                    printf("   Submitted At  : %s\n", printed_job.submitted_at);
                    printf("   ==========================================\n");
                    printf("   FIFO Principle: This job arrived first at the FRONT and was dequeued first!\n");
                    printf("   Remaining Jobs in Queue: %d\n", ps->queue.count);
                } else {
                    printf("\n❌ FAILED: %s\n", error_msg);
                }
                break;
            }

            case 4: {
                /* Feature 4: Cancel Print Job */
                printf("\n--- CANCEL PRINT JOB ---\n");
                if (queue_is_empty(&ps->queue)) {
                    printf("⚠️  Queue is empty. No jobs to cancel.\n");
                    break;
                }

                int job_id = 0;
                if (!read_input_int("Enter Job ID to cancel (e.g., 1001): ", &job_id)) {
                    printf("⚠️  Invalid Job ID entered.\n");
                    break;
                }

                PrintJob cancelled_job;
                char error_msg[128] = {0};
                if (printer_cancel_job(ps, job_id, &cancelled_job, error_msg)) {
                    printf("\n🗑️  SUCCESS: Job Cancelled and Removed!\n");
                    printf("   Job ID        : #%d\n", cancelled_job.job_id);
                    printf("   Document Name : %s\n", cancelled_job.doc_name);
                    printf("   Pages         : %d\n", cancelled_job.pages);
                    printf("   Remaining Jobs: %d\n", ps->queue.count);
                    printf("   FIFO Note     : Subsequent jobs shifted forward to maintain queue order.\n");
                } else {
                    printf("\n❌ FAILED: %s\n", error_msg);
                }
                break;
            }

            case 5: {
                /* Feature 5: Count Pending Jobs */
                int count = printer_get_pending_count(ps);
                printf("\n📊 Total Pending Print Jobs in Queue: %d (Maximum Capacity: %d)\n",
                       count, MAX_QUEUE_SIZE);
                if (count == 0) {
                    printf("   Status: The queue is currently empty.\n");
                } else if (count == MAX_QUEUE_SIZE) {
                    printf("   Status: The queue is completely full.\n");
                } else {
                    printf("   Status: %d slots remaining before overflow.\n", MAX_QUEUE_SIZE - count);
                }
                break;
            }

            case 6: {
                /* Display Printer Metrics and Hardware Summary */
                printer_display_status(ps);
                break;
            }

            case 7: {
                /* Feature 6: Exit safely */
                printf("\nExiting Printer Job Management System. Thank you!\n\n");
                return;
            }

            default:
                printf("\n⚠️  Choice must be between 1 and 7. Please try again.\n");
                break;
        }
    }
}

/**
 * Application Entry Point
 */
int main(int argc, char *argv[]) {
    PrinterSystem printer;
    printer_init(&printer);

    int port = DEFAULT_PORT;

    /* Check for command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cli") == 0) {
            run_cli_mode(&printer);
            return 0;
        } else if (strcmp(argv[i], "--web") == 0 || strcmp(argv[i], "--server") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = atoi(argv[i + 1]);
                i++;
            }
            return server_start(&printer, port);
        } else if (strcmp(argv[i], "--test") == 0) {
            return run_all_queue_tests();
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  --cli            Launch interactive terminal CLI mode\n");
            printf("  --web [PORT]     Start C POSIX HTTP Web Server (default port: 8080)\n");
            printf("  --test           Run automated queue test suite (7 test scenarios)\n");
            printf("  --help           Display this help manual\n");
            return 0;
        }
    }

    /* If no arguments passed, prompt user to select mode */
    printf("\n====================================================================\n");
    printf("       🖨️  PRINTER JOB MANAGEMENT SYSTEM USING QUEUE IN C           \n");
    printf("         BCA College Project-Based Learning (PBL) Project           \n");
    printf("====================================================================\n");
    printf("  Select Execution Mode:\n");
    printf("  [1] Interactive Terminal Mode (CLI)\n");
    printf("  [2] Start Modern Web Dashboard (http://localhost:%d)\n", port);
    printf("  [3] Run Automated Verification Tests\n");
    printf("  [4] Exit\n");
    printf("====================================================================\n");

    int mode_choice = 0;
    if (!read_input_int("Enter selection (1-4): ", &mode_choice)) {
        printf("Defaulting to Interactive Terminal Mode...\n");
        mode_choice = 1;
    }

    if (mode_choice == 1) {
        run_cli_mode(&printer);
    } else if (mode_choice == 2) {
        return server_start(&printer, port);
    } else if (mode_choice == 3) {
        return run_all_queue_tests();
    } else {
        printf("Exiting. Goodbye!\n");
    }

    return 0;
}
