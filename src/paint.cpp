#include <utility>
#include <cstdio>
#include <array>
#include <png.h>
#include "olcPixelGameEngine.h"
#include "colorMenu.h"

namespace paint {


	class Paint : public olc::PixelGameEngine {
	private:
		static constexpr int max_scale = 50;
		std::string filename{};
		std::unique_ptr<olc::Sprite> surface{};
		std::unique_ptr<olc::Decal> decal{};
		int scale{1};
		float posx, posy;
		int invert_move = 1;
		olc::vi2d last_mouse;
		float text_counter{};
		std::string text{};
		olc::Pixel text_color{};

		ColorMenu<24> colorMenu{};
	public:
		Paint() : filename() {}
		Paint(const char* filename) : filename(filename) {}

		bool OnUserCreate() noexcept override {
			if (filename.empty()) surface = std::make_unique<olc::Sprite>(640, 480);
			else surface = std::make_unique<olc::Sprite>(filename);

			sAppName = "olcPaint";

			posx = (ScreenWidth() - surface->width) / 2;
			posy = (ScreenHeight() - surface->height) / 2;

			colorMenu[ 0] = olc::RED;
			colorMenu[ 1] = olc::DARK_RED;
			colorMenu[ 2] = olc::VERY_DARK_RED;
			colorMenu[ 3] = olc::GREEN;
			colorMenu[ 4] = olc::DARK_GREEN;
			colorMenu[ 5] = olc::VERY_DARK_GREEN;
			colorMenu[ 6] = olc::BLUE;
			colorMenu[ 7] = olc::DARK_BLUE;
			colorMenu[ 8] = olc::VERY_DARK_BLUE;
			colorMenu[ 9] = olc::GREY;
			colorMenu[10] = olc::DARK_GREY;
			colorMenu[11] = olc::VERY_DARK_GREY;
			colorMenu[12] = olc::CYAN;
			colorMenu[13] = olc::DARK_CYAN;
			colorMenu[14] = olc::VERY_DARK_CYAN;
			colorMenu[15] = olc::YELLOW;
			colorMenu[16] = olc::DARK_YELLOW;
			colorMenu[17] = olc::VERY_DARK_YELLOW;
			colorMenu[18] = olc::MAGENTA;
			colorMenu[19] = olc::DARK_MAGENTA;
			colorMenu[20] = olc::VERY_DARK_MAGENTA;
			colorMenu[21] = olc::WHITE;
			colorMenu[22] = olc::BLACK;
			colorMenu[23] = olc::BLANK;

			colorMenu.pos = { 100, 100 };
			colorMenu.boxSize = 20;
			colorMenu.colorsPerRow = 3;
			colorMenu.update(*this, 0);

			updateDecal();

			return true;
		}

