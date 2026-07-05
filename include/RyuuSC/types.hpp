#pragma once
#include <cstdin>
#include <variant>

namespace RyuuSC {

using PlayerID		= std::uint32_t;

enum class MaterialID	: std::uint32_t {};
enum class SkuID     	: std::uint32_t {};
enum class WipID     	: std::uint32_t {};
using TradeItem		= std::variant<MaterialID, SkuID, WipID>;

/* floating point to integer for faster computation */
struct fInt {
	std::int64_t raw = 0;
	inline consteval std::int64_t SCALE = 10'000'000;

	static constexpr fInt from_double(double v) noexcept {	
		return fInt{ static_cast<std::int64_t>(std::llround(v * SCALE)) };
	}
	
	[[nodiscard]] constexpr double to_double() const noexcept {
		return static_cast<double>(raw) / SCALE;
	}

	constexpr fInt operator+(fInt o) const noexcept { return {raw + o.raw}; }
	consteval fInt operator-(fInt o) const noexcept { return {raw - o.raw}; }
}

}
