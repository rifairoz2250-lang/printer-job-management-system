#!/usr/bin/env bash
# ==============================================================================
# run.sh - Quick Launch Script for Printer Job Management System
# ==============================================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "======================================================================="
echo "   🖨️  PRINTER JOB MANAGEMENT SYSTEM USING QUEUE IN C (COLLEGE PBL)   "
echo "======================================================================="

# Compile if binary does not exist
if [ ! -f "./printer_system" ]; then
    echo "Compiling C application with gcc..."
    make
fi

# Run selection or arguments
if [ "$1" == "--cli" ] || [ "$1" == "cli" ]; then
    ./printer_system --cli
elif [ "$1" == "--test" ] || [ "$1" == "test" ]; then
    ./printer_system --test
elif [ "$1" == "--web" ] || [ "$1" == "web" ] || [ "$1" == "run" ]; then
    echo "Starting Web Application on http://localhost:8080..."
    which open > /dev/null 2>&1 && open http://localhost:8080 || true
    ./printer_system --web 8080
elif [ "$1" == "--py" ] || [ "$1" == "py" ]; then
    python3 server.py 8080
else
    # Default: launch Web Dashboard directly, or pass custom args
    echo "Starting Web Application Dashboard on http://localhost:8080..."
    which open > /dev/null 2>&1 && open http://localhost:8080 || true
    ./printer_system --web 8080
fi
