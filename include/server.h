/**
 * ============================================================================
 * File: server.h
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Lightweight POSIX socket HTTP server header in C. Exposes REST
 *              API endpoints and serves the modern frontend dashboard.
 * ============================================================================
 */

#ifndef SERVER_H
#define SERVER_H

#include "printer.h"

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 8192

/**
 * Start the built-in C HTTP Web Server on the specified port.
 * 
 * Parameters:
 *   ps   - Pointer to the shared PrinterSystem instance
 *   port - Port number to bind and listen on (e.g. 8080)
 * 
 * Returns:
 *   0 on normal shutdown, non-zero on binding/socket error.
 */
int server_start(PrinterSystem *ps, int port);

/**
 * Signal handler flag to gracefully stop the web server loop.
 */
void server_stop(void);

#endif /* SERVER_H */
