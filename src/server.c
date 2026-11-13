/**
 * ============================================================================
 * File: server.c
 * Project: Printer Job Management System using Queue in C
 * Target Audience: BCA Data Science / Computer Science College Students
 * Description: Pure C HTTP Micro-Server implemented using standard POSIX
 *              sockets (sys/socket.h). Directly connects the HTML/CSS/JS web
 *              dashboard to the C Queue data structure without requiring any
 *              external web frameworks.
 * ============================================================================
 */

#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

static volatile sig_atomic_t g_server_running = 1;
static int g_server_socket_fd = -1;

void server_stop(void) {
    g_server_running = 0;
    if (g_server_socket_fd >= 0) {
        close(g_server_socket_fd);
        g_server_socket_fd = -1;
    }
}

static void sigint_handler(int sig) {
    (void)sig;
    printf("\n[Server] Caught shutdown signal (SIGINT). Stopping cleanly...\n");
    server_stop();
}

/**
 * Helper: Escapes double quotes in string for safe JSON output.
 */
static void json_escape(const char *src, char *dest, size_t dest_len) {
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
 * Helper: Send an HTTP response with headers and body.
 */
static void send_http_response(int client_fd, int status_code, const char *status_text,
                               const char *content_type, const char *body, size_t body_len) {
    char header_buf[1024];
    int header_len = snprintf(header_buf, sizeof(header_buf),
        "HTTP/1.1 %d %s\r\n"
        "Server: PrinterJobQueue-C-Engine/1.0\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_text, content_type, body_len);

    write(client_fd, header_buf, header_len);
    if (body_len > 0 && body != NULL) {
        write(client_fd, body, body_len);
    }
}

/**
 * Helper: Reads a static file from disk (e.g. index.html, style.css, app.js)
 * and sends it to the HTTP client. Checks multiple relative directory locations.
 */
static bool serve_static_file(int client_fd, const char *filepath, const char *content_type) {
    const char *base = strrchr(filepath, '/');
    base = (base != NULL) ? base + 1 : filepath;

    char candidates[5][256];
    snprintf(candidates[0], sizeof(candidates[0]), "%s", filepath);
    snprintf(candidates[1], sizeof(candidates[1]), "web/%s", base);
    snprintf(candidates[2], sizeof(candidates[2]), "printer_queue_system/web/%s", base);
    snprintf(candidates[3], sizeof(candidates[3]), "../web/%s", base);
    snprintf(candidates[4], sizeof(candidates[4]), "./%s", base);

    FILE *fp = NULL;
    for (int i = 0; i < 5; i++) {
        fp = fopen(candidates[i], "rb");
        if (fp) break;
    }

    if (!fp) {
        const char *not_found = "{\"error\": \"File not found\"}";
        send_http_response(client_fd, 404, "Not Found", "application/json", not_found, strlen(not_found));
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(fp);
        return false;
    }

    char *file_buf = (char *)malloc(file_size);
    if (!file_buf) {
        fclose(fp);
        return false;
    }

    size_t read_bytes = fread(file_buf, 1, file_size, fp);
    fclose(fp);

    send_http_response(client_fd, 200, "OK", content_type, file_buf, read_bytes);
    free(file_buf);
    return true;
}

/**
 * Build JSON payload representing the complete state of the Printer System.
 * Delegates directly to the shared C function in printer.c.
 */
static void build_jobs_json(const PrinterSystem *ps, char *out_buf, size_t out_max) {
    printer_to_json(ps, out_buf, out_max);
}

/**
 * Handle a single HTTP client connection.
 */
static void handle_client(int client_fd, PrinterSystem *ps) {
    char req_buf[BUFFER_SIZE];
    memset(req_buf, 0, sizeof(req_buf));

    ssize_t bytes_read = read(client_fd, req_buf, sizeof(req_buf) - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    /* Parse request line: METHOD PATH VERSION */
    char method[16] = {0};
    char path[256] = {0};
    sscanf(req_buf, "%15s %255s", method, path);

    /* Handle CORS preflight */
    if (strcmp(method, "OPTIONS") == 0) {
        send_http_response(client_fd, 204, "No Content", "text/plain", "", 0);
        close(client_fd);
        return;
    }

    /* Locate HTTP body (after \r\n\r\n) */
    char *body_ptr = strstr(req_buf, "\r\n\r\n");
    if (body_ptr) {
        body_ptr += 4;
        size_t header_len = (size_t)(body_ptr - req_buf);
        size_t body_bytes_read = (size_t)(bytes_read - header_len);

        char *cl_header = strstr(req_buf, "Content-Length:");
        if (!cl_header) cl_header = strstr(req_buf, "content-length:");
        if (cl_header) {
            int content_len = atoi(cl_header + 15);
            while (content_len > 0 && body_bytes_read < (size_t)content_len && bytes_read < (ssize_t)(sizeof(req_buf) - 1)) {
                ssize_t n = read(client_fd, req_buf + bytes_read, sizeof(req_buf) - 1 - bytes_read);
                if (n <= 0) break;
                bytes_read += n;
                body_bytes_read += (size_t)n;
                req_buf[bytes_read] = '\0';
            }
        }
    } else {
        body_ptr = "";
    }

    /* ROUTE: GET / or /index.html */
    if (strcmp(method, "GET") == 0 && (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0)) {
        serve_static_file(client_fd, "index.html", "text/html");
        close(client_fd);
        return;
    }

    /* ROUTE: GET /style.css */
    if (strcmp(method, "GET") == 0 && strcmp(path, "/style.css") == 0) {
        serve_static_file(client_fd, "style.css", "text/css");
        close(client_fd);
        return;
    }

    /* ROUTE: GET /app.js */
    if (strcmp(method, "GET") == 0 && strcmp(path, "/app.js") == 0) {
        serve_static_file(client_fd, "app.js", "application/javascript");
        close(client_fd);
        return;
    }

    /* ROUTE: GET /api/jobs (Retrieve queue state) */
    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/jobs") == 0) {
        char json_buf[8192];
        build_jobs_json(ps, json_buf, sizeof(json_buf));
        send_http_response(client_fd, 200, "OK", "application/json", json_buf, strlen(json_buf));
        close(client_fd);
        return;
    }

    /* ROUTE: POST /api/jobs (Enqueue a new print job) */
    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/jobs") == 0) {
        char doc_name[MAX_DOC_NAME_LEN] = {0};
        int pages = 1;

        /* Extract "doc_name" from JSON body */
        char *doc_key = strstr(body_ptr, "\"doc_name\":");
        if (doc_key) {
            char *quote_start = strchr(doc_key + 11, '"');
            if (quote_start) {
                quote_start++;
                char *quote_end = strchr(quote_start, '"');
                if (quote_end) {
                    size_t len = quote_end - quote_start;
                    if (len >= sizeof(doc_name)) len = sizeof(doc_name) - 1;
                    strncpy(doc_name, quote_start, len);
                    doc_name[len] = '\0';
                }
            }
        }

        /* Extract "pages" from JSON body */
        char *pages_key = strstr(body_ptr, "\"pages\":");
        if (pages_key) {
            pages = atoi(pages_key + 8);
        }

        PrintJob added_job;
        char error_msg[128] = {0};
        bool ok = printer_add_job(ps, doc_name, pages, &added_job, error_msg);

        char resp[512];
        if (ok) {
            char escaped[128];
            json_escape(added_job.doc_name, escaped, sizeof(escaped));
            snprintf(resp, sizeof(resp),
                "{\"success\": true, \"message\": \"Job #%d enqueued at rear successfully!\", "
                "\"job\": {\"job_id\": %d, \"doc_name\": \"%s\", \"pages\": %d, \"submitted_at\": \"%s\"}}",
                added_job.job_id, added_job.job_id, escaped, added_job.pages, added_job.submitted_at);
            send_http_response(client_fd, 200, "OK", "application/json", resp, strlen(resp));
        } else {
            char escaped_err[128];
            json_escape(error_msg, escaped_err, sizeof(escaped_err));
            snprintf(resp, sizeof(resp), "{\"success\": false, \"message\": \"%s\"}", escaped_err);
            send_http_response(client_fd, 400, "Bad Request", "application/json", resp, strlen(resp));
        }
        close(client_fd);
        return;
    }

    /* ROUTE: POST /api/print (Dequeue next job in FIFO order) */
    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/print") == 0) {
        PrintJob printed_job;
        char error_msg[128] = {0};
        bool ok = printer_print_next(ps, &printed_job, error_msg);

        char resp[512];
        if (ok) {
            char escaped[128];
            json_escape(printed_job.doc_name, escaped, sizeof(escaped));
            snprintf(resp, sizeof(resp),
                "{\"success\": true, \"message\": \"Printed Job #%d ('%s') in FIFO order!\", "
                "\"printed_job\": {\"job_id\": %d, \"doc_name\": \"%s\", \"pages\": %d}}",
                printed_job.job_id, escaped, printed_job.job_id, escaped, printed_job.pages);
            send_http_response(client_fd, 200, "OK", "application/json", resp, strlen(resp));
        } else {
            char escaped_err[128];
            json_escape(error_msg, escaped_err, sizeof(escaped_err));
            snprintf(resp, sizeof(resp), "{\"success\": false, \"message\": \"%s\"}", escaped_err);
            send_http_response(client_fd, 400, "Bad Request", "application/json", resp, strlen(resp));
        }
        close(client_fd);
        return;
    }

    /* ROUTE: POST /api/cancel (Cancel job by Job ID) */
    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/cancel") == 0) {
        int job_id = 0;
        char *id_key = strstr(body_ptr, "\"job_id\":");
        if (id_key) {
            job_id = atoi(id_key + 9);
        }

        PrintJob cancelled_job;
        char error_msg[128] = {0};
        bool ok = printer_cancel_job(ps, job_id, &cancelled_job, error_msg);

        char resp[512];
        if (ok) {
            char escaped[128];
            json_escape(cancelled_job.doc_name, escaped, sizeof(escaped));
            snprintf(resp, sizeof(resp),
                "{\"success\": true, \"message\": \"Job #%d ('%s') was cancelled and removed.\", "
                "\"cancelled_job\": {\"job_id\": %d, \"doc_name\": \"%s\"}}",
                cancelled_job.job_id, escaped, cancelled_job.job_id, escaped);
            send_http_response(client_fd, 200, "OK", "application/json", resp, strlen(resp));
        } else {
            char escaped_err[128];
            json_escape(error_msg, escaped_err, sizeof(escaped_err));
            snprintf(resp, sizeof(resp), "{\"success\": false, \"message\": \"%s\"}", escaped_err);
            send_http_response(client_fd, 400, "Bad Request", "application/json", resp, strlen(resp));
        }
        close(client_fd);
        return;
    }

    /* Default: 404 Not Found */
    const char *not_found = "{\"error\": \"Endpoint not found\"}";
    send_http_response(client_fd, 404, "Not Found", "application/json", not_found, strlen(not_found));
    close(client_fd);
}

