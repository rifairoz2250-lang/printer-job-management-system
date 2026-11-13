# 🎓 COLLEGE PBL PROJECT REPORT

## **PRINTER JOB MANAGEMENT SYSTEM USING QUEUE IN C**

* **Academic Program**: Bachelor of Computer Applications (BCA) / Data Science
* **Course**: Data Structures & Algorithms (PBL - Project-Based Learning)
* **Language**: C (Standard C99/C11) & Web Frontend Interface
* **Core Data Structure**: Linear Queue (Array with `front` and `rear` pointers)
* **Author / Student**: BCA Data Science Student

---

## 📑 TABLE OF CONTENTS
1. [Abstract](#1-abstract)
2. [Introduction & Background](#2-introduction--background)
3. [Project Objectives](#3-project-objectives)
4. [Data Structure Architecture (Queue & FIFO)](#4-data-structure-architecture-queue--fifo)
5. [System Modules & Code Organization](#5-system-modules--code-organization)
6. [Detailed Algorithms](#6-detailed-algorithms)
7. [Time & Space Complexity Analysis](#7-time--space-complexity-analysis)
8. [User Interface Design (Blue & White Theme)](#8-user-interface-design-blue--white-theme)
9. [Testing, Validation & Results](#9-testing-validation--results)
10. [Conclusion & Future Enhancements](#10-conclusion--future-enhancements)

---

## 1. ABSTRACT

In modern computing, multiple users or processes frequently request access to a shared physical resource such as a network printer. Because a physical printer can only print one physical document at a time, incoming print jobs must be buffered in an organized, fair, and deterministic sequence.

This project implements a complete **Printer Job Management System** in the **C programming language** using an **array-based Queue data structure** governed strictly by the **FIFO (First-In, First-Out)** discipline. Each submitted print job is tracked with a unique Job ID, document name, page count, and timestamp. The system provides real-time job submission (enqueue), print execution (dequeue), specific job cancellation, pending job counting, and queue state visualization through both a terminal CLI and a modern blue-and-white web dashboard.

---

## 2. INTRODUCTION & BACKGROUND

### 2.1 The Concept of Spooling
In operating systems, the printer management mechanism is known as **SPOOLing** (**S**imultaneous **P**eripheral **O**perations **O**n-**L**ine). When multiple applications generate print commands simultaneously, the CPU operates thousands of times faster than mechanical printer hardware. If applications were blocked waiting for the printer, system throughput would crash.

Instead, the OS spools documents into a temporary storage structure: a **Print Queue**.

### 2.2 Why Use a Queue?
A **Queue** is a linear data structure that operates under the **FIFO (First-In, First-Out)** rule:
* The first element inserted into the queue is the first one to be removed.
* In everyday life, this mirrors a line of people waiting at a ticket counter.
* In a printer system, fairness requires that whoever submitted their document first gets their document printed first.

---

## 3. PROJECT OBJECTIVES

1. **Implement Core Queue Logic in C**: Build the Queue using an array without relying on high-level language libraries or replacing C with another language.
2. **Explicit Pointer Tracking**: Maintain explicit `front` and `rear` integer variables to mark the boundaries of active jobs.
3. **Core Operations**:
   - `Enqueue`: Append a new print job at the rear.
   - `Dequeue`: Extract and simulate printing of the document at the front.
   - `Cancel`: Locate any job by its Job ID and remove it safely.
   - `Count`: Provide the exact count of active pending jobs.
4. **Boundary Condition Safety**: Rigorously detect and handle **Queue Overflow** (when capacity is reached) and **Queue Underflow** (when attempting to print from an empty buffer).
5. **Modern Visual Dashboard**: Provide a clean, college-ready interface using a professional **blue, white, and light-grey color palette**.

---

## 4. DATA STRUCTURE ARCHITECTURE (QUEUE & FIFO)

### 4.1 PrintJob Structure
Each document waiting in the queue is encapsulated in a C `struct`:

```c
typedef struct {
    int job_id;                          /* Unique ID: 1001, 1002, ... */
    char doc_name[MAX_DOC_NAME_LEN];     /* e.g., "OperatingSystems.pdf" */
    int pages;                           /* Page count (e.g., 12) */
    char submitted_at[MAX_TIME_STR_LEN]; /* Timestamp: "14:20:05" */
} PrintJob;
```

### 4.2 PrintQueue Structure
The queue container maintains a fixed-capacity contiguous array and state variables:

```c
typedef struct {
    PrintJob jobs[MAX_QUEUE_SIZE];  /* Array buffer (size = 10) */
    int front;                      /* Index of next job to print */
    int rear;                       /* Index of latest added job */
    int count;                      /* Number of pending jobs */
    int next_job_id;                /* Auto-incrementing ID generator */
} PrintQueue;
```

### 4.3 State Transitions of `front` and `rear`

| Operation | Condition | `front` | `rear` | `count` | Description |
|---|---|---|---|---|---|
| **Initial State** | Empty | `-1` | `-1` | `0` | Buffer cleared, no active jobs. |
| **First Enqueue** | Empty $\to$ 1 Job | `0` | `0` | `1` | `front` moves to 0; `rear` moves to 0. |
| **Subsequent Enqueue** | $k$ Jobs | `0` | `k` | `k + 1` | `rear` increments by 1; `front` stays at 0. |
| **Dequeue (Print)** | Multiple Jobs | `0` | `rear - 1` | `count - 1` | Front job removed; remaining elements shifted left. |
| **Dequeue (Last Job)** | 1 Job $\to$ 0 | `-1` | `-1` | `0` | Queue returns to initial empty state. |

---

## 5. SYSTEM MODULES & CODE ORGANIZATION

```
printer_queue_system/
├── include/
│   ├── queue.h       # Queue structures, limits, and function prototypes
│   ├── printer.h     # High-level printer state, metrics, and simulation
│   └── server.h      # POSIX socket C micro-server header
├── src/
│   ├── queue.c       # Array queue logic (enqueue, dequeue, cancel, count)
│   ├── printer.c     # Print job coordinator and metric tracking
│   ├── server.c      # Pure C HTTP server + JSON REST API endpoints
│   └── main.c        # CLI menu, web launcher, and test dispatcher
├── web/
│   ├── index.html    # Modern HTML5 Dashboard layout
│   ├── style.css     # Blue, White, Light-Grey CSS3 design
│   └── app.js        # Dynamic UI updates, visual queue track, API fetch
├── tests/
│   └── test_queue.c  # Automated test suite (all 7 test cases)
├── docs/
│   ├── PROJECT_REPORT.md  # Formal academic report
│   └── VIVA_QUESTIONS.md  # Viva voce questions and answers
├── Makefile          # Build automation
├── run.sh            # Quick launcher script
└── README.md         # Comprehensive project documentation
```

---

## 6. DETAILED ALGORITHMS

### 6.1 Enqueue Algorithm (Adding a Print Job)
```
Step 1: Start.
Step 2: Check if count == MAX_QUEUE_SIZE.
        If YES, report "Queue Overflow: Maximum capacity reached" and EXIT.
Step 3: Validate input parameters:
        - If doc_name is empty or whitespace, report validation error and EXIT.
        - If pages <= 0, report validation error and EXIT.
Step 4: If queue is empty (count == 0 or front == -1):
        Set front = 0.
        Set rear = 0.
        Else:
        Set rear = rear + 1.
Step 5: Create new PrintJob:
        - job_id = next_job_id (then increment next_job_id by 1).
        - doc_name = input doc_name.
        - pages = input pages.
        - submitted_at = current system time.
Step 6: Assign jobs[rear] = new PrintJob.
Step 7: Increment count by 1.
Step 8: Return SUCCESS.
```

### 6.2 Dequeue Algorithm (Printing Next Job - FIFO)
```
Step 1: Start.
Step 2: Check if queue is empty (count == 0 or front == -1).
        If YES, report "Queue Underflow: No print jobs pending" and EXIT.
Step 3: Retrieve jobs[front] as the printed document.
Step 4: Shift all subsequent jobs forward:
        FOR i = 0 TO rear - 1:
            jobs[i] = jobs[i + 1]
Step 5: Zero-fill the vacated jobs[rear] position.
Step 6: Decrement rear by 1.
Step 7: Decrement count by 1.
Step 8: If count == 0:
        Set front = -1.
        Set rear = -1.
Step 9: Output the details of the printed document.
Step 10: Return SUCCESS.
```

### 6.3 Cancel Job Algorithm (Removing by Job ID)
```
Step 1: Start.
Step 2: Check if queue is empty. If YES, report error and EXIT.
Step 3: Initialize target_index = -1.
Step 4: FOR i = 0 TO rear:
            IF jobs[i].job_id == requested_id THEN:
                target_index = i
                BREAK
Step 5: IF target_index == -1 THEN:
            Report "Job ID not found in queue" and EXIT.
Step 6: Save jobs[target_index] to return details.
Step 7: Shift remaining elements forward to fill the gap:
        FOR i = target_index TO rear - 1:
            jobs[i] = jobs[i + 1]
Step 8: Zero-fill jobs[rear].
Step 9: Decrement rear by 1, decrement count by 1.
Step 10: IF count == 0 THEN set front = -1, rear = -1.
Step 11: Return SUCCESS.
```

---

## 7. TIME & SPACE COMPLEXITY ANALYSIS

| Operation | Time Complexity | Space Complexity | Explanation |
|---|---|---|---|
| `queue_init()` | $O(1)$ | $O(1)$ | Resets pointer integers and zeros fixed buffer. |
| `queue_is_empty()` | $O(1)$ | $O(1)$ | Direct comparison of `count == 0`. |
| `queue_is_full()` | $O(1)$ | $O(1)$ | Direct comparison of `count == MAX_QUEUE_SIZE`. |
| `queue_peek()` | $O(1)$ | $O(1)$ | Direct array access at `jobs[front]`. |
| `queue_count()` | $O(1)$ | $O(1)$ | Returns stored `count` variable. |
| `queue_enqueue()` | $O(1)$ | $O(1)$ | Appends directly at index `jobs[rear + 1]`. |
| `queue_dequeue()` | $O(n)$ | $O(1)$ | Dequeues index 0, shifts $n-1$ elements forward to maintain contiguous array. |
| `queue_cancel()` | $O(n)$ | $O(1)$ | Linear scan to find Job ID ($O(n)$) + shifting forward ($O(n)$). |

*Note for Viva*: Shifting takes $O(n)$ time but guarantees that `front` is always index 0, eliminating memory fragmentation and making array slot indices completely predictable for students and hardware simulation.

---

## 8. USER INTERFACE DESIGN (BLUE & WHITE THEME)

The user interface follows professional UI/UX standards:
1. **Color Scheme**:
   - Primary Blue: `#2563EB` (Royal Blue) & `#1E40AF` (Deep Blue)
   - Backgrounds: `#F8FAFC` (Light-Grey) & `#FFFFFF` (Pure White)
   - Accents: `#10B981` (Success Green), `#EF4444` (Danger Red)
2. **Key Dashboard Components**:
   - **Metrics Bar**: Live cards for Pending Count, Front Job, Rear Job, and Lifetime Printed stats.
   - **Visual Queue Track**: 10 horizontal memory slots showing indices `[0]` to `[9]`, highlighting `FRONT` and `REAR` tags dynamically.
   - **Action Controls**: Add Job form, Print Next button, and Cancel Job form.
   - **Pending Jobs Table**: Clean tabular view with real-time badges and individual quick-cancel buttons.
   - **Empty State Illustration**: Visual indicator when buffer contains zero jobs.

---

## 9. TESTING, VALIDATION & RESULTS

An automated test suite (`tests/test_queue.c`) verifies all 7 critical PBL scenarios:

| # | Test Scenario | Expected Result | Actual Result | Status |
|---|---|---|---|---|
| **1** | Add multiple jobs | IDs 1001, 1002, 1003 assigned; front=0, rear=2 | Verified | **PASS** |
| **2** | Display pending jobs | Formatted table output showing positions and order | Verified | **PASS** |
| **3** | FIFO Print Order | Dequeue yields #1001 first, then #1002, then #1003 | Verified | **PASS** |
| **4** | Cancel specific job | Cancel middle job (#1002); remaining elements shift | Verified | **PASS** |
| **5** | Count pending jobs | Accurately returns 0, 1, 3, 2 after respective steps | Verified | **PASS** |
| **6** | Print from empty queue | Returns false; reports "Queue Underflow" | Verified | **PASS** |
| **7** | Invalid inputs & overflow | Rejects empty name, $\le 0$ pages, 11th job overflow | Verified | **PASS** |

**Summary**: 43 out of 43 assertions passed with 0 failures.

---

## 10. CONCLUSION & FUTURE ENHANCEMENTS

### Conclusion
The **Printer Job Management System** demonstrates the practical utility of the Queue data structure in solving real-world concurrency and resource-sharing problems. By implementing the core logic in C using an array with explicit `front` and `rear` pointers, the project grounds theoretical computer science principles into working software.

### Future Enhancements
1. **Priority Scheduling**: Extend the linear queue to a Priority Queue where urgent documents (e.g., Executive or Medical emergency prints) jump ahead of standard documents.
2. **Circular Queue Variant**: Implement circular modular arithmetic `(rear + 1) % MAX` to achieve $O(1)$ dequeue without shifting elements.
3. **Multi-Printer Pool**: Connect multiple printer hardware endpoints to distribute jobs across available printers simultaneously.
