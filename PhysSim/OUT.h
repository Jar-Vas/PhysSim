#pragma once
#include "Connector.h"

using namespace std;
// file for open recording
int gf = 12;


class OUT
{
private:
    int fps;
    int frame;
    int width;
    int height;
public:
    OUT(int _fps, int _frame, int _width, int _height) {
        fps = _fps;
        frame = _frame;
        width = _width;
        height = _height;
    }

    void draw_frame(vector<f> &pos_x, vector<f>& pos_y, vector<f>& radius, f Energy, ll tick, int circles_count) {
        string EnergyText = "Energy: " + to_string(Energy);
        DrawText(EnergyText.c_str(), 10, 70, 20, DARKGRAY);

        string TickText = "Tick: " + to_string(tick);
        DrawText(TickText.c_str(), 10, 100, 20, DARKGRAY);

        if (IsKeyPressed(KEY_ONE)) frame++;
        if (IsKeyPressed(KEY_TWO)) frame -= 1 * frame > 1;
        if (IsKeyPressed(KEY_THREE)) frame = 1;

        // Получаем точное значение FPS для кастомной отрисовки
        int currentFPS = GetFPS();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 2. Кастомный счетчик (с вашим стилем и цветом)
        string fpsText = "FPS: " + to_string(currentFPS);
        DrawText(fpsText.c_str(), 10, 10, 20, DARKGRAY);


        string render_per_fps = "Render per fps: " + to_string(frame);
        DrawText(render_per_fps.c_str(), 10, 40, 20, DARKGRAY);

        for (int i = 0; i < circles_count; i++) {
            DrawCircle(pos_x[i], height - pos_y[i], radius[i], BLACK);
        }
        EndDrawing();
    }
};

