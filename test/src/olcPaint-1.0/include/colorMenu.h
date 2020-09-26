#ifndef FILE_COLORMENU_H
#define FILE_COLORMENU_H
#include <array>
#include "menu.h"

namespace paint {
	template<std::size_t N>
	class ColorMenu : public Menu {
	private:
		std::array<olc::Pixel, N> colors{};
	public:
		olc::Pixel fgColor, bgColor;
		int colorsPerRow = 2;
		int boxSize = 16;
		int padding = 5;
		int padding2 = 15;
		int upperPadding = 10;
		olc::Pixel menuBackground = olc::GREY;
		olc::Pixel border = olc::WHITE;

		ColorMenu() = default;
		ColorMenu(const std::array<olc::Pixel, N>& colors)
			: colors(colors) {}

		void setColor(std::size_t i, olc::Pixel c) noexcept {
			if (i < colors.size()) colors[i] = c;
		}
		[[nodiscard]]
		auto getColor(std::size_t i) const noexcept {
			return i < colors.size() ? colors[i] : olc::BLANK;
		}
		[[nodiscard]]
		const olc::Pixel& operator[](std::size_t i) const noexcept {
			return colors[i];
		}
		[[nodiscard]]
		olc::Pixel& operator[](std::size_t i) noexcept {
			return colors[i];
		}

		[[nodiscard]]
		olc::vi2d getSize() const noexcept override {
			const int w = ((colorsPerRow + 1) * padding) + (colorsPerRow * boxSize);
			const int rows = (colors.size() + colorsPerRow - 1) / colorsPerRow;
			const int h = (rows * padding) + (rows * boxSize) + upperPadding;
			return { w, h };
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
	protected:
		void onUpdate(olc::PixelGameEngine& pge, float delta) override {

		}
		olc::vi2d getBoxPos(std::size_t ci) const noexcept {
			const int posx = (ci % colorsPerRow) * boxSize + (ci % colorsPerRow + 1) * padding;
			const int posy = (ci / colorsPerRow) * boxSize + (ci / colorsPerRow) * padding + upperPadding;
			return { posx, posy };
		}
		std::shared_ptr<olc::Sprite> draw() override {
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

}

#endif /* FILE_COLORMENU_H */