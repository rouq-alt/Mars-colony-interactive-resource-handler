#include "banker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

pthread_mutex_t sys_lock = PTHREAD_MUTEX_INITIALIZER;
bool sim_active = false;

int num_processes, num_resources;
int available[MAX_RESOURCES];
int max[MAX_PROCESSES][MAX_RESOURCES];
int allocation[MAX_PROCESSES][MAX_RESOURCES];
int need[MAX_PROCESSES][MAX_RESOURCES];
int request[MAX_PROCESSES][MAX_RESOURCES];

SystemState state_history[MAX_STATES];
int state_count = 0;

const char* mission_names[10] = {
    "Rover-A", "Ice Drill", "Hab-Build", "Drone-X", "Comms-Fix", 
    "Bio-Lab", "Miner-1", "Solar-Rig", "Astro-T", "Scout-V"
};

const char* asset_names[10] = {
    "O2 Tanks", "Batteries", "Drones", "Panels", "Antennas", 
    "Water", "Heaters", "Tools", "Medkits", "Rations"
};

void display_state() {
    printf("\n=== ARES-1: CURRENT ASSET DEPLOYMENT ===\n");
    printf("Available Assets: ");
    for (int i = 0; i < num_resources; i++) printf("%d ", available[i]);
    printf("\n\nMission    | ");
    for (int j = 0; j < num_resources; j++) printf("A%d ", j);
    printf("\n-----------|");
    for (int j = 0; j < num_resources; j++) printf("---");
    printf("\n");
    for (int i = 0; i < num_processes; i++) {
        printf("M%-2d        | ", i);
        for (int j = 0; j < num_resources; j++) printf("%d  ", allocation[i][j]);
        printf("\n");
    }
    printf("\n");
}

void display_need() {
    printf("\n=== ARES-1: PENDING ASSET NEEDS ===\n");
    printf("Mission    | ");
    for (int j = 0; j < num_resources; j++) printf("A%d ", j);
    printf("\n-----------|");
    for (int j = 0; j < num_resources; j++) printf("---");
    printf("\n");
    for (int i = 0; i < num_processes; i++) {
        printf("M%-2d        | ", i);
        for (int j = 0; j < num_resources; j++) printf("%d  ", need[i][j]);
        printf("\n");
    }
    printf("\n");
}

bool is_safe(char safe_seq[]) {
    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES] = {false};
    for (int i = 0; i < num_resources; i++) work[i] = available[i];
    int safe_count = 0;
    int seq_idx = 0;
    while (safe_count < num_processes) {
        bool found = false;
        for (int p = 0; p < num_processes; p++) {
            if (!finish[p]) {
                bool can_allocate = true;
                for (int j = 0; j < num_resources; j++) {
                    if (need[p][j] > work[j]) {
                        can_allocate = false;
                        break;
                    }
                }
                if (can_allocate) {
                    for (int j = 0; j < num_resources; j++) work[j] += allocation[p][j];
                    finish[p] = true;
                    safe_seq[seq_idx++] = 'M';
                    safe_seq[seq_idx++] = '0' + p;
                    safe_seq[seq_idx++] = ' ';
                    safe_count++;
                    found = true;
                }
            }
        }
        if (!found) return false; 
    }
    safe_seq[seq_idx] = '\0';
    return true;
}

void log_state() {
    if (state_count < MAX_STATES) {
        SystemState *state = &state_history[state_count];
        for (int i = 0; i < num_resources; i++) state->avail[i] = available[i];
        for (int i = 0; i < num_processes; i++) {
            for (int j = 0; j < num_resources; j++) state->alloc[i][j] = allocation[i][j];
        }
        state->is_safe = is_safe(state->safe_seq);
        state->timestamp = state_count++;
    }
}

