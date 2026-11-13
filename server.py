#!/usr/bin/env python3
"""
==============================================================================
File: server.py
Project: Printer Job Management System using Queue in C
Target: College PBL Project (BCA Data Science)
Description: Python Local Backend Interface connecting the browser frontend
             (HTML, CSS, JavaScript) to the actual C Queue implementation
             via Python's built-in ctypes foreign function interface.
==============================================================================
"""

import os
import sys
import json
import ctypes
import subprocess
import webbrowser
from http.server import HTTPServer, SimpleHTTPRequestHandler

PORT = 8080
if len(sys.argv) > 1 and sys.argv[1].isdigit():
    PORT = int(sys.argv[1])

# Determine directory paths
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.join(BASE_DIR, "printer_queue_system")
if not os.path.exists(PROJECT_DIR):
    PROJECT_DIR = BASE_DIR

WEB_DIR = os.path.join(PROJECT_DIR, "web")
if not os.path.exists(WEB_DIR):
    WEB_DIR = os.path.join(BASE_DIR, "web")

# Shared Library compilation target
IS_MAC = sys.platform == "darwin"
LIB_NAME = "libprinter_queue.dylib" if IS_MAC else "libprinter_queue.so"
LIB_PATH = os.path.join(PROJECT_DIR, LIB_NAME)

def ensure_compiled_c_library():
    """Compiles the C Queue code into a native shared library if not already built."""
    if not os.path.exists(LIB_PATH):
        print("[Build] Compiling C Queue shared library using GCC/Clang...")
        src_queue = os.path.join(PROJECT_DIR, "src", "queue.c")
        src_printer = os.path.join(PROJECT_DIR, "src", "printer.c")
        inc_dir = os.path.join(PROJECT_DIR, "include")

        cmd = [
            "gcc", "-shared", "-fPIC", "-O2",
            "-I", inc_dir,
            src_queue, src_printer,
            "-o", LIB_PATH
        ]
        res = subprocess.run(cmd)
        if res.returncode != 0:
            print("[Error] Failed to compile C shared library!")
            sys.exit(1)
        print(f"[Build] Successfully built native C library: {LIB_PATH}")

# Compile if needed
ensure_compiled_c_library()

# ==============================================================================
# CTYPES STRUCTURE DEFINITIONS MATCHING C HEADERS (queue.h & printer.h)
# ==============================================================================
class PrintJob(ctypes.Structure):
    _fields_ = [
        ("job_id", ctypes.c_int),
        ("doc_name", ctypes.c_char * 64),
        ("pages", ctypes.c_int),
        ("submitted_at", ctypes.c_char * 32),
    ]

class PrintQueue(ctypes.Structure):
    _fields_ = [
        ("jobs", PrintJob * 10),
        ("front", ctypes.c_int),
        ("rear", ctypes.c_int),
        ("count", ctypes.c_int),
        ("next_job_id", ctypes.c_int),
    ]

class PrinterSystem(ctypes.Structure):
    _fields_ = [
        ("queue", PrintQueue),
        ("last_printed_job", PrintJob),
        ("has_last_printed", ctypes.c_bool),
        ("total_printed_count", ctypes.c_int),
        ("total_cancelled_count", ctypes.c_int),
    ]

# Load native C shared library
try:
    c_lib = ctypes.CDLL(LIB_PATH)
except Exception as e:
    print(f"[Error] Could not load C shared library {LIB_PATH}: {e}")
    sys.exit(1)

# Configure C function signatures
c_lib.printer_init.argtypes = [ctypes.c_void_p]
c_lib.printer_init.restype = None

c_lib.printer_add_job.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_char_p
]
c_lib.printer_add_job.restype = ctypes.c_bool

c_lib.printer_print_next.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p]
c_lib.printer_print_next.restype = ctypes.c_bool

c_lib.printer_cancel_job.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_char_p]
c_lib.printer_cancel_job.restype = ctypes.c_bool

c_lib.printer_to_json.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t]
c_lib.printer_to_json.restype = None

# Initialize the shared C PrinterSystem instance in memory
c_printer = PrinterSystem()
c_lib.printer_init(ctypes.byref(c_printer))

