#pragma once
#include <vector>
#include <atomic>
#include <thread>
#include <string_view>

#include "RyuuSC/book.hpp"
#include "RyuuSC/types.hpp"
#include "RyuuSC/decision.hpp"
#include "RyuuSC/Forecast/forecastAlgorithms.hpp"

#include "RyuuArch/Ring/mpsc.hpp"


namespace RyuuSC {

struct Order {
	std::vector<PlayerID>			suppliers;
	std::vector<std::vector<fInt>>		quantity;
};


// ─── Player ───────────────────────────────────────────────────────────────────
/* 
 * 
 */
enum class PlayerState : unsigned char {
	Analysing,
	Executing,
	Done
};

struct Player {
        // Thread
	std::atomic<PlayerState> state{ PlayerState::Done };
        std::atomic<bool>       running { false };
        std::thread             thread;

        // Identification
        PlayerID                id;
        std::string_view        name;

	// Chosen algorithms
	std::unique_ptr<ForecastAlgoBase> forecast_algo;

        // Others
	Book			book;
        std::vector<PlayerID>	suppliers;
        Visibility		visibility = Visibility::Partial;

	// Ring
	MPSC<Order>		inbox;


	constexpr Player(std::string_view n, Visibility v) noexcept
		: name(n), visibility(v) {}


        [[nodiscard]] static constexpr Player make_full(std::string_view n) noexcept {
                return Player{ n, Visibility::Full };
        }

        [[nodiscard]] static constexpr Player make_partial(std::string_view n) noexcept {
                return Player{ n, Visibility::Partial };
        }

        // ─── Forecast Wrapper ─────────────────────────────────────────────────
        /*
         *
         */
        [[nodiscard]] Decision forecast() noexcept {
		return (*forecast_algo)(book);
	}


        // ─── Generate SOP ─────────────────────────────────────────────────────
        /*
         *
         */
        [[nodiscard]] Decision generate_sop() noexcept {
		return {};
	}


        // ─── Generate MPS ─────────────────────────────────────────────────────
        /*
         *
         */
        [[nodiscard]] Decision generate_mps() noexcept {
		return {};
	}


        // ─── Lot Size ─────────────────────────────────────────────────────────
        /*
         *
         */
        [[nodiscard]] Decision opt_lotsize() noexcept {
		return {};
	}


	// ─── Procurement ──────────────────────────────────────────────────────
        /*
         *
         */
        // [[nodiscard]] constexpr Order procure(Decision& d) const noexcept {};

private:
        Decision			decision;

}; // Player

} // Namespace RyuuSC