bool resource_request(int process_id) {
    printf("\n=== ASSET DEPLOYMENT REQUEST FOR MISSION %d ===\n", process_id);
    int req[MAX_RESOURCES];
    for (int i = 0; i < num_resources; i++) {
        req[i] = request[process_id][i];
        printf("Request A%d: %d ", i, req[i]);
    }
    printf("\n");
    for (int i = 0; i < num_resources; i++) {
        if (req[i] > need[process_id][i]) {
            printf("[ERROR] Request exceeds maximum Mission requirements!\n");
            return false;
        }
    }
    for (int i = 0; i < num_resources; i++) {
        if (req[i] > available[i]) {
            printf("[DENIED] Insufficient assets in Base Inventory.\n");
            return false;
        }
    }
    for (int i = 0; i < num_resources; i++) {
        available[i] -= req[i];
        allocation[process_id][i] += req[i];
        need[process_id][i] -= req[i];
    }
    
    bool process_completes = true;
    for (int i = 0; i < num_resources; i++) {
        if (need[process_id][i] > 0) {
            process_completes = false;
            break;
        }
    }
    
    char safe_seq[MAX_PROCESSES * 3];
    if (is_safe(safe_seq) || process_completes) {
        printf("[GRANTED] Deployment successful! Safe sequence: %s%s\n", safe_seq, process_completes ? " (Process completes)" : "");
        log_state();
        return true;
    } else {
        printf("[DENIED - GRIDLOCK RISK] Deployment rolled back.\n");
        for (int i = 0; i < num_resources; i++) {
            available[i] += req[i];
            allocation[process_id][i] -= req[i];
            need[process_id][i] += req[i];
        }
        return false;
    }
}

void init_random_system() {
    srand(time(NULL));
    printf("Initializing randomized ARES-1 conditions...\n");
    for (int i = 0; i < num_resources; i++) available[i] = 5 + rand() % 10; // Increased available resources
    for (int i = 0; i < num_processes; i++) {
        for (int j = 0; j < num_resources; j++) {
            max[i][j] = 3 + rand() % 5;
            allocation[i][j] = rand() % (max[i][j] + 1);
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }
    char safe_seq[MAX_PROCESSES * 3];
    while (!is_safe(safe_seq)) {
        // Increase available resources until safe
        for (int i = 0; i < num_resources; i++) available[i] += 2;
    }
    log_state();
}

void init_interactive() {
    printf("Enter Base Asset Inventory (%d items): ", num_resources);
    for (int i = 0; i < num_resources; i++) scanf("%d", &available[i]);
    printf("\nEnter Maximum Assets Required per Mission (%dx%d):\n", num_processes, num_resources);
    for (int i = 0; i < num_processes; i++) {
        for (int j = 0; j < num_resources; j++) {
            printf("Mission %d, Asset %d: ", i, j);
            scanf("%d", &max[i][j]);
        }
    }
    printf("\nEnter Currently Deployed Assets:\n");
    for (int i = 0; i < num_processes; i++) {
        for (int j = 0; j < num_resources; j++) {
            printf("Mission %d, Asset %d: ", i, j);
            scanf("%d", &allocation[i][j]);
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }
    log_state();
}

void display_history() {
    printf("\n=== MISSION LOG (%d events) ===\n", state_count);
    for (int i = 0; i < state_count; i++) {
        printf("Log %d [T=%d] %s | Seq: %s\n", 
               i, state_history[i].timestamp,
               state_history[i].is_safe ? "MISSION GO" : "GRIDLOCK",
               state_history[i].safe_seq);
    }
}

void performance_analysis() {
    int safe_states = 0, unsafe_states = 0;
    for (int i = 0; i < state_count; i++) {
        if (state_history[i].is_safe) safe_states++;
        else unsafe_states++;
    }
    printf("\n=== SYSTEM HEALTH ANALYSIS ===\n");
    printf("Total Logs: %d\n", state_count);
    printf("Safe States: %d (%.1f%%)\n", safe_states, state_count ? (safe_states * 100.0 / state_count) : 0);
    printf("Gridlock Risks Prevented: %d (%.1f%%)\n", unsafe_states, state_count ? (unsafe_states * 100.0 / state_count) : 0);
}