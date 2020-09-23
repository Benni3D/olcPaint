#define OLC_PGE_APPLICATION
#include <cstdio>
#include "olcPixelGameEngine.h"

struct ColorBox {
	static constexpr int size = 20;
	static constexpr int offx = 20;
	static constexpr int offy = 50;
	olc::vi2d pos;
	olc::Pixel color;

	constexpr bool contains(olc::vi2d other) const noexcept {
		return other.x >= pos.x && other.y >= pos.y && other.x < (pos.x + size) && other.y < (pos.y + size);
	}
};

class Paint : public olc::PixelGameEngine {
private:
	std::string filename{};
	std::unique_ptr<olc::Sprite> surface{};
	int scale{1};
	float posx, posy;
	std::array<ColorBox, 16> colors{};
	olc::Pixel selectedForeground{};
	olc::Pixel selectedBackground{};
	int invert_move = 1;
	olc::vi2d last_mouse;
public:
	Paint() : filename() {}
	Paint(const char* filename) : filename(filename) {}

	bool OnUserCreate() noexcept override {
		if (filename.empty()) surface = std::make_unique<olc::Sprite>(640, 480);
		else surface = std::make_unique<olc::Sprite>(filename);

		posx = (ScreenWidth() - surface->width) / 2;
		posy = (ScreenHeight() - surface->height) / 2;

		for (std::size_t i = 0; i < colors.size(); ++i) {
			const int x = (i & 1) == 0 ? ColorBox::offx : (ColorBox::offx + ColorBox::size + 5);
			const int y = (i >> 1) * (ColorBox::size + 5) + ColorBox::offy;
			colors[i].pos = { x + 5, y + 5 };
		}
		colors[ 0].color = olc::RED;
		colors[ 1].color = olc::DARK_RED;
		colors[ 2].color = olc::GREEN;
		colors[ 3].color = olc::DARK_GREEN;
		colors[ 4].color = olc::BLUE;
		colors[ 5].color = olc::DARK_BLUE;
		colors[ 6].color = olc::GREY;
		colors[ 7].color = olc::DARK_GREY;
		colors[ 8].color = olc::CYAN;
		colors[ 9].color = olc::DARK_CYAN;
		colors[10].color = olc::YELLOW;
		colors[11].color = olc::DARK_YELLOW;
		colors[12].color = olc::MAGENTA;
		colors[13].color = olc::DARK_MAGENTA;
		colors[14].color = olc::WHITE;
		colors[15].color = olc::BLACK;

		selectedForeground = olc::WHITE;
		selectedBackground = olc::BLACK;

		return true;
	}

	bool OnUserUpdate(float delta) noexcept override {
		float speed = 100.0f * invert_move;
		const auto imagePos = [this]() {
			return olc::vi2d{int(posx - ((scale - 1) * surface->width / 2)), int(posy - ((scale - 1) * surface->height / 2))};
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
			std::puts("Saving is not implemented!");
		}

		if (const int m = GetMouseWheel(); m) {
			if (GetKey(olc::Key::SHIFT).bHeld) posx += m * delta * speed / scale;
			else if (GetKey(olc::Key::CTRL).bHeld) {
				if (m > 0) ++scale;
				else --scale;
			}
			else posy += m * delta * speed / scale;
		}

		if (scale <= 0) scale = 1;
		else if (scale > 10) scale = 10;

		if (GetMouse(0).bPressed || GetMouse(1).bPressed) {
			const int x = GetMouseX();
			const int y = GetMouseY();
			if (x >= ColorBox::offx + 5 && y >= ColorBox::offy + 5
				&& x < (ColorBox::offx + 2 * ColorBox::size + 10)
				&& y < (ColorBox::offy + (colors.size() / 2 + 1) * ColorBox::size + 10)) {
				olc::Pixel* const color = GetMouse(0).bPressed ? &selectedForeground : &selectedBackground;
				for (std::size_t i = 0; i < colors.size(); ++i) {
					if (colors[i].contains({ x, y })) {
						*color = colors[i].color;
						goto draw;
					}
				}
			}
		}
		if (GetMouse(0).bHeld || GetMouse(1).bHeld) {
			const int x = GetMouseX();
			const int y = GetMouseY();
			const auto pos = imagePos();
			const auto in_image = [this, pos](int x, int y) {
				return x >= int(pos.x) && y >= int(pos.y)
				&& x < (int(pos.x) + surface->width * scale)
				&& y < (int(pos.y) + surface->height * scale);
			};
			if (in_image(x, y) && in_image(last_mouse.x, last_mouse.y)) {
				const olc::Pixel color = GetMouse(0).bHeld ? selectedForeground : selectedBackground;
				const int sx = (x - int(pos.x)) / scale;
				const int sy = (y - int(pos.y)) / scale;
				const int lx = (last_mouse.x - int(pos.x)) / scale;
				const int ly = (last_mouse.y - int(pos.y)) / scale;
				SetDrawTarget(surface.get());
				//surface->SetPixel(sx, sy, selectedForeground->color);
				DrawLine(lx, ly, sx, sy, color);
				SetDrawTarget(nullptr);
			}
		}

		last_mouse = GetMousePos();

		draw:
		Clear(olc::Pixel(200, 255, 255));

		// Draw Image
		DrawSprite(imagePos(), surface.get(), uint32_t(scale));

		// Draw UI
		FillRect(ColorBox::offx, ColorBox::offy, ColorBox::size * 2 + 15, (ColorBox::size + 5) * colors.size() / 2 + 5, olc::GREY);
		for (std::size_t i = 0; i < colors.size(); ++i) {
			FillRect(colors[i].pos, { ColorBox::size, ColorBox::size }, colors[i].color);
			DrawRect(colors[i].pos, { ColorBox::size, ColorBox::size });
		}
		FillRect(ColorBox::offx, ColorBox::offy + (colors.size() / 2 + 3) * ColorBox::size - 5, ColorBox::size * 2 + 15, ColorBox::size + 10, olc::GREY);
		FillRect(ColorBox::offx + 5, ColorBox::offy + (colors.size() / 2 + 3) * ColorBox::size, ColorBox::size, ColorBox::size, selectedForeground);
		DrawRect(ColorBox::offx + 5, ColorBox::offy + (colors.size() / 2 + 3) * ColorBox::size, ColorBox::size, ColorBox::size, olc::WHITE);
		FillRect(ColorBox::offx + ColorBox::size + 10, ColorBox::offy + (colors.size() / 2 + 3) * ColorBox::size, ColorBox::size, ColorBox::size, selectedBackground);
		DrawRect(ColorBox::offx + ColorBox::size + 10, ColorBox::offy + (colors.size() / 2 + 3) * ColorBox::size, ColorBox::size, ColorBox::size, olc::WHITE);

		return true;
	}
};

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
	Paint paint{argv[1]};
	if (paint.Construct(w, h, scale, scale))
		paint.Start();
	return 0;
}
