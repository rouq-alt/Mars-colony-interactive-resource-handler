#ifndef BANKER_H
#define BANKER_H

#include <stdbool.h>
#include <pthread.h> // Added for Mutex

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10
#define MAX_STATES 100

typedef struct {
    int avail[MAX_RESOURCES];
    int alloc[MAX_PROCESSES][MAX_RESOURCES];
    char safe_seq[MAX_PROCESSES];
    bool is_safe;
    int timestamp;
} SystemState;

extern int num_processes;
extern int num_resources;
extern int available[MAX_RESOURCES];
extern int max[MAX_PROCESSES][MAX_RESOURCES];
extern int allocation[MAX_PROCESSES][MAX_RESOURCES];
extern int need[MAX_PROCESSES][MAX_RESOURCES];
extern int request[MAX_PROCESSES][MAX_RESOURCES];

extern SystemState state_history[MAX_STATES];
extern int state_count;

// System-wide Lock and Simulation Flag
extern pthread_mutex_t sys_lock;
extern bool sim_active;

extern const char* mission_names[10];
extern const char* asset_names[10];

void display_state();
void display_need();
bool is_safe(char safe_seq[]);
void log_state();
bool resource_request(int process_id);
void init_random_system();
void init_interactive();
void display_history();
void performance_analysis();
void* cli_thread_function(void* arg);

#endif