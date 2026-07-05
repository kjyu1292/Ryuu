#pragma once

#include <array>
#include <atomic>
#include <optional>

namespace RyuuArch::Ring {

#ifdef __cpp_lib_hardware_interference_size
	inline constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
	inline constexpr size_t CACHE_LINE_SIZE = 64;
#endif


template <typename T, std::size_t Capacity>
class MPSC {
	static_assert((Capacity & (Capacity - 1)) == 0
		, "Capacity must be power of 2.");
	static_assert(Capacity >= 2
		, "Capacity must be at least 2.");

public:
	MPSC() noexcept = default;

	/* 
	 * No copy, no move
	 */
	MPSC(const MPSC&) = delete;
	MPSC& operator=(const MPSC&) = delete;
	MPSC(MPSC&&) = delete;
	MPSC& operator=(MPSC&&) = delete;

	/* * */
	[[nodiscard]] bool try_push(const T& item) noexcept;

	[[nodiscard]] std::optional<T> try_pop() noexcept;

private:
	// static constexpr size_t m_ = Capacity - 1;

	alignas(CACHE_LINE_SIZE) std::atomic<size_t> h_{0};
	alignas(CACHE_LINE_SIZE) size_t cached_t_{0};

	alignas(CACHE_LINE_SIZE) std::atomic<size_t> t_{0};
	alignas(CACHE_LINE_SIZE) size_t cached_h_{0};

	alignas(CACHE_LINE_SIZE) std::array<T, Capacity> buffer_{};
}; // class MPSC

} // namespace RyuuArch::Ring






















