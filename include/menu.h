#ifndef FILE_MENU_H
#define FILE_MENU_H
#include <memory>
#include "olcPixelGameEngine.h"

namespace paint {
	class Menu {
	private:
		std::shared_ptr<olc::Sprite> sprite{};
		std::shared_ptr<olc::Decal> decal{};
	public:
		olc::vi2d pos{};
		virtual ~Menu() noexcept = default;

		[[nodiscard]]
		virtual olc::vi2d getSize() const noexcept = 0;
		virtual bool contains(int x, int y) noexcept {
			const auto size = getSize();
			return x >= pos.x && y >= pos.y && x < (pos.x + size.x) && y < (pos.y + size.y);
		}
		void update(olc::PixelGameEngine& pge, float delta) {
			onUpdate(pge, delta);
			sprite = draw();
			decal = std::make_shared<olc::Decal>(sprite.get());
		}
		int getX() const noexcept { return pos.x; }
		int getY() const noexcept { return pos.y; }
		int getWidth() const noexcept { return getSize().x; }
		int getHeight() const noexcept { return getSize().y; }
		const auto& getSprite() const noexcept { return sprite; }
		const auto& getDecal() const noexcept { return decal; }
	protected:
		virtual void onUpdate(olc::PixelGameEngine& pge, float delta) {}
		virtual std::shared_ptr<olc::Sprite> draw() = 0;
	};
}

#endif /* FILE_MENU_H */