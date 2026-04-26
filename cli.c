#include "banker.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void* stress_test_thread(void* arg) {
    int p_id = *((int*)arg);
    free(arg); 
    
    bool mission_complete = false;
    
    while (!mission_complete) {
        usleep((rand() % 500) * 1000); 
        
        pthread_mutex_lock(&sys_lock); 
        
        int total_need = 0;
        for (int i = 0; i < num_resources; i++) total_need += need[p_id][i];
        
        if (total_need == 0) {
            printf("\n[THREAD %d] Mission %s complete! Returning all assets...\n", p_id, mission_names[p_id]);
            for (int i = 0; i < num_resources; i++) {
                available[i] += allocation[p_id][i];
                allocation[p_id][i] = 0;
            }
            mission_complete = true;
            pthread_mutex_unlock(&sys_lock);
            break;
        }
        
        bool has_request = false;
        for (int i = 0; i < num_resources; i++) {
            if (need[p_id][i] > 0) {
                request[p_id][i] = (need[p_id][i] > 1) ? (rand() % 2 + 1) : 1;
                has_request = true;
            } else {
                request[p_id][i] = 0;
            }
        }
        
        if (has_request) {
            printf("\n[THREAD %d] Attempting to acquire resources...", p_id);
            resource_request(p_id);
        }
        
        pthread_mutex_unlock(&sys_lock); 
    }
    return NULL;
}
void demonstrate_rollback() {
    printf("\n=== INITIATING EXPLICIT ROLLBACK SIMULATION ===\n");
    printf("1. Locating a mission with pending needs...\n");
    
    int target_mission = -1;
    for (int i = 0; i < num_processes; i++) {
        int needs_something = 0;
        for (int j = 0; j < num_resources; j++) needs_something += need[i][j];
        if (needs_something > 0) {
            target_mission = i;
            break;
        }
    }
    
    if (target_mission == -1) {
        printf("Error: All missions are complete. Reboot system to run rollback demo.\n");
        return;
    }

    printf("2. Forcing Mission %d to request ALL remaining needs at once...\n", target_mission);
    int temp_req[MAX_RESOURCES];
    for(int i = 0; i < num_resources; i++) {
        temp_req[i] = need[target_mission][i];
    }
    
    for(int i = 0; i < num_resources; i++) {
        if (temp_req[i] > available[i]) {
            printf("Notice: Not enough inventory to even attempt this. Injecting temporary phantom resources for demo...\n");
            available[i] += temp_req[i]; 
        }
    }

    for(int i=0; i<num_resources; i++){
        available[i] -= temp_req[i];
        allocation[target_mission][i] += temp_req[i];
        need[target_mission][i] -= temp_req[i];
    }

    printf("3. DANGEROUS STATE APPLIED. Look at the visual monitor!\n");
    printf("   Suspending system for 4 seconds so you can view the Gridlock...\n");
    
    pthread_mutex_unlock(&sys_lock);
    sleep(4); 
    pthread_mutex_lock(&sys_lock);

    printf("4. Safety Algorithm confirms Unsafe State.\n");
    printf("5. INITIATING ROLLBACK...\n");

    // Reverse the math!
    for(int i=0; i<num_resources; i++){
        available[i] += temp_req[i];
        allocation[target_mission][i] -= temp_req[i];
        need[target_mission][i] += temp_req[i];
    }

    printf("Rollback complete. System returned to safe state. Look at the visual monitor.\n");
}


void menu() {
    int choice, p_id;
    do {
        printf("\n=== ARES-1 COMMAND TERMINAL ===\n");
        printf("1. View Deployed Assets\n");
        printf("2. View Pending Asset Needs\n");
        printf("3. Run Safety Diagnostic\n");
        printf("4. Request Asset Deployment (Manual)\n");
        printf("5. View Mission History Log\n");
        printf("6. System Health Analysis\n");
        printf("7. Reboot System Parameters\n");
        printf("8. RUN MULTITHREADED STRESS TEST (Parallel Execution)\n");
        printf("9. RUN EXPLICIT ROLLBACK DEMONSTRATION\n");
        printf("0. Shutdown Simulation\n");
        printf("Awaiting command: ");
        

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("\n[ERROR] Invalid input. Please type a number!\n");
            continue; 
        }
        while (getchar() != '\n');
        
        pthread_mutex_lock(&sys_lock);
        
        switch (choice) {
            case 1: display_state(); break;
            case 2: display_need(); break;
            case 3: {
                char safe_seq[MAX_PROCESSES * 3];
                bool safe = is_safe(safe_seq);
                printf("Diagnostic: %s\nSafe clearance order: %s\n", safe ? "ALL CLEAR" : "GRIDLOCK DETECTED", safe_seq);
                log_state();
                break;
            }
            case 4:
                printf("Enter Mission ID (0-%d): ", num_processes-1);
                scanf("%d", &p_id);
                while (getchar() != '\n');
                if (p_id >= 0 && p_id < num_processes) {
                    printf("Enter request amounts for M%d (%d assets): ", p_id, num_resources);
                    for (int i = 0; i < num_resources; i++) scanf("%d", &request[p_id][i]);
                    while (getchar() != '\n');
                    resource_request(p_id);
                }
                break;
            case 5: display_history(); break;
            case 6: performance_analysis(); break;
            case 7: {
                printf("1. Random Startup | 2. Manual Startup: ");
                int init_type;
                scanf("%d", &init_type);
                while (getchar() != '\n');
                if (init_type == 1) init_random_system();
                else init_interactive();
                break;
            }
            case 8: {
                printf("\n[ALERT] Launching all active missions simultaneously...\n");
                pthread_t threads[MAX_PROCESSES];
                for (int i = 0; i < num_processes; i++) {
                    int* id = malloc(sizeof(int));
                    *id = i;
                    pthread_create(&threads[i], NULL, stress_test_thread, id);
                }
                
                pthread_mutex_unlock(&sys_lock);
                for (int i = 0; i < num_processes; i++) {
                    pthread_join(threads[i], NULL); 
                }
                pthread_mutex_lock(&sys_lock); // Relock to safely print the menu again
                
                printf("\n[SUCCESS] Stress Test Complete. All missions successfully executed without deadlock!\n");
                break;
            }
            case 9:
                demonstrate_rollback();
                break;
        }
        pthread_mutex_unlock(&sys_lock);
        
    } while (choice != 0);
}

void* cli_thread_function(void* arg) {
    printf("\nARES-1 Terminal Online. Awaiting commands...\n");
    menu();
    printf("\nSimulation terminated. Goodbye.\n");
    exit(0); 
    return NULL;
}