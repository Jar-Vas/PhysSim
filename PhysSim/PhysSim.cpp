#include "Connector.h"
using namespace std;

const int fps = 60;
int frame = 0;
const int width = 1600;
const int height = 1000;

f dt = 0.005;
ll tick = 1;
ll end_tick = -1;
f k = 1;

f Energy;

f MAX_FORCE = 1000;

ll circles_count = 750*3;

bool is_recording = false;
json Recording;
int frame_recorded_koof = 50;
string recorded_file_name = "Recording3";

struct Cell {
    vector<int> indices;
    ll last_tick = -1;
};

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
vector<int> dt_m(circles_count, 1);

vector<Vector3> px;

vector<ll> del_id;

inline f dist_sq(f x1, f y1, f x2, f y2) {
    f dx = x1 - x2;
    f dy = y1 - y2;
    return dx * dx + dy * dy;
}

void calc_dt_m(ll index) {
    dt_m[index] = vel_x[index] * vel_x[index] + vel_y[index] * vel_y[index];
}

void insert_into_grid(int64_t key, int i) {
    Cell& cell = grid[key];              // создаст пустую Cell, если ключа ещё не было
    if (cell.last_tick != tick) {        // клетка "устарела" с прошлого раза — она логически пуста
        cell.indices.clear();            // но физическая память вектора уже выделена — переиспользуем
        cell.last_tick = tick;
    }
    cell.indices.push_back(i);
}


void spawn(f x, f y, f vx, f vy, f r, f m) {
    circles_count++;
    pos_x.push_back(x);
    pos_y.push_back(y);
    vel_x.push_back(vx);
    vel_y.push_back(vy);
    accleration_x.push_back(0);
    accleration_y.push_back(0);
    radius.push_back(r);
    mass.push_back(m);
    int cx = (int)(x / cell_size);
    int cy = (int)(y / cell_size);
    int64_t key = (((int64_t)(cx)) << 32) | (uint32_t)(cy);
    insert_into_grid(key, circles_count - 1);
}

