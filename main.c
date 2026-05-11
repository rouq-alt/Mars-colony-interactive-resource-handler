#include "raylib.h"
#include "banker.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 


Color deepSpaceBg = { 10, 14, 25, 255 };      
Color holoBlue    = { 0, 229, 255, 255 };     
Color safeGreen   = { 0, 255, 102, 255 };     
Color warningRed  = { 255, 51, 102, 255 };    
Color solarYellow = { 255, 204, 0, 255 };     
Color panelBorder = { 35, 50, 80, 255 };      
Color textDim     = { 150, 170, 200, 255 };   
Color textBright  = { 240, 248, 255, 255 };   

Font spaceFont;
Rectangle requestButtons[MAX_PROCESSES];

void DrawSpaceText(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(spaceFont, text, (Vector2){(float)x, (float)y}, (float)size, 1.0f, color);
}

void DrawMatrix(int startX, int startY, const char* title, int matrix[MAX_PROCESSES][MAX_RESOURCES], Color color, bool drawButtons) {
    if (title[0] != '\0') {
        DrawSpaceText(title, startX, startY, 20, textBright); 
    }
    
    int cellWidth = 70; 
    int cellHeight = 28; 
    int colOffset = 120; 
    
    for (int j = 0; j < num_resources; j++) {
        DrawSpaceText(asset_names[j], startX + colOffset + 5 + j * cellWidth, startY + 25, 14, textDim); 
    }
    
    for (int i = 0; i < num_processes; i++) {
        DrawSpaceText(mission_names[i], startX, startY + 50 + i * cellHeight, 16, textDim); 
        
        if (drawButtons) {
            requestButtons[i] = (Rectangle){startX - 25, startY + 48 + i * cellHeight, 18, 18};
            DrawRectangleRec(requestButtons[i], panelBorder);
            DrawSpaceText("R", startX - 20, startY + 51 + i * cellHeight, 12, holoBlue);
        }

        for (int j = 0; j < num_resources; j++) {
            DrawRectangleLines(startX + colOffset + j * cellWidth, startY + 45 + i * cellHeight, cellWidth, cellHeight, panelBorder);
            DrawSpaceText(TextFormat("%d", matrix[i][j]), startX + colOffset + 28 + j * cellWidth, startY + 50 + i * cellHeight, 18, color); 
        }
    }
}

bool is_complete(int p_id) {
    for (int j = 0; j < num_resources; j++) {
        if (need[p_id][j] > 0) return false;
    }
    return true;
}

void DrawCargoBelts(int startX, int startY) {
    int beltWidth = 320;
    int beltHeight = 20; 
    int cellHeight = 28; 
    int colOffset = 120;
    
    for (int i = 0; i < num_processes; i++) {
        DrawSpaceText(mission_names[i], startX, startY + 50 + (i * cellHeight), 16, textDim); 
        int total_max = 0;
        int total_alloc = 0;
        for (int j = 0; j < num_resources; j++) {
            total_max += max[i][j];
            total_alloc += allocation[i][j];
        }
        
        float progress = 0.0f;
        if (total_max > 0) progress = (float)total_alloc / (float)total_max;
        
        DrawRectangle(startX + colOffset, startY + 48 + (i * cellHeight), beltWidth, beltHeight, panelBorder);
        
        Color beltFillColor = (progress >= 1.0f) ? safeGreen : holoBlue;
        DrawRectangle(startX + colOffset, startY + 48 + (i * cellHeight), (int)(beltWidth * progress), beltHeight, beltFillColor);
        
        if (!is_complete(i) && progress > 0.05f) {
            double time = GetTime() * (1.0f + (total_max / 5.0f)); 
            int offset = ((int)(time * 50.0f)) % beltWidth;

            if (offset < (int)(beltWidth * progress)) {
                DrawRectangle(startX + colOffset + offset, startY + 52 + (i * cellHeight), 12, 12, textBright);
            }
        }
        DrawRectangleLines(startX + colOffset, startY + 48 + (i * cellHeight), beltWidth, beltHeight, holoBlue);
        DrawSpaceText(TextFormat("%d%%", (int)(progress * 100)), startX + colOffset + 10 + beltWidth, startY + 50 + (i * cellHeight), 16, textBright); 
    }
}

