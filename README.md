# 🖨️ Printer Job Management System Using Queue in C

> **College Project-Based Learning (PBL) Project**  
> **Course**: Data Structures & Algorithms  
> **Target Audience**: Bachelor of Computer Applications (BCA) / Data Science & Computer Science  
> **Core Technology**: Pure C (C99/C11) with Modern Web Dashboard Interface (Blue, White, Light-Grey Theme)

---

## 📌 1. Project Introduction

In multi-user computing and operating systems, a printer is a shared hardware resource that can physically process only one document stream at a time. When multiple users or processes send documents to be printed simultaneously, the system must buffer incoming print tasks in an organized, fair, and deterministic sequence.

The **Printer Job Management System** is a software engineering project that demonstrates the real-world application of the **Queue data structure** implemented in **C**. It models modern **OS Print Spooling** (Simultaneous Peripheral Operations On-Line) while strictly adhering to the **First-In, First-Out (FIFO)** scheduling principle.

---

## 🎯 2. Project Objectives

1. **Demonstrate FIFO Principle**: Ensure documents print in the exact chronological sequence they are submitted.
2. **Fixed-Size Array Implementation**: Implement the Queue in C using a contiguous array, managing explicit `front` and `rear` variables.
3. **Robust State & Boundary Handling**: Prevent and safely handle **Queue Overflow** (attempting to enqueue when full) and **Queue Underflow** (attempting to print when empty).
4. **Interactive Dual Interface**:
   - A full-featured **Terminal CLI** for standard command-line environments and academic lab evaluations.
   - A modern **Web Dashboard** with a clean blue, white, and light-grey theme connected directly to the C engine.
5. **Dynamic Job Management**: Provide document submission, FIFO dequeue, individual job cancellation by Job ID, live pending job counting, and queue visual layout.

---

## 🧱 3. Data Structure Used: The Queue

A **Queue** is a linear data structure following the **FIFO (First-In, First-Out)** discipline.

### Characteristics in this Project:
- **Storage**: Fixed contiguous array `PrintJob jobs[MAX_QUEUE_SIZE]` (capacity: 10 jobs).
- **`front` Variable**: Points to the index of the document that has been waiting the longest and is scheduled to be printed next.
- **`rear` Variable**: Points to the index of the most recently submitted print job.
- **`count` Variable**: Tracks the active number of pending print jobs.
- **Contiguous Shifting**: Whenever a document is printed or cancelled, subsequent jobs shift left by one position. This ensures `front` is always index `0` whenever jobs exist, completely preventing memory fragmentation and false overflows.

```
                  +----------------------------------------------+
Enqueue (Add) --->| jobs[3]  |  jobs[2]  |  jobs[1]  |  jobs[0]  |---> Dequeue (Print)
(Enters at REAR)  |  (REAR)  |           |           |  (FRONT)  |     (Leaves from FRONT)
                  +----------------------------------------------+
```

---

## 🔄 4. Detailed FIFO Explanation

**FIFO (First-In, First-Out)** guarantees that the document submitted first will be serviced first:

