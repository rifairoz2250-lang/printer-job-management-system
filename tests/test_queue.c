/**
 * ============================================================================
 * File: test_queue.c
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Automated test runner that executes and validates all 7 test
 *              scenarios requested in the project requirements specification.
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/queue.h"
#include "../include/printer.h"

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, name) do { \
    if (cond) { \
        printf("    [PASS] %s\n", name); \
        g_tests_passed++; \
    } else { \
        printf("    [FAIL] %s (Line %d)\n", name, __LINE__); \
        g_tests_failed++; \
    } \
} while(0)

/**
 * TEST 1: Add multiple print jobs.
 */
static void test_case_1_add_multiple_jobs(void) {
    printf("\n=== TEST CASE 1: Add Multiple Print Jobs (Enqueue) ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    PrintJob j1, j2, j3;
    char err[128];

    bool ok1 = printer_add_job(&ps, "Operating_Systems_Unit1.pdf", 12, &j1, err);
    TEST_ASSERT(ok1 == true, "Enqueue Job 1 successfully");
    TEST_ASSERT(j1.job_id == 1001, "Job 1 assigned ID #1001");
    TEST_ASSERT(ps.queue.front == 0 && ps.queue.rear == 0, "Front = 0, Rear = 0 after first job");

    bool ok2 = printer_add_job(&ps, "Data_Structures_Assignment.docx", 5, &j2, err);
    TEST_ASSERT(ok2 == true, "Enqueue Job 2 successfully");
    TEST_ASSERT(j2.job_id == 1002, "Job 2 assigned ID #1002");
    TEST_ASSERT(ps.queue.front == 0 && ps.queue.rear == 1, "Front = 0, Rear = 1 after second job");

    bool ok3 = printer_add_job(&ps, "BCA_Project_Report.pdf", 25, &j3, err);
    TEST_ASSERT(ok3 == true, "Enqueue Job 3 successfully");
    TEST_ASSERT(j3.job_id == 1003, "Job 3 assigned ID #1003");
    TEST_ASSERT(ps.queue.front == 0 && ps.queue.rear == 2, "Front = 0, Rear = 2 after third job");

    TEST_ASSERT(ps.queue.count == 3, "Queue count is exactly 3");
}

/**
 * TEST 2: Display all jobs.
 */
static void test_case_2_display_jobs(void) {
    printf("\n=== TEST CASE 2: Display All Jobs in Queue ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    printer_add_job(&ps, "Python_Lecture_Slide.pdf", 10, NULL, NULL);
    printer_add_job(&ps, "SQL_Lab_Manual.pdf", 8, NULL, NULL);

    printf("  Calling display function (visual verification):\n");
    printer_display_pending(&ps);
    TEST_ASSERT(ps.queue.count == 2, "Queue has 2 jobs ready for display");
}

/**
 * TEST 3: Print jobs in FIFO order.
 */
static void test_case_3_print_fifo_order(void) {
    printf("\n=== TEST CASE 3: Print Jobs in Strict FIFO Order (Dequeue) ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    char err[128];
    PrintJob j1, j2, j3;
    printer_add_job(&ps, "First_Arrived_Doc.pdf", 4, &j1, err);
    printer_add_job(&ps, "Second_Arrived_Doc.pdf", 6, &j2, err);
    printer_add_job(&ps, "Third_Arrived_Doc.pdf", 14, &j3, err);

    PrintJob printed;
    /* First dequeue must return First_Arrived_Doc */
    bool ok1 = printer_print_next(&ps, &printed, err);
    TEST_ASSERT(ok1 == true, "First dequeue succeeded");
    TEST_ASSERT(printed.job_id == j1.job_id, "FIFO: First job printed was First_Arrived_Doc (#1001)");
    TEST_ASSERT(ps.queue.count == 2, "Queue count decreased to 2");

    /* Second dequeue must return Second_Arrived_Doc */
    bool ok2 = printer_print_next(&ps, &printed, err);
    TEST_ASSERT(ok2 == true, "Second dequeue succeeded");
    TEST_ASSERT(printed.job_id == j2.job_id, "FIFO: Second job printed was Second_Arrived_Doc (#1002)");

    /* Third dequeue must return Third_Arrived_Doc */
    bool ok3 = printer_print_next(&ps, &printed, err);
    TEST_ASSERT(ok3 == true, "Third dequeue succeeded");
    TEST_ASSERT(printed.job_id == j3.job_id, "FIFO: Third job printed was Third_Arrived_Doc (#1003)");

    TEST_ASSERT(queue_is_empty(&ps.queue) == true, "Queue is empty after all 3 jobs printed");
    TEST_ASSERT(ps.queue.front == -1 && ps.queue.rear == -1, "Front and Rear reset to -1 when empty");
}

/**
 * TEST 4: Cancel a particular job.
 */
static void test_case_4_cancel_job(void) {
    printf("\n=== TEST CASE 4: Cancel a Particular Job by ID ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    PrintJob j1, j2, j3;
    printer_add_job(&ps, "Keep_Doc_1.pdf", 2, &j1, NULL);
    printer_add_job(&ps, "Cancel_Me_Doc.pdf", 5, &j2, NULL);
    printer_add_job(&ps, "Keep_Doc_3.pdf", 9, &j3, NULL);

    TEST_ASSERT(ps.queue.count == 3, "Initially 3 jobs in queue");

    PrintJob cancelled;
    char err[128];
    bool ok = printer_cancel_job(&ps, j2.job_id, &cancelled, err);
    TEST_ASSERT(ok == true, "Successfully cancelled Job #1002");
    TEST_ASSERT(cancelled.job_id == j2.job_id, "Cancelled job details match target ID");
    TEST_ASSERT(ps.queue.count == 2, "Queue count reduced to 2");

    /* Check that remaining elements preserved order */
    TEST_ASSERT(ps.queue.jobs[0].job_id == j1.job_id, "Slot 0 is still Keep_Doc_1");
    TEST_ASSERT(ps.queue.jobs[1].job_id == j3.job_id, "Slot 1 is now Keep_Doc_3 (shifted forward)");
}

/**
 * TEST 5: Count pending jobs.
 */
static void test_case_5_count_pending_jobs(void) {
    printf("\n=== TEST CASE 5: Count Pending Jobs ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    TEST_ASSERT(printer_get_pending_count(&ps) == 0, "Initial pending count is 0");

    printer_add_job(&ps, "Doc_A.pdf", 1, NULL, NULL);
    TEST_ASSERT(printer_get_pending_count(&ps) == 1, "Pending count is 1 after 1 add");

    printer_add_job(&ps, "Doc_B.pdf", 2, NULL, NULL);
    printer_add_job(&ps, "Doc_C.pdf", 3, NULL, NULL);
    TEST_ASSERT(printer_get_pending_count(&ps) == 3, "Pending count is 3 after 3 adds");

    printer_print_next(&ps, NULL, NULL);
    TEST_ASSERT(printer_get_pending_count(&ps) == 2, "Pending count is 2 after 1 print");
}

/**
 * TEST 6: Try printing from an empty Queue (Underflow).
 */
static void test_case_6_empty_queue_underflow(void) {
    printf("\n=== TEST CASE 6: Try Printing from Empty Queue (Underflow) ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    PrintJob printed;
    char err[128] = {0};

    bool ok = printer_print_next(&ps, &printed, err);
    TEST_ASSERT(ok == false, "Print next on empty queue returned false (safely rejected)");
    TEST_ASSERT(strstr(err, "Underflow") != NULL, "Descriptive Queue Underflow error message returned");
    TEST_ASSERT(ps.queue.front == -1 && ps.queue.rear == -1, "Pointers remain safely at -1");
}

/**
 * TEST 7: Try invalid inputs and Queue Overflow.
 */
static void test_case_7_invalid_inputs(void) {
    printf("\n=== TEST CASE 7: Input Validation and Overflow Handling ===\n");
    PrinterSystem ps;
    printer_init(&ps);

    char err[128];

    /* Invalid 7.1: Empty document name */
    bool ok_empty = printer_add_job(&ps, "", 5, NULL, err);
    TEST_ASSERT(ok_empty == false, "Empty document name rejected");
    TEST_ASSERT(strstr(err, "empty") != NULL, "Error message states name cannot be empty");

    /* Invalid 7.2: Whitespace only document name */
    bool ok_spaces = printer_add_job(&ps, "   ", 5, NULL, err);
    TEST_ASSERT(ok_spaces == false, "Whitespace-only document name rejected");

    /* Invalid 7.3: Zero or negative pages */
    bool ok_neg_pages = printer_add_job(&ps, "Test.pdf", 0, NULL, err);
    TEST_ASSERT(ok_neg_pages == false, "Zero page count rejected");

    bool ok_neg = printer_add_job(&ps, "Test.pdf", -4, NULL, err);
    TEST_ASSERT(ok_neg == false, "Negative page count rejected");

    /* Invalid 7.4: Cancel non-existent Job ID */
    bool ok_bad_cancel = printer_cancel_job(&ps, 9999, NULL, err);
    TEST_ASSERT(ok_bad_cancel == false, "Cancelling non-existent Job ID #9999 rejected");

    /* Invalid 7.5: Queue Overflow Test (fill MAX_QUEUE_SIZE jobs) */
    printf("  Testing Queue Overflow up to MAX_QUEUE_SIZE (%d jobs):\n", MAX_QUEUE_SIZE);
    for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Bulk_Doc_%d.pdf", i + 1);
        bool filled = printer_add_job(&ps, name, 2, NULL, err);
        assert(filled == true);
    }
    TEST_ASSERT(ps.queue.count == MAX_QUEUE_SIZE, "Queue filled to maximum capacity");
    TEST_ASSERT(queue_is_full(&ps.queue) == true, "queue_is_full() returns true");

    /* Attempt to add (MAX_QUEUE_SIZE + 1)th job */
    bool overflow_attempt = printer_add_job(&ps, "Overflow_Doc.pdf", 1, NULL, err);
    TEST_ASSERT(overflow_attempt == false, "Overflow insertion rejected");
    TEST_ASSERT(strstr(err, "Overflow") != NULL, "Descriptive Queue Overflow error message returned");
}

/**
 * Test Runner Entry Point
 */
int run_all_queue_tests(void) {
    g_tests_passed = 0;
    g_tests_failed = 0;

    printf("\n===============================================================\n");
    printf("  RUNNING AUTOMATED TEST SUITE: PRINTER QUEUE IN C (FIFO)      \n");
    printf("===============================================================\n");

    test_case_1_add_multiple_jobs();
    test_case_2_display_jobs();
    test_case_3_print_fifo_order();
    test_case_4_cancel_job();
    test_case_5_count_pending_jobs();
    test_case_6_empty_queue_underflow();
    test_case_7_invalid_inputs();

    printf("\n===============================================================\n");
    printf("  TEST RESULTS SUMMARY\n");
    printf("  Total Passed: %d\n", g_tests_passed);
    printf("  Total Failed: %d\n", g_tests_failed);
    printf("===============================================================\n");

    if (g_tests_failed == 0) {
        printf("🎉 ALL 7 TEST SCENARIOS PASSED WITH ZERO ERRORS!\n\n");
        return 0;
    } else {
        printf("⚠️ SOME TESTS FAILED. Review output above.\n\n");
        return 1;
    }
}

#ifndef STANDALONE_TEST_MAIN
/* When compiled with main.c, run_all_queue_tests is called by main */
#else
int main(void) {
    return run_all_queue_tests();
}
#endif