# ==============================================================================
# HTTP REQUEST HANDLER (Serving Web Frontend + Connecting to C Backend)
# ==============================================================================
class PrinterHTTPHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def end_headers(self):
        # Enable CORS for local development
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(204)
        self.end_headers()

    def do_GET(self):
        if self.path == "/api/jobs":
            # Call C function to retrieve queue JSON
            json_buffer = ctypes.create_string_buffer(8192)
            c_lib.printer_to_json(ctypes.byref(c_printer), json_buffer, 8192)
            
            raw_bytes = json_buffer.value
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(raw_bytes)))
            self.end_headers()
            self.wfile.write(raw_bytes)
            return

        # Serve static web frontend files (index.html, style.css, app.js)
        if self.path in ("/", ""):
            self.path = "/index.html"
        return super().do_GET()

    def do_POST(self):
        content_length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_length).decode("utf-8") if content_length > 0 else "{}"
        try:
            req_data = json.loads(body)
        except Exception:
            req_data = {}

        # ENDPOINT: Add Print Job (Enqueue)
        if self.path == "/api/jobs":
            raw_doc = req_data.get("doc_name", "")
            if not isinstance(raw_doc, str):
                raw_doc = str(raw_doc)
            doc_name = raw_doc.encode("utf-8")

            try:
                pages = int(req_data.get("pages", 1))
            except (ValueError, TypeError):
                pages = 0

            added_job = PrintJob()
            err_buf = ctypes.create_string_buffer(128)
            
            # Execute C enqueue logic
            success = c_lib.printer_add_job(
                ctypes.byref(c_printer), doc_name, pages, ctypes.byref(added_job), err_buf
            )

            if success:
                resp = {
                    "success": True,
                    "message": f"Job #{added_job.job_id} enqueued at rear successfully!",
                    "job": {
                        "job_id": added_job.job_id,
                        "doc_name": added_job.doc_name.decode("utf-8"),
                        "pages": added_job.pages,
                        "submitted_at": added_job.submitted_at.decode("utf-8"),
                    }
                }
                status_code = 200
            else:
                resp = {
                    "success": False,
                    "message": err_buf.value.decode("utf-8", errors="ignore")
                }
                status_code = 400

            self._send_json(status_code, resp)
            return

        # ENDPOINT: Print Next Job (Dequeue - FIFO)
        if self.path == "/api/print":
            printed_job = PrintJob()
            err_buf = ctypes.create_string_buffer(128)

            # Execute C dequeue logic (FIFO)
            success = c_lib.printer_print_next(
                ctypes.byref(c_printer), ctypes.byref(printed_job), err_buf
            )

            if success:
                resp = {
                    "success": True,
                    "message": f"Printed Job #{printed_job.job_id} in FIFO order!",
                    "printed_job": {
                        "job_id": printed_job.job_id,
                        "doc_name": printed_job.doc_name.decode("utf-8"),
                        "pages": printed_job.pages,
                    }
                }
                status_code = 200
            else:
                resp = {
                    "success": False,
                    "message": err_buf.value.decode("utf-8", errors="ignore")
                }
                status_code = 400

            self._send_json(status_code, resp)
            return

        # ENDPOINT: Cancel Print Job
        if self.path == "/api/cancel":
            try:
                job_id = int(req_data.get("job_id", 0))
            except (ValueError, TypeError):
                job_id = 0
            cancelled_job = PrintJob()
            err_buf = ctypes.create_string_buffer(128)

            # Execute C cancellation logic
            success = c_lib.printer_cancel_job(
                ctypes.byref(c_printer), job_id, ctypes.byref(cancelled_job), err_buf
            )

            if success:
                resp = {
                    "success": True,
                    "message": f"Job #{cancelled_job.job_id} ('{cancelled_job.doc_name.decode('utf-8')}') was cancelled and removed.",
                    "cancelled_job": {
                        "job_id": cancelled_job.job_id,
                        "doc_name": cancelled_job.doc_name.decode("utf-8"),
                    }
                }
                status_code = 200
            else:
                resp = {
                    "success": False,
                    "message": err_buf.value.decode("utf-8", errors="ignore")
                }
                status_code = 400

            self._send_json(status_code, resp)
            return

        # 404 for unknown routes
        self._send_json(404, {"error": "Endpoint not found"})

    def _send_json(self, status_code, data):
        body = json.dumps(data).encode("utf-8")
        self.send_response(status_code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

def main():
    print("=" * 70)
    print("   🖨️  PRINTER JOB MANAGEMENT WEB APPLICATION (COLLEGE PBL)     ")
    print("=" * 70)
    print(f"  Frontend Dashboard : HTML5 + CSS3 + Vanilla JavaScript")
    print(f"  Backend Data Struct: 100% C Queue via native {os.path.basename(LIB_PATH)}")
    print(f"  Web Server Port    : http://localhost:{PORT}")
    print(f"  FIFO Discipline    : Enforced strictly in C array (front & rear)")
    print("=" * 70)
    print(f"\n🚀 Opening browser at: http://localhost:{PORT}")
    print("Press Ctrl+C in this terminal to stop the server.\n")

    # Try opening default web browser automatically
    try:
        webbrowser.open(f"http://localhost:{PORT}")
    except Exception:
        pass

    server = HTTPServer(("127.0.0.1", PORT), PrinterHTTPHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[Server] Shutting down cleanly. Goodbye!")
        server.server_close()

if __name__ == "__main__":
    main()
