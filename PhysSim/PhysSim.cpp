#include "raylib.h"
#include <string>
#include <unordered_map>
#include <cstdint> 
#include <vector>

using namespace std;

using f = float;
using ll = long long;

const int fps = 60;
const int frame = 30;
const int width = 1600;
const int height = 1000;

f time_koof = 0.01;
ll tick = 1;
f k = 1;

ll circles_count = 100;
ll obj_count = circles_count;
ll moved_obj_count = circles_count;
ll max_interactions = (obj_count * obj_count + obj_count)/2; // 1**2 + 2**2 + 3**2 + ... + k**2 = (k**2+k)/2 

f cell_size = 20;

vector<f> pos_x(circles_count);
vector<f> pos_y(circles_count);
vector<f> vel_x(circles_count);
vector<f> vel_y(circles_count);
vector<f> accleration_x(circles_count);
vector<f> accleration_y(circles_count);
vector<f> radius(circles_count, 15);

f dist_sq(f x1, f y1, f x2, f y2) {
    f dx = x1 - x2;
    f dy = y1 - y2;
    return dx * dx + dy * dy;
}

Vector2 f1(f x1, f y1, f x2, f y2, f r1, f r2) {
    f dx = x1 - x2;
    f dy = y1 - y2;
    f dist = sqrt(dx * dx + dy * dy);
    f min_dist = r1 + r2;

    if (dist >= min_dist || dist == 0) {
        return { 0.0f, 0.0f };
    }

    f overlap = min_dist - dist;

    overlap *= 10;

    return { (dx / dist) * overlap, (dy / dist) * overlap };
}

void interpr_Phase_1(f dt) {
    for (int i = 0; i < circles_count; i++) {
        vel_x[i] += 1 * accleration_x[i] * dt;
        vel_y[i] += 1 * accleration_y[i] * dt;
    }
}

void interpr_Phase_2(f dt) {
    for (int i = 0; i < circles_count; i++) {
        pos_x[i] += vel_x[i] * dt;
        pos_y[i] += vel_y[i] * dt;
    }
}

void interpr_Phase_3(f dt) {
    for (int i = 0; i < circles_count; i++) {
        accleration_x[i] = k * (abs(pos_x[i] - radius[i]) - abs(pos_x[i] - width + radius[i]) + width - 2 * (pos_x[i] - radius[i]) - 2 * radius[i]) / 2;
        accleration_y[i] = k * (abs(pos_y[i] - radius[i]) - abs(pos_y[i] - height + radius[i]) + height - 2 * (pos_y[i] - radius[i]) - 2 * radius[i]) / 2;
    }

    for (int i = 0; i < circles_count; i++) {
        for (int j = i + 1; j < circles_count; j++) {
            Vector2 force = f1(pos_x[i], pos_y[i], pos_x[j], pos_y[j], radius[i], radius[j]);
            accleration_x[i] += k * force.x;
            accleration_y[i] += k * force.y;
            accleration_x[j] += -k * force.x;
            accleration_y[j] += -k * force.y;
        }
    }
}


// Verlet
inline void interpreter(f dt = time_koof) {
    //interpr_Phase_1(dt);
    interpr_Phase_2(dt);
    interpr_Phase_3(dt);
    interpr_Phase_1(dt);
    tick++;
}




vector<float> energy_history;
const size_t MAX_POINTS = 800;


int main() {
    interpr_Phase_1(time_koof / 2);

    for (int i = 0; i < circles_count; i++) {
        pos_x[i] = rand() % (width - 20) + 10;
        pos_y[i] = rand() % (height - 20) + 10;
        vel_y[i] = 6;
        vel_x[i] = 6;
    }

    InitWindow(width, height, "Test");
    SetTargetFPS(fps); // Ограничиваем FPS (можно изменить или убрать для теста)

    Vector2 circlePos = { 400.0f, 300.0f };

    //RenderTexture2D graphTarget = LoadRenderTexture(500, 500);

    //InitWindow(500, 500, "Energy Graph");
    //SetWindowPosition(1050, 100);


    while (!WindowShouldClose()) {
        f Energy = 0;
        for (int i = 0; i < circles_count; i++) {
            f Scalar_vel = vel_x[i] * vel_x[i] + vel_y[i] * vel_y[i];
            Energy += Scalar_vel;
        }
        Energy /= 2;
        string EnergyText = "Energy: " + to_string(Energy);
        DrawText(EnergyText.c_str(), 10, 40, 20, DARKGRAY);


        string TickText = "Tick: " + to_string(tick);
        DrawText(TickText.c_str(), 10, 70, 20, DARKGRAY);

        /*Hello world
        // Логика
        if (IsKeyDown(KEY_RIGHT)) circlePos.x += 4.0f;
        if (IsKeyDown(KEY_LEFT))  circlePos.x -= 4.0f;
        */

        unordered_map<int64_t, vector<int>> grid;

        for (int i = 0; i < circles_count; i++) {
            int cx = (int)(pos_x[i] / cell_size);
            int cy = (int)(pos_y[i] / cell_size);
            int64_t key = ((int64_t)cx << 32) | (uint32_t)cy;  // упаковать 2 int в один ключ
            grid[key].push_back(i);
        }

        for (int h = 0; h < frame; h++) {
            interpreter();
        }


        // Получаем точное значение FPS для кастомной отрисовки
        int currentFPS = GetFPS();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        // 1. Встроенный счетчик Raylib (готовый зелёный текст в углу)
        //DrawFPS(10, 10);

        // 2. Кастомный счетчик (с вашим стилем и цветом)
        std::string fpsText = "FPS: " + std::to_string(currentFPS);
        DrawText(fpsText.c_str(), 10, 10, 20, DARKGRAY);

        // Рисуем объект
        //DrawCircleV(circlePos, 40, MAROON);
        

        for (int i = 0; i < circles_count; i++) {
            DrawCircle(pos_x[i], height-pos_y[i], radius[i], BLACK);
        }

        //RenderGraphContent(500, 500);

        EndDrawing();
    }

    CloseWindow();
}