void DrawSafeSequenceRunway(int startX, int startY, const char* safe_seq_str) {
    DrawSpaceText("BANKER'S SAFE ROUTING DEPARTURE PATH", startX, startY, 20, safeGreen); 
    
    char parsed_seq[MAX_PROCESSES][3]; 
    int count = 0;
    
    char* temp_str = strdup(safe_seq_str);
    char* token = strtok(temp_str, " ");
    while (token != NULL && count < MAX_PROCESSES) {
        strcpy(parsed_seq[count++], token);
        token = strtok(NULL, " ");
    }
    free(temp_str);
    
    int boxWidth = 80;
    int boxHeight = 45; 
    int spacing = 20;

    for (int i = 0; i < count; i++) {
        int x = startX + (i * (boxWidth + spacing + 15)); 
        int y = startY + 30;
        DrawRectangleLines(x, y, boxWidth, boxHeight, safeGreen);
        DrawRectangle(x+2, y+2, boxWidth-4, boxHeight-4, (Color){0, 50, 20, 180}); 
        
        DrawSpaceText(TextFormat("STEP %d", i+1), x + 8, y + 5, 12, textDim); 
        DrawSpaceText(parsed_seq[i], x + 25, y + 22, 18, textBright); 
        
        if (i < count - 1) {
            int arrowX = x + boxWidth + spacing - 10;
            int arrowY = y + (boxHeight / 2);
            DrawLineEx((Vector2){(float)x + boxWidth + 2, (float)arrowY}, (Vector2){(float)arrowX + 8, (float)arrowY}, 2.0f, safeGreen);
            DrawTriangle((Vector2){(float)arrowX + 2, (float)arrowY - 6}, (Vector2){(float)arrowX + 2, (float)arrowY + 6}, (Vector2){(float)arrowX + 10, (float)arrowY}, safeGreen);
        }
    }
}

void HandleMouseInteractivity() {
    Vector2 mousePos = GetMousePosition();

    for (int i = 0; i < num_processes; i++) {
        if (CheckCollisionPointRec(mousePos, requestButtons[i])) {
            DrawRectangleRec(requestButtons[i], textBright); 
            DrawSpaceText("R", requestButtons[i].x + 4, requestButtons[i].y + 2, 10, deepSpaceBg);
            DrawSpaceText(TextFormat("Click to AUTO-REQUEST assets for %s", mission_names[i]), mousePos.x + 15, mousePos.y + 15, 14, holoBlue);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                pthread_mutex_lock(&sys_lock); 
                
                if (is_complete(i)) {
                    printf("\n[ERROR] %s is already complete!\n", mission_names[i]);
                } else {
                    bool has_req = false;
                    for (int j = 0; j < num_resources; j++) {
                        if (need[i][j] > 0) {
                            request[i][j] = (need[i][j] > 1) ? (rand() % 2 + 1) : 1; 
                            has_req = true;
                        } else {
                            request[i][j] = 0;
                        }
                    }
                    if (has_req) {
                        resource_request(i); 
                    }
                }
                pthread_mutex_unlock(&sys_lock); 
            }
        }
    }
}

int main() {
    srand(time(NULL));

    printf("MARS BASE: MISSION CONTROL\n");
    printf("==================================================\n");
    printf("Enter number of Active Missions (Max 10): ");
    scanf("%d", &num_processes);
    printf("Enter number of Asset Types (Max 10): ");
    scanf("%d", &num_resources);

    printf("\n--- INITIALIZATION SEQUENCE ---\n");
    printf("1. Random Asset Generation | 2. Manual Asset Entry: ");
    int init_type;
    scanf("%d", &init_type);
    if (init_type == 1) {
        init_random_system();
    } else {
        init_interactive();
    }

    pthread_t cli_thread;
    pthread_create(&cli_thread, NULL, cli_thread_function, NULL);
    InitWindow(1200, 880, "Mars Colony Interactive Resource Manager");
    SetWindowState(FLAG_WINDOW_TOPMOST); 
    spaceFont = LoadFont("orbitron.ttf");
    SetTargetFPS(60);  

    while (!WindowShouldClose()) {
        BeginDrawing();
      
        ClearBackground(deepSpaceBg); 
        
        pthread_mutex_lock(&sys_lock);
        
        DrawSpaceText("AWAITING TERMINAL OVERRIDE...", 30, 20, 18, warningRed);
        if (sim_active) {
            DrawSpaceText("AUTONOMOUS STRESS-TEST ACTIVE", 30, 85, 20, solarYellow);
        }
        
        char temp_seq[MAX_PROCESSES * 3];
        bool current_safe_state = is_safe(temp_seq);
        
        if (current_safe_state) {
            DrawSpaceText("SYSTEM STATUS: MISSION GO (SAFE)", 30, 50, 26, safeGreen);
        } else {
            DrawSpaceText("SYSTEM STATUS: ABORT / GRIDLOCK DETECTED", 30, 50, 26, warningRed);
        }

        DrawSpaceText("BASE ASSET INVENTORY:", 650, 20, 20, textBright);
        for(int i = 0; i < num_resources; i++) {
            DrawRectangleLines(650 + i * 75, 50, 65, 45, holoBlue);
            DrawSpaceText(TextFormat("%d", available[i]), 670 + i * 75, 63, 20, textBright);
            DrawSpaceText(asset_names[i], 650 + i * 75, 100, 14, textDim);
        }

        DrawMatrix(30, 120, "CURRENTLY DEPLOYED (ALLOC)", allocation, solarYellow, true); 
        DrawMatrix(620, 120, "MAXIMUM REQUIRED (MAX)", max, holoBlue, false);
        
        DrawMatrix(30, 440, "", need, warningRed, false);
        DrawCargoBelts(620, 440); 
        DrawSafeSequenceRunway(30, 770, current_safe_state ? temp_seq : "GRIDLOCK");
        pthread_mutex_unlock(&sys_lock);
        
        HandleMouseInteractivity(); 
        EndDrawing();
    }

    UnloadFont(spaceFont);
    CloseWindow();
    return 0;
}