		void updateDecal() noexcept {
			decal = std::make_unique<olc::Decal>(surface.get());
		}
		bool OnUserUpdate(float delta) noexcept override {
			float speed = 100.0f * invert_move;
			const auto imagePos = [this]() {
				return olc::vi2d{int(posx - ((scale - 1) * surface->width / 2)), int(posy - ((scale - 1) * surface->height / 2))};
			};
			const auto in_image = [this, imagePos](int x, int y) {
				const auto pos = imagePos();
				return x >= int(pos.x) && y >= int(pos.y)
				&& x < (int(pos.x) + surface->width * scale)
				&& y < (int(pos.y) + surface->height * scale);
			};
			if (GetKey(olc::Key::SHIFT).bHeld && !GetMouseWheel()) speed *= 10.0f;
			else if (GetKey(olc::Key::ALT).bHeld) speed /= 5.0f;
			if (GetKey(olc::Key::UP).bHeld) posy -= delta * speed;
			if (GetKey(olc::Key::DOWN).bHeld) posy += delta * speed;
			if (GetKey(olc::Key::LEFT).bHeld) posx -= delta * speed;
			if (GetKey(olc::Key::RIGHT).bHeld) posx += delta * speed;
			if (GetKey(olc::Key::PLUS).bPressed) ++scale;
			else if (GetKey(olc::Key::MINUS).bPressed) --scale;
			if (GetKey(olc::Key::F2).bPressed) invert_move = -invert_move;

			if (GetKey(olc::Key::CTRL).bHeld && GetKey(olc::Key::S).bPressed) {
				// SAVE ME
				const bool r = saveImage(*surface);
				if (r) {
					text = "Saved.";
					text_color = olc::DARK_GREY;
				}
				else {
					text = "Failed to save!";
					text_color = olc::RED;
				}
				text_counter = 3;
			}

			if (const int m = GetMouseWheel(); m) {
				if (GetKey(olc::Key::SHIFT).bHeld) posx += m * delta * speed;
				else if (GetKey(olc::Key::CTRL).bHeld) {
					if (m > 0) ++scale;
					else --scale;
				}
				else posy += m * delta * speed;
			}

			if (scale <= 0) scale = 1;
			else if (scale > max_scale) scale = max_scale;
			if (GetMouse(0).bPressed || GetMouse(1).bPressed) {
				const int x = GetMouseX();
				const int y = GetMouseY();
				const std::size_t ci = colorMenu.getColorIndexAtPos({ x, y });
				if (ci != -1) {
					olc::Pixel color = colorMenu.getColor(ci);
					if (GetMouse(0).bPressed) colorMenu.fgColor = color;
					else colorMenu.bgColor = color;
					colorMenu.update(*this, 0);
				}
			}
			if (GetMouse(2).bPressed) {
				const int x = GetMouseX();
				const int y = GetMouseY();
				const auto pos = imagePos();
				if (in_image(x, y)) {
					const int sx = (x - int(pos.x)) / scale;
					const int sy = (y - int(pos.y)) / scale;
					colorMenu.fgColor = surface->GetPixel(sx, sy);
					colorMenu.update(*this, delta);
				}
			}
			if (GetMouse(0).bHeld || GetMouse(1).bHeld) {
				const int x = GetMouseX();
				const int y = GetMouseY();
				if (last_mouse.x >= colorMenu.pos.x && last_mouse.y >= colorMenu.pos.y
					&& last_mouse.x < (colorMenu.pos.x + colorMenu.getWidth())
					&& last_mouse.y < (colorMenu.pos.y + colorMenu.upperPadding)) {
					colorMenu.pos.x = std::clamp(x - (last_mouse.x - colorMenu.pos.x), 0, (ScreenWidth() - colorMenu.getWidth()));
					colorMenu.pos.y = std::clamp(y - (last_mouse.y - colorMenu.pos.y), 0, (ScreenHeight() - colorMenu.getHeight()));
				}
				else if (in_image(x, y) && in_image(last_mouse.x, last_mouse.y)) {
				const auto pos = imagePos();
					const olc::Pixel color = GetMouse(0).bHeld ? colorMenu.fgColor : colorMenu.bgColor;
					const int sx = (x - int(pos.x)) / scale;
					const int sy = (y - int(pos.y)) / scale;
					const int lx = (last_mouse.x - int(pos.x)) / scale;
					const int ly = (last_mouse.y - int(pos.y)) / scale;
					SetDrawTarget(surface.get());
					//surface->SetPixel(sx, sy, selectedForeground->color);
					DrawLine(lx, ly, sx, sy, color);
					SetDrawTarget(nullptr);
					updateDecal();
				}
			}

			last_mouse = GetMousePos();

			draw:
			Clear(olc::Pixel(200, 255, 255));

			// Draw Image
			DrawRect(imagePos().x - 1, imagePos().y - 1, (surface->width * scale) + 1 , (surface->height * scale) + 1, olc::VERY_DARK_GREY);

			SetPixelMode(olc::Pixel::MASK);
			//DrawSprite(imagePos(), surface.get(), uint32_t(scale));
			DrawDecal(imagePos(), decal.get(), { scale, scale });
			SetPixelMode(olc::Pixel::NORMAL);

			SetPixelMode(olc::Pixel::MASK);
			DrawDecal(colorMenu.pos, colorMenu.getDecal().get());
			SetPixelMode(olc::Pixel::NORMAL);

			if (text_counter != 0.0f) {
				if (text_counter < 0.0f) text_counter = 0.0f;
				else {
					text_counter -= delta;
					DrawString(ScreenWidth() / 2 - GetTextSize(text).x * 2, 20, text, text_color, 5);
				}
			}

			return true;
		}

		bool saveImage(const olc::Sprite& spr) {
			FILE* file = std::fopen(filename.c_str(), "wb");
			png_structp png = nullptr;
			png_infop info = nullptr;
			png_bytep* rows = nullptr;

			if (!surface->GetData() || !file) return false;

			png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			if (!png) return false;

			info = png_create_info_struct(png);
			if (!info) return false;

			png_set_IHDR(
				png,
				info,
				surface->width,
				surface->height,
				8,
				PNG_COLOR_TYPE_RGB_ALPHA,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_BASE,
				PNG_FILTER_TYPE_BASE
			);
			rows = new png_bytep[surface->height];
			for (int y = 0; y < surface->height; ++y) {
				rows[y] = new png_byte[surface->width * sizeof(olc::Pixel)];
				for (int x = 0; x < surface->width; ++x) {
					const auto px = spr.GetPixel(x, y);
					rows[y][x * 4 + 0] = px.r;
					rows[y][x * 4 + 1] = px.g;
					rows[y][x * 4 + 2] = px.b;
					rows[y][x * 4 + 3] = px.a;
				}
			}
			png_init_io(png, file);
			png_set_rows(png, info, rows);
			png_write_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);

			for (int y = 0; y < surface->height; ++y)
				delete[] rows[y];
			delete[] rows;

			png_destroy_write_struct(&png, &info);

			fclose(file);

			return true;
		}
	};
}

int main(const int argc, const char** argv) {
	const char* filename;
	int w = 1280, h = 720, scale = 1;
	if (argc == 5) {
		w = std::atoi(argv[2]);
		h = std::atoi(argv[3]);
		scale = std::atoi(argv[4]);
	}
	else if (argc == 4) {
		w = std::atoi(argv[2]);
		h = std::atoi(argv[3]);
	}
	else if (argc != 2) {
		std::printf("Usage: %s <image.png> [<width> <height>] [scale]\n", *argv);
		return 1;
	}
	paint::Paint paint{argv[1]};
	if (paint.Construct(w, h, scale, scale))
		paint.Start();
	return 0;
}