void pop(ll i) {
    int64_t key = (((int64_t)(pos_x[i])) << 32) | (uint32_t)(pos_y[i]);
    auto it = grid.find(key);
    insert_into_grid(key, circles_count-1);
    //circles_count--;
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
const f EPSILON = 5;  // глубина потенциальной ямы
const f SIGMA = 16.0;  // характерный размер частицы
const f CUTOFF_RADIUS = 3.0 * SIGMA;

Vector2 f1(f x1, f y1, f x2, f y2, f r1, f r2) {
    f dx = x2 - x1;
    f dy = y2 - y1;
    r2 = dx * dx + dy * dy;

    if (r2 > CUTOFF_RADIUS * CUTOFF_RADIUS || r2 < 1e-12) {
        return { 0, 0 };
    }

    f r = sqrt(r2);

    f sr2 = (SIGMA * SIGMA) / r2;
    f sr6 = sr2 * sr2 * sr2;
    f sr12 = sr6 * sr6;

    // Модуль силы: F(r) = (24*eps/r) * (2*sr12 - sr6)
    f force_magnitude = (24.0 * EPSILON / r) * (2.0 * sr12 - sr6);

    // Направление — единичный вектор от частицы 1 к частице 2
    f nx = dx / r;
    f ny = dy / r;

    //
    if (force_magnitude > min(force_magnitude, MAX_FORCE)) {
        cout << "Boom at " << x1 << ", " << y1 << ", force:" << force_magnitude
            << ", Tick " << tick << endl;
        f r = 1;
        if (force_magnitude - min(force_magnitude, MAX_FORCE) > 1000) r = 2;
        if (force_magnitude - min(force_magnitude, MAX_FORCE) > 5000) r = 3;
        if (force_magnitude - min(force_magnitude, MAX_FORCE) > 10000) r = 4;
        if (force_magnitude - min(force_magnitude, MAX_FORCE) > 100000) r = 5;

        px.push_back({ x1, y1, r });
    } 

    force_magnitude = min(force_magnitude, MAX_FORCE);
    force_magnitude = max(force_magnitude, -MAX_FORCE); // Для F < 0

    // Положительная force_magnitude при отталкивании должна толкать
    // частицу 1 ПРОТИВ направления к частице 2 — то есть с минусом
    return { -force_magnitude * nx, -force_magnitude * ny };
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
        accleration_x[i] = 0 * 5 * k * (abs(pos_x[i] - radius[i]) - abs(pos_x[i] - width + radius[i]) + width - 2 * (pos_x[i] - radius[i]) - 2 * radius[i]) / 2;
        accleration_y[i] = 0 * 5 * k * (abs(pos_y[i] - radius[i]) - abs(pos_y[i] - height + radius[i]) + height - 2 * (pos_y[i] - radius[i]) - 2 * radius[i]) / 2;
        accleration_y[i] += -9.81 * 0 * (!IsKeyDown(KEY_G) * 2 - 1);
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


void interpr_Phase_0() {
    for (int i = 0; i < circles_count; i++) {
        calc_dt_m(i);
    }
}

// Verlet
inline void interpreter() {
    //interpr_Phase_1(dt);
    if (tick % frame_recorded_koof == 0 && is_recording) {
        string frame_name = "Frame" + to_string(tick);
        Recording[frame_name]["Energy"] = Energy;
        Recording[frame_name]["Objects"]["x"] = pos_x;
        Recording[frame_name]["Objects"]["y"] = pos_y;
        Recording[frame_name]["Objects"]["radius"] = radius;
    }

    interpr_Phase_0();
    interpr_Phase_2(dt);
    interpr_Phase_3(dt);
    interpr_Phase_1(dt);

    if (tick == -1) {
        pos_x[circles_count - 1] = 500;
        pos_y[circles_count - 1] = 500;
        vel_y[circles_count - 1] = -50;
        vel_x[circles_count - 1] = -50;
        radius[circles_count - 1] = 10;
        mass[circles_count - 1] = 100;
    }
    tick++;
}

f mouse_x_temp_1;
f mouse_y_temp_1;
f mouse_x_temp_2;
f mouse_y_temp_2;
ll c_id;


int main() {
    interpr_Phase_1(dt / 2);

    /*
    for (int i = 0; i < circles_count; i++) {
        pos_x[i] = rand() % (width - 20) + 10;
        pos_y[i] = rand() % (height - 20) + 10;
        vel_y[i] = 1;
        vel_x[i] = 1;
    }

    for (int i = 0; i < circles_count; i++) {
        pos_x[i] = 17.5 * (i % 50) + 300 + (rand() % 100) / 10000;
        pos_y[i] = 17.5 * (i / 50) + 300 + (rand() % 100) / 10000;
        vel_y[i] = 0;
        vel_x[i] = 0;
    }*/
    
    f lr = 17.5;
    for (int i = 0; i < circles_count; i++) {
        pos_x[i] = lr * (i % 50) + lr / 2 * ((int)((i) / (50)) % 2) + 300;
        pos_y[i] = sqrt(3) * lr / 2 * (i / 50) + 300;
        vel_y[i] = 0;
        vel_x[i] = 0;
    }


    InitWindow(width, height, "Test");
    SetTargetFPS(fps); // Ограничиваем FPS (можно изменить или убрать для теста)


    bool temp_k = 0;
    while (!WindowShouldClose()) {

        px.clear();
        for (int h = 0; h < frame + temp_k; h++) {
            temp_k = 0;
            if (tick > end_tick && end_tick != -1) {
                break;
            }
            interpreter();
        }
        if (tick > end_tick && end_tick != -1) {
            break;
        }


        BeginDrawing();
        if (IsKeyDown(KEY_R)) {
            for (int i = 0; i <= width; i += cell_size) {
                for (int j = height; j >= -100; j -= cell_size) {
                    DrawRectangleLines(i, j, cell_size, cell_size, GRAY);
                }
            }
        }
        ClearBackground(RAYWHITE);


        for (int i = 0; i < circles_count; i++) {
            DrawCircle(pos_x[i], height - pos_y[i], radius[i], {0, (unsigned char)(int)min(255.0f, 3 * sqrt(vel_x[i] * vel_x[i] + vel_y[i] * vel_y[i])), 0, 255});
        }

        for (Vector3 i : px) {
            DrawCircle(i.x, height - i.y, i.z, RED);
        }
        // Получаем точное значение FPS для кастомной отрисовки
        int currentFPS = GetFPS();

        f Energy = 0;
        for (int i = 0; i < circles_count; i++) {
            f Scalar_vel = vel_x[i] * vel_x[i] + vel_y[i] * vel_y[i];
            Energy += Scalar_vel * mass[i];
        }
        Energy /= 2;



        if (IsKeyPressed(KEY_ONE)) frame++;
        if (IsKeyPressed(KEY_TWO)) frame -= 1 * frame > 0;
        if (IsKeyPressed(KEY_THREE)) frame = 1;
        if (IsKeyPressed(KEY_ZERO)) frame = 20;
        if (IsKeyPressed(KEY_FOUR) && frame == 0) temp_k = 1;
        if (IsKeyPressed(KEY_FIVE)) {
            dt /= 2;
            MAX_FORCE = 10 / dt;
        }
        if (IsKeyPressed(KEY_SIX)) {
            dt *= 2;
            MAX_FORCE = 10 / dt;
        }

        if (IsKeyDown(KEY_P)) {
            for (int i = 0; i < circles_count; i++) {
                vel_x[i] = 0;
                vel_y[i] = 0;
            }
        }

        if (IsKeyPressed(KEY_N)) {
            for (int i = 0; i < circles_count; i++) {
                vel_x[i] *= 1.1;
                vel_y[i] *= 1.1;
            }
        }
        if (IsKeyPressed(KEY_M)) {
            for (int i = 0; i < circles_count; i++) {
                vel_x[i] *= 0.9;
                vel_y[i] *= 0.9;
            }
        }

        f scale = 0.1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mouse_x_temp_1 = GetMouseX();
            mouse_y_temp_1 = height - GetMouseY();
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            spawn(mouse_x_temp_1, mouse_y_temp_1, scale * (GetMouseX() - mouse_x_temp_1), scale * ((height - GetMouseY()) - mouse_y_temp_1), 7, 1);
        }
        
        mouse_x_temp_2 = GetMouseX();
        mouse_y_temp_2 = height - GetMouseY();
        int mx = (int)(mouse_x_temp_2 / cell_size);
        int my = (int)(mouse_y_temp_2 / cell_size);
        ll min_dist_id = -1;
        f min_dist = -1;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int64_t key = (((int64_t)(mx + dx)) << 32) | (uint32_t)(my + dy);
                auto it = grid.find(key);

                if (it == grid.end()) continue;

                for (int j : it->second.indices) {
                    f new_dist = sqrt(dist_sq(mouse_x_temp_2, mouse_y_temp_2, pos_x[j], pos_y[j]));
                    if (new_dist < min_dist || min_dist == -1) {
                        min_dist_id = j;
                        min_dist = new_dist;
                    }
                }
            }
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (min_dist < radius[min_dist_id] && min_dist_id != -1) {
                c_id = min_dist_id;
            }
        }
        f acc_scale = 0.005;
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            vel_x[c_id] += acc_scale * (GetMouseX() - pos_x[c_id]);
            vel_y[c_id] += acc_scale * ((height - GetMouseY()) - pos_y[c_id]);
        }


        //RenderGraphContent(500, 500);

        string EnergyText = "Energy: " + to_string(Energy);
        DrawText(EnergyText.c_str(), 10, 70, 20, DARKGRAY);

        string TickText = "Tick: " + to_string(tick);
        DrawText(TickText.c_str(), 10, 100, 20, DARKGRAY);

        string fpsText = "FPS: " + to_string(currentFPS);
        DrawText(fpsText.c_str(), 10, 10, 20, DARKGRAY);

        string render_per_fps = "Render per fps: " + to_string(frame);
        DrawText(render_per_fps.c_str(), 10, 40, 20, DARKGRAY);

        string rendered_time = "Time rendered: " + to_string((double)tick * dt);
        DrawText(rendered_time.c_str(), 10, 130, 20, DARKGRAY);

        string dt_out = "dt: " + to_string(dt);
        DrawText(dt_out.c_str(), 10, 160, 20, DARKGRAY);

        if (min_dist < 1.5*radius[min_dist_id] && min_dist_id != -1) {
            int r_st_x = pos_x[min_dist_id] + 10 - 220 * (pos_x[min_dist_id] > width - 210);
            int r_st_y = pos_y[min_dist_id] + 10 + 44 * (pos_y[min_dist_id] < 70);

            DrawRectangle(r_st_x, height - (r_st_y), 200, 60, LIGHTGRAY);
            DrawRectangleLines(r_st_x, height - (r_st_y), 200, 60, DARKGRAY);
            string position = "Position: (" + to_string((int)pos_x[min_dist_id]) + ", " + to_string((int)pos_y[min_dist_id]) + ")";
            DrawText(position.c_str(), r_st_x + 2, height - (r_st_y - 2), 10, DARKGRAY);
            string velocity = "Velocity: (" + to_string(vel_x[min_dist_id]) + ", " + to_string(vel_y[min_dist_id]) + ")";
            DrawText(velocity.c_str(), r_st_x + 2, height - (r_st_y - 13), 10, DARKGRAY);
            string accleration = "Accleration: (" + to_string(accleration_x[min_dist_id]) + ", " + to_string(accleration_y[min_dist_id]) + ")";
            DrawText(accleration.c_str(), r_st_x + 2, height - (r_st_y - 26), 10, DARKGRAY);
            string radius_out = "Radius: " + to_string(radius[min_dist_id]);
            DrawText(radius_out.c_str(), r_st_x + 2, height - (r_st_y - 38), 10, DARKGRAY);
            string mass_out = "Mass: " + to_string(mass[min_dist_id]);
            DrawText(mass_out.c_str(), r_st_x + 2, height - (r_st_y - 49), 10, DARKGRAY);
            
            DrawLine(pos_x[min_dist_id], height - pos_y[min_dist_id], pos_x[min_dist_id] + vel_x[min_dist_id], height - (pos_y[min_dist_id] + vel_y[min_dist_id]), RED);
            DrawLine(pos_x[min_dist_id], height - pos_y[min_dist_id], pos_x[min_dist_id] + accleration_x[min_dist_id], height - (pos_y[min_dist_id] + accleration_y[min_dist_id]), BLUE);
        }
        EndDrawing();
    }

    CloseWindow();
    if (is_recording) {
        ofstream file(recorded_file_name);
        file << Recording.dump(4);
        file.close();
    }

    
    //Out_Recording video{ "Recording2", 60, width, height, frame, circles_count };
    //video.Out();


}