/**
 * server_start()
 * Creates socket, binds to localhost:port, and enters accept loop.
 */
int server_start(PrinterSystem *ps, int port) {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[Server] Socket creation failed");
        return 1;
    }
    g_server_socket_fd = server_fd;

    /* Set SO_REUSEADDR so port is immediately reusable */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[Server] setsockopt failed");
        close(server_fd);
        return 1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[Server] Port bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("[Server] Listen failed");
        close(server_fd);
        return 1;
    }

    printf("\n+-------------------------------------------------------------+\n");
    printf("|       PRINTER QUEUE SYSTEM - C POSIX HTTP SERVER            |\n");
    printf("+-------------------------------------------------------------+\n");
    printf("  Status  : Active & Listening\n");
    printf("  Address : http://localhost:%d\n", port);
    printf("  Serving : Modern Web Dashboard + REST JSON API in Pure C\n");
    printf("  FIFO    : First-In, First-Out Queue Engine Initialized\n");
    printf("+-------------------------------------------------------------+\n");
    printf("  Open http://localhost:%d in your web browser!\n", port);
    printf("  Press Ctrl+C in this terminal to stop the server.\n\n");

    while (g_server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd < 0) {
            if (!g_server_running || errno == EINTR) {
                break;
            }
            continue;
        }

        handle_client(client_fd, ps);
    }

    if (g_server_socket_fd >= 0) {
        close(g_server_socket_fd);
        g_server_socket_fd = -1;
    }

    printf("[Server] Server stopped successfully. Goodbye!\n");
    return 0;
}
