#include "raylib.h"
#include <string>
#include <unordered_map>
#include <cstdint> 
#include <vector>

using namespace std;

using f = float;
using ll = long long;

const int fps = 60;
const int frame = 10;
const int width = 1600;
const int height = 1000;

struct Cell {
    vector<int> indices;
    ll last_tick = -1;
};

f time_koof = 0.01;
ll tick = 1;
f k = 10;

ll circles_count = 1601;
ll obj_count = circles_count;
ll moved_obj_count = circles_count;
ll max_interactions = (obj_count * obj_count + obj_count)/2; // 1**2 + 2**2 + 3**2 + ... + k**2 = (k**2+k)/2 

f cell_size = 48;
unordered_map<int64_t, Cell> grid;

vector<f> pos_x(circles_count);
vector<f> pos_y(circles_count);
vector<f> vel_x(circles_count);
vector<f> vel_y(circles_count);
vector<f> accleration_x(circles_count);
vector<f> accleration_y(circles_count);
vector<f> radius(circles_count, 7);
vector<f> mass(circles_count, 1);

f dist_sq(f x1, f y1, f x2, f y2) {
    f dx = x1 - x2;
    f dy = y1 - y2;
    return dx * dx + dy * dy;
}

/*
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
}*/

/**/
// но для начала возьмём общие константы
const f EPSILON = 60.0;  // глубина потенциальной ямы
const f SIGMA = 16.0;  // характерный размер частицы
const f CUTOFF_RADIUS = 3.0 * SIGMA;

Vector2 f1(f x1, f y1, f x2, f y2, f r1, f r2) {
    f dx = x2 - x1;
    f dy = y2 - y1;
    r2 = dx * dx + dy * dy;

    // Отсечка по дистанции — за пределами cutoff сила пренебрежимо мала,
    // не тратим время на вычисление тяжёлых степеней
    if (r2 > CUTOFF_RADIUS * CUTOFF_RADIUS || r2 < 1e-12) {
        return { 0, 0 };
    }

    f r = sqrt(r2);

    // (sigma/r)^6 и (sigma/r)^12 — считаем через квадраты, дешевле, чем pow()
    f sr2 = (SIGMA * SIGMA) / r2;
    f sr6 = sr2 * sr2 * sr2;
    f sr12 = sr6 * sr6;

    // Модуль силы: F(r) = (24*eps/r) * (2*sr12 - sr6)
    f force_magnitude = (24.0 * EPSILON / r) * (2.0 * sr12 - sr6);

    // Направление — единичный вектор от частицы 1 к частице 2
    f nx = dx / r;
    f ny = dy / r;

    // Положительная force_magnitude при отталкивании должна толкать
    // частицу 1 ПРОТИВ направления к частице 2 — то есть с минусом
    return { -force_magnitude * nx, -force_magnitude * ny };
}


void insert_into_grid(int64_t key, int i) {
    Cell& cell = grid[key];              // создаст пустую Cell, если ключа ещё не было
    if (cell.last_tick != tick) { // клетка "устарела" с прошлого раза — она логически пуста
        cell.indices.clear();             // но физическая память вектора уже выделена — переиспользуем
        cell.last_tick = tick;
    }
    cell.indices.push_back(i);
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
        int cx = (int)(pos_x[i] / cell_size);
        int cy = (int)(pos_y[i] / cell_size);
        int64_t key = ((int64_t)cx << 32) | (uint32_t)cy;
        insert_into_grid(key, i);
    }

    for (int i = 0; i < circles_count; i++) {
        accleration_x[i] = 5 * k * (abs(pos_x[i] - radius[i]) - abs(pos_x[i] - width + radius[i]) + width - 2 * (pos_x[i] - radius[i]) - 2 * radius[i]) / 2;
        accleration_y[i] = 5 * k * (abs(pos_y[i] - radius[i]) - abs(pos_y[i] - height + radius[i]) + height - 2 * (pos_y[i] - radius[i]) - 2 * radius[i]) / 2;
        accleration_y[i] += -9.81 * time_koof * 0;
    }

    for (int i = 0; i < circles_count; i++) {
        int cx = (int)(pos_x[i] / cell_size);
        int cy = (int)(pos_y[i] / cell_size);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int64_t key = (((int64_t)(cx + dx)) << 32) | (uint32_t)(cy + dy);
                auto it = grid.find(key);
                if (it == grid.end() || it->second.last_tick != tick) continue;

                for (int j : it->second.indices) {
                    if (j <= i) continue;
                    Vector2 force = f1(pos_x[i], pos_y[i], pos_x[j], pos_y[j], radius[i], radius[j]);
                    accleration_x[i] += k * force.x / mass[i];
                    accleration_y[i] += k * force.y / mass[i];
                    accleration_x[j] += -k * force.x / mass[j];
                    accleration_y[j] += -k * force.y / mass[j];
                }
            }
        }
    }
}


// Verlet
inline void interpreter(f dt = time_koof) {
    //interpr_Phase_1(dt);
    interpr_Phase_2(dt);
    interpr_Phase_3(dt);
    interpr_Phase_1(dt);
    if (tick % 100 == 0) {
        printf("grid size (distinct keys ever seen): %zu\n", grid.size());
    }
    if (tick == 2000) {
        pos_x[1600] = 900;
        pos_y[1600] = 900;
        vel_y[1600] = -50;
        vel_x[1600] = -50;
        radius[1600] = 10;
        mass[1600] = 200;
    }
    tick++;
}



int main() {
    interpr_Phase_1(time_koof / 2);

    /*
    for (int i = 0; i < circles_count; i++) {
        pos_x[i] = rand() % (width - 20) + 10;
        pos_y[i] = rand() % (height - 20) + 10;
        vel_y[i] = 1;
        vel_x[i] = 1;
    }*/

    for (int i = 0; i < circles_count-1; i++) {
        pos_x[i] = 17 * (i % 40) + 20 + (rand() % 100) / 10000;
        pos_y[i] = 17 * (i / 40) + 20 + (rand() % 100) / 10000;
        vel_y[i] = 0;
        vel_x[i] = 0;
    }
    

    InitWindow(width, height, "Test");
    SetTargetFPS(fps); // Ограничиваем FPS (можно изменить или убрать для теста)


    while (!WindowShouldClose()) {
        f Energy = 0;
        for (int i = 0; i < circles_count; i++) {
            f Scalar_vel = vel_x[i] * vel_x[i] + vel_y[i] * vel_y[i];
            Energy += Scalar_vel * mass[i];
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