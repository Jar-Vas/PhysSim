#include "raylib.h"
#include <string>
#include <vector>

using namespace std;

using f = float;
using ll = long long;

const int fps = 60;
const int frame = 100;
const int width = 1600;
const int height = 1000;

f time_koof = 0.00001;
f k = 1000;

int circles_count = 50;

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


vector<float> energy_history;
const size_t MAX_POINTS = 800;

int tick = 1;

int main() {
    vector<f> pos_x(circles_count);
    vector<f> pos_y(circles_count);
    vector<f> vel_x(circles_count);
    vector<f> vel_y(circles_count);
    vector<f> radius(circles_count, 15);


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
        for (int h = 0; h < frame; h++) {
            for (int i = 0; i < circles_count; i++) {
                vel_x[i] += k * (abs(pos_x[i] - radius[i]) - abs(pos_x[i] - width + radius[i]) + width - 2 * (pos_x[i] - radius[i]) - 2 * radius[i]) / 2;
                vel_y[i] += k * (abs(pos_y[i] - radius[i]) - abs(pos_y[i] - height + radius[i]) + height - 2 * (pos_y[i] - radius[i]) - 2 * radius[i]) / 2;

                //vel_x[i] += -k * pos_x[i] + (k * abs(pos_x[i])) / 2 - (k * abs(pos_x[i] - width)) / 2 + k * (width) / 2;
                //vel_y[i] += -k * pos_y[i] + (k * abs(pos_y[i])) / 2 - (k * abs(pos_y[i] - height)) / 2 + k * (height) / 2;
                for (int j = 0; j < circles_count; j++) {
                    if (i == j) continue;
                    Vector2 force = f1(pos_x[i], pos_y[i], pos_x[j], pos_y[j], radius[i], radius[j]);
                    vel_x[i] += k * force.x * time_koof;
                    vel_y[i] += k * force.y * time_koof;
                }
                pos_x[i] += vel_x[i] * time_koof;
                pos_y[i] += vel_y[i] * time_koof;
                tick++;
            }
        }

        for (int i = 0; i < circles_count; i++) {
            DrawCircleV({ pos_x[i], pos_y[i] }, radius[i], BLACK);
        }
        //RenderGraphContent(500, 500);
        EndDrawing();
    }

    CloseWindow();
}