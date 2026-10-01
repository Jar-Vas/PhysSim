#pragma once
#include "Connector.h"
#include "OUT.h"
using namespace std;
// file for open recording
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

	inline void draw_frame(vector<f>& pos_x, vector<f>& pos_y, vector<f>& radius, f Energy, ll tick, int circles_count) {
		
	}
};

class Out_Recording
{
private:
	string file_name;
	int fps;
	int frame;
	int width;
	int height;
	ll circles_count;
public:
	Out_Recording(string _file_name, int _fps, int _width, int _height, int _frame, ll _circles_count) {
		file_name = _file_name;
		fps = _fps;
		frame = _frame;
		circles_count = _circles_count;
		width = _width;
		height = _height;
	}

	void Out() {
		ifstream file(file_name);
		json j;
		file >> j;
		ll tick = 1;
		bool opend = 1;
		InitWindow(width, height, "Test");
		SetTargetFPS(fps); // Ограничиваем FPS (можно изменить или убрать для теста)
		while (opend && !WindowShouldClose()) {
			BeginDrawing();
			DrawCircle(10, 10, 4, RED);
			try {
				string frame_name = "frame" + to_string(tick);
				vector<f> pos_x = j[frame_name]["Objects"]["x"];
				vector<f> pos_y = j[frame_name]["Objects"]["y"];
				vector<f> radius = j[frame_name]["Objects"]["radius"];
				f Energy = j[frame_name]["Energy"];

				string EnergyText = "Energy: " + to_string(Energy);
				DrawText(EnergyText.c_str(), 10, 70, 20, DARKGRAY);

				string TickText = "Tick: " + to_string(tick);
				DrawText(TickText.c_str(), 10, 100, 20, DARKGRAY);

				if (IsKeyPressed(KEY_ONE)) frame++;
				if (IsKeyPressed(KEY_TWO)) frame -= 1 * frame > 1;
				if (IsKeyPressed(KEY_THREE)) frame = 1;

				// Получаем точное значение FPS для кастомной отрисовки
				int currentFPS = GetFPS();

				ClearBackground(RAYWHITE);

				// 2. Кастомный счетчик (с вашим стилем и цветом)
				string fpsText = "FPS: " + to_string(currentFPS);
				DrawText(fpsText.c_str(), 10, 10, 20, DARKGRAY);


				string render_per_fps = "Render per fps: " + to_string(frame);
				DrawText(render_per_fps.c_str(), 10, 40, 20, DARKGRAY);

				for (int i = 0; i < circles_count; i++) {
					DrawCircle(pos_x[i], height - pos_y[i], radius[i], BLACK);
				}
			}
			catch (const exception& e) {
				EndDrawing();
				CloseWindow();
			}
			EndDrawing();
			tick++;
		}
		CloseWindow();
	}
};