1. **Enqueue (Arrival)**:
   - When User A submits `Assignment.pdf`, it enters at `front = 0`, `rear = 0` (Job #1001).
   - When User B submits `Lab_Report.docx`, it joins behind User A at `rear = 1` (Job #1002).
   - When User C submits `Thesis.pdf`, it joins at `rear = 2` (Job #1003).
2. **Dequeue (Printing)**:
   - When the printer is ready, it selects the job at `front` (`jobs[0]`, which is `Assignment.pdf`).
   - `Assignment.pdf` is printed and removed.
   - All subsequent documents shift forward: `Lab_Report.docx` moves to index 0 (new `front`), and `Thesis.pdf` moves to index 1.
   - The original submission order is 100% preserved.

---

## ✨ 5. Application Features

| Feature | Description | Implementation Details |
|---|---|---|
| **1. Add Print Job (Enqueue)** | Enters document name and page count. Automatically assigns a unique sequential Job ID (e.g. 1001, 1002). Appends the job to the rear of the queue. | `queue_enqueue()` in `queue.c` |
| **2. Display Pending Jobs** | Shows all pending jobs in an orderly table with Position, Job ID, Document Name, Pages, and Submission Time. | `queue_display()` in `queue.c` |
| **3. Print Next Job (Dequeue)** | Dequeues the frontmost document and simulates physical printing. Strictly follows FIFO order. | `queue_dequeue()` in `queue.c` |
| **4. Cancel Print Job** | Allows entering any valid Job ID to cancel that job before it prints. Safely shifts remaining jobs forward. | `queue_cancel()` in `queue.c` |
| **5. Count Pending Jobs** | Displays the current number of pending documents and remaining capacity. | `queue_count()` in `queue.c` |
| **6. Printer Status & Metrics** | Displays lifetime printed count, cancelled count, and the last printed document. | `printer_display_status()` in `printer.c` |
| **7. Safe Exit** | Cleanly terminates the program and releases allocated resources. | Graceful exit handling in `main.c` |

---

## 🛠️ 6. Technologies Used

- **Core Programming Language**: C (Standard C99 / C11)
- **Compiler**: GCC / Clang
- **Build System**: GNU Make
- **Network Interface**: POSIX Sockets (`sys/socket.h`, `netinet/in.h`) for the native C HTTP micro-server
- **Web Frontend**: Modern HTML5, Semantic CSS3 (Flexbox & CSS Grid), Pure Vanilla JavaScript (Fetch API)
- **Styling**: Modern Blue, White, and Light-Grey Theme

---

## 📂 7. Project Folder Structure

```
printer_queue_system/
├── include/
│   ├── queue.h             # PrintJob and PrintQueue structs + prototypes
│   ├── printer.h           # High-level printer management & stats
│   └── server.h            # POSIX socket C micro-server declarations
├── src/
│   ├── queue.c             # Array Queue implementation with front & rear
│   ├── printer.c           # Printer operations and input validation
│   ├── server.c            # Pure C HTTP server + JSON REST API endpoints
│   └── main.c              # Application entry point (CLI, Web, & Tests)
├── web/
│   ├── index.html          # Modern, clean Web Dashboard layout
│   ├── style.css           # Blue, white, light-grey responsive CSS
│   └── app.js              # Real-time UI updates, visual queue track
├── tests/
│   └── test_queue.c        # Automated unit and integration test suite
├── docs/
│   ├── PROJECT_REPORT.md   # Complete academic project report
│   └── VIVA_QUESTIONS.md   # Comprehensive viva voce Q&A preparation
├── Makefile                # Build and execution targets
├── run.sh                  # One-click launch script
└── README.md               # Main project manual (this file)
```

---

## 🚀 8. How to Install & Run

### Prerequisites
- macOS, Linux, or WSL (Windows Subsystem for Linux)
- GCC or Clang compiler
- GNU Make

### Quick Start (One Command)
```bash
cd printer_queue_system
./run.sh
```

### Option A: Launch Modern Web Dashboard (Recommended)
```bash
make run
```
Then open your browser at **[http://localhost:8080](http://localhost:8080)**.

### Option B: Launch Interactive Terminal CLI Mode
```bash
make cli
```
Or directly:
```bash
./printer_system --cli
```

### Option C: Run Automated Verification Tests (All 7 Test Scenarios)
```bash
make test
```

---

## 💻 9. Sample Terminal Output

### Main Menu:
```
====================================================================
     🖨️  PRINTER JOB MANAGEMENT SYSTEM - TERMINAL CLI MODE          
        Data Structure: Linear Queue (Array with Front & Rear)      
====================================================================

----------------------- MAIN MENU -----------------------
  [1] Add Print Job (Enqueue)
  [2] Display Pending Jobs in Queue
  [3] Print Next Job (Dequeue - FIFO Demonstration)
  [4] Cancel Print Job (Remove by Job ID)
  [5] Count Pending Jobs
  [6] Display Printer Dashboard & Statistics
  [7] Exit Application
---------------------------------------------------------
Enter your choice (1-7): 1

--- ADD PRINT JOB (ENQUEUE) ---
Enter Document Name: Final_BCA_Project.pdf
Enter Number of Pages: 25

✅ SUCCESS: Print Job Added!
   Job ID        : #1001
   Document      : Final_BCA_Project.pdf
   Pages         : 25
   Submitted At  : 21:07:02
   Queue Position: #1 (Rear Index: 0)
   FIFO Note     : Job joined at the rear and will wait its turn.
```

### Displaying Pending Jobs:
```
+-------------------------------------------------------------------------+
|                         CURRENT PRINT QUEUE STATUS                      |
+-------------------------------------------------------------------------+
  Capacity: 3/10 jobs  |  Front Index: 0  |  Rear Index: 2
+-------+---------+--------------------------------+-------+--------------+
| Pos   | Job ID  | Document Name                  | Pages | Submitted At |
+-------+---------+--------------------------------+-------+--------------+
| #1 (FRONT) | #1001   | Final_BCA_Project.pdf          | 25    | 21:07:02     |
| #2         | #1002   | Data_Science_Notes.docx        | 8     | 21:07:15     |
| #3 (REAR)  | #1003   | Operating_Systems_Lab.pdf      | 14    | 21:07:30     |
+-------+---------+--------------------------------+-------+--------------+
  FIFO Principle: Job at Pos #1 (FRONT) will be printed next.
```

### Printing Next Job (FIFO Dequeue):
```
--- PRINT NEXT JOB (DEQUEUE) ---

🖨️  NOW PRINTING (FIFO EXECUTION):
   ==========================================
   Job ID        : #1001
   Document Name : Final_BCA_Project.pdf
   Pages         : 25 pages
   Submitted At  : 21:07:02
   ==========================================
   FIFO Principle: This job arrived first at the FRONT and was dequeued first!
   Remaining Jobs in Queue: 2
```

---

## 🧪 10. Automated Test Results

The automated test suite (`tests/test_queue.c`) exercises all 7 required scenarios:
1. **Add multiple print jobs**: Successfully enqueues 3 jobs, auto-assigns IDs, and verifies `front=0, rear=2`.
2. **Display all jobs**: Formats and prints tabular status.
3. **Print jobs in FIFO order**: Sequentially dequeues all jobs and asserts chronological order.
4. **Cancel a particular job**: Locates middle job, cancels it, and verifies forward array shift.
5. **Count pending jobs**: Asserts exact pending counts at every stage.
6. **Print from empty queue**: Confirms safe rejection with a descriptive `Queue Underflow` error.
7. **Invalid inputs**: Confirms rejection of empty strings, negative page numbers, and `Queue Overflow` past 10 jobs.

**Test Summary**: `43 / 43 Assertions Passed (0 Failures)`

---

## 👨‍🏫 11. Viva Voce Preparation
Refer to `docs/VIVA_QUESTIONS.md` for 15 detailed questions and answers covering queue theory, complexity analysis, pointer behavior, circular queues, and OS spooling principles.
