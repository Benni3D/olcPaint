#define OLC_PGE_APPLICATION
#include <utility>
#include <cstdio>
#include <array>
#include "olcPixelGameEngine.h"

template<std::size_t N>
class ColorMenu {
private:
	std::array<olc::Pixel, N> colors{};
	std::shared_ptr<olc::Sprite> sprite{};
	std::shared_ptr<olc::Decal> decal{};
public:
	olc::Pixel fgColor{ olc::WHITE }, bgColor{ olc::BLACK };
	olc::vi2d pos;
	int colorsPerRow = 2;
	int boxSize = 16;
	int padding = 5;
	int padding2 = 15;
	int upperPadding = 10;
	olc::Pixel menuBackground = olc::GREY;
	olc::Pixel border = olc::WHITE;

	constexpr ColorMenu(olc::vi2d pos = {}) noexcept : pos(pos) {}
	ColorMenu(const std::array<olc::Pixel, N>& a, olc::vi2d pos = {}) noexcept
		: colors(a), pos(pos) {
		update();
	}

	void setColor(std::size_t i, olc::Pixel c) noexcept {
		if (i < colors.size()) colors[i] = c;
	}
	[[nodiscard]]
	auto getColor(std::size_t i) const noexcept {
		return i < colors.size() ? colors[i] : olc::BLANK;
	}

	void update() noexcept {
		sprite = draw();
		decal = std::make_shared<olc::Decal>(sprite.get());
	}

	[[nodiscard]]
	auto getDecal() const noexcept { return decal; }

	[[nodiscard]]
	const olc::Pixel& operator[](std::size_t i) const noexcept {
		return colors[i];
	}
	[[nodiscard]]
	olc::Pixel& operator[](std::size_t i) noexcept {
		return colors[i];
	}

	[[nodiscard]]
	constexpr bool contains(olc::vi2d p) const noexcept {
		return (p.x >= pos.x + padding)
			&& p.y >= (pos.y + padding)
			&& p.x < (pos.x + getWidth() - padding)
			&& p.y < (pos.y + getHeight() - padding);
	}
	[[nodiscard]]
	constexpr std::size_t getColorIndexAtPos(olc::vi2d p) const noexcept {
		const auto isSelected = [p, this](std::size_t ci) {
			const auto p2 = getBoxPos(ci) + pos;
			return p.x >= p2.x && p.y >= p2.y && p.x < (p2.x + boxSize) && p.y < (p2.y + boxSize);
		};

		for (std::size_t i = 0; i < colors.size(); ++i) {
			if (isSelected(i)) return i;
		}

		return -1;
	}
	[[nodiscard]]
	constexpr int getWidth() const noexcept {
		return ((colorsPerRow + 1) * padding) + (colorsPerRow * boxSize);
	}
	[[nodiscard]]
	constexpr int getHeight() const noexcept {
		const int rows = (colors.size() + colorsPerRow - 1) / colorsPerRow;
		return (rows * padding) + (rows * boxSize) + upperPadding;
	}
	[[nodiscard]]
	constexpr int getFullHeight() const noexcept {
		return getHeight() + padding2 + 2 * padding + boxSize;
	}

private:
	olc::vi2d getBoxPos(std::size_t ci) const noexcept {
		const int posx = (ci % colorsPerRow) * boxSize + (ci % colorsPerRow + 1) * padding;
		const int posy = (ci / colorsPerRow) * boxSize + (ci / colorsPerRow) * padding + upperPadding;
		return { posx, posy };
	}
	[[nodiscard]]
	auto draw() const noexcept {
		const olc::Pixel background = menuBackground;
		const int rows = (colors.size() + colorsPerRow - 1) / colorsPerRow;
		const int width = getWidth();
		const int height = getHeight();
		const int width_sel = (3 * padding) + (2 * boxSize);
		const int height_sel = (2 * padding) + boxSize;
		auto sprite = std::make_shared<olc::Sprite>(width, height + height_sel + padding2);

		const auto fillRect = [sprite](int x, int y, int w, int h, olc::Pixel color) {
			for (int y2 = 0; y2 < h; ++y2) {
				for (int x2 = 0; x2 < w; ++x2) {
					sprite->SetPixel(x2 + x, y2 + y, color);
				}
			}
		};
 		const auto drawBox = [&sprite, this](int x, int y, olc::Pixel color){
			for (int i = 0; i < boxSize; ++i) {
				sprite->SetPixel(i + x, y, border);
				sprite->SetPixel(i + x, y + boxSize, border);
				sprite->SetPixel(x, i + y, border);
				sprite->SetPixel(x + boxSize, i + y, border);
			}
			for (int y2 = 1; y2 < (boxSize - 1); ++y2) {
				for (int x2 = 1; x2 < (boxSize - 1); ++x2) {
					sprite->SetPixel(x2 + x, y2 + y, color);
				}
			}
		};

		// draw background
		fillRect(0, 0, width, height, background);
		fillRect(0, height, width, padding2, olc::BLANK);
		fillRect(0, height + padding2, width_sel, height_sel, background);
		fillRect(width_sel, height + padding2, width - width_sel, height_sel, olc::BLANK);

		// draw boxes for main color selector
		for (int y = 0; y < rows; ++y) {
			for (int x = 0; x < colorsPerRow; ++x) {
				if ((y * colorsPerRow + x) >= colors.size()) goto L1;
				const int posx = (x + 1) * padding + (x * boxSize);
				const int posy = y * padding + (y * boxSize) + upperPadding;
				drawBox(posx, posy, colors[y * colorsPerRow + x]);
			}
		}
		L1:;

		// draw boxes for selected colors view
		drawBox(padding, height + padding2 + padding, fgColor);
		drawBox(2 * padding + boxSize, height + padding2 + padding, bgColor);

		return sprite;
	}

};

class Paint : public olc::PixelGameEngine {
private:
	static constexpr int max_scale = 50;
	std::string filename{};
	std::unique_ptr<olc::Sprite> surface{};
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
		colorMenu.update();

		return true;
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
			if (GetKey(olc::Key::SHIFT).bHeld) posx += m * delta * speed / scale;
			else if (GetKey(olc::Key::CTRL).bHeld) {
				if (m > 0) ++scale;
				else --scale;
			}
			else posy += m * delta * speed / scale;
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
				colorMenu.update();
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
				colorMenu.update();
			}
		}
		if (GetMouse(0).bHeld || GetMouse(1).bHeld) {
			const int x = GetMouseX();
			const int y = GetMouseY();
			if (last_mouse.x >= colorMenu.pos.x && last_mouse.y >= colorMenu.pos.y
				&& last_mouse.x < (colorMenu.pos.x + colorMenu.getWidth())
				&& last_mouse.y < (colorMenu.pos.y + colorMenu.upperPadding)) {
				colorMenu.pos.x = std::clamp(x - (last_mouse.x - colorMenu.pos.x), 0, (ScreenWidth() - colorMenu.getWidth()));
				colorMenu.pos.y = std::clamp(y - (last_mouse.y - colorMenu.pos.y), 0, (ScreenHeight() - colorMenu.getFullHeight()));
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
			}
		}

		last_mouse = GetMousePos();

		draw:
		Clear(olc::Pixel(200, 255, 255));

		// Draw Image
		DrawRect(imagePos().x - 1, imagePos().y - 1, (surface->width * scale) + 2 , (surface->height * scale) + 2, olc::VERY_DARK_GREY);

		SetPixelMode(olc::Pixel::MASK);
		DrawSprite(imagePos(), surface.get(), uint32_t(scale));
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
