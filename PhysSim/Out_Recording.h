#pragma once
#include "Connector.h"
#include "OUT.h"
using namespace std;
// file for open recording

class Out_Recording
{
private:
	string file_name;
	int fps;
	int frame;
	int width;
	int height;
	int circles_count;
public:
	Out_Recording(string _file_name, int _fps, int _width, int _height, int _frame, int _circles_count) {
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
			try {
				string frame_name = "frame" + to_string(tick);
				vector<f> pos_x = j[frame_name]["Objects"]["x"];
				vector<f> pos_y = j[frame_name]["Objects"]["y"];
				vector<f> radius = j[frame_name]["Objects"]["radius"];
				f Energy = j[frame_name]["Energy"];

				OUT framet( 60, 1, 1600, 1000 );
				framet.draw_frame(pos_x, pos_y, radius, Energy, tick, circles_count);
			}
			catch (const exception& e) {

			}
			tick++;
		}
		CloseWindow();
	}
};


