#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define MAX_PORT 65535
#define MIN_PORT 1

// Global variable to store the child process ID for cleanup
pid_t ssh_pid = 0;

// Signal handler for Ctrl+C
void handle_interrupt(int) {
    if (ssh_pid > 0) {
        printf("\nStopping tunnel...\n");
        kill(ssh_pid, SIGTERM); // Terminate the SSH process
    }
    exit(0);
}

// Check if a port is in use on localhost
int is_port_in_use(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if (errno == EADDRINUSE) {
            close(sockfd);
            return 1; // Port is in use
        }
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0; // Port is free
}

void print_usage(const char *progname) {
    printf("Usage: %s <remotehost> <remoteusername> <port>\n", progname);
    printf("  remotehost: The remote host to connect to (e.g., example.com)\n");
    printf("  remoteusername: The username for the remote host\n");
    printf("  port: The port to forward (1-65535)\n");
}

int main(int argc, char *argv[]) {
    // Check argument count
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse arguments
    const char *remotehost = argv[1];
    const char *remoteusername = argv[2];
    int port = atoi(argv[3]);

    // Validate port
    if (port < MIN_PORT || port > MAX_PORT) {
        printf("Error: Port must be between %d and %d.\n", MIN_PORT, MAX_PORT);
        return 1;
    }

    // Check if port is in use
    int port_status = is_port_in_use(port);
    if (port_status < 0) {
        return 1; // Error already printed
    } else if (port_status == 1) {
        printf("Port %d is already bound. Exiting.\n", port);
        return 1;
    }

    // Set up signal handler for Ctrl+C
    signal(SIGINT, handle_interrupt);

    // Construct SSH command
    char ssh_cmd[512];
    snprintf(ssh_cmd, sizeof(ssh_cmd), 
             "ssh -L %d:localhost:%d %s@%s -N",
             port, port, remoteusername, remotehost);

    // Fork to run SSH command
    ssh_pid = fork();
    if (ssh_pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (ssh_pid == 0) {
        // Child process: execute SSH
        execlp("sh", "sh", "-c", ssh_cmd, NULL);
        perror("Failed to execute SSH"); // Only reached if execlp fails
        exit(1);
    } else {
        // Parent process
        printf("Tunnel established: localhost:%d -> %s:%d. Press Ctrl+C to stop.\n", 
               port, remotehost, port);

        // Wait for child to exit (or be interrupted)
        int status;
        waitpid(ssh_pid, &status, 0);
    }

    return 0;
}
