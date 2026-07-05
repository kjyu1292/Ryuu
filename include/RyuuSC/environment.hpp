#pragma once
#include <array>
#include <thread>
#include <future>
#include <cstddef>
#include <barrier>
#include <optional>
#include <string_view>
#include <unordered_map>

#include "RyuuSC/types.hpp"
#include "RyuuSC/player.hpp"

#include "RyuuArch/Ring/mpsc.hpp"


namespace RyuuSC {

// ─── Visibility enum ─────────────────────────────────────────────────────────
/**
 * Visibility
 * An attribute of every Player that controls what get_player_state() returns
 * for that player.  Add new levels here as the design grows.
 */
enum class Visibility : unsigned char {
	Full,
	Partial
};


// ─── State structs ───────────────────────────────────────────────────────────
/**
 * GlobalState  —  source of truth, only Environment owns and mutates this.
 * NOTE: state_ is mutated at runtime, consider reimplement get_*_state() methods
 */
struct GlobalState {
	int     			round        = 0;
	int     			total_score  = 0;
};

/**
 * StateView  —  what a player receives from get_player_state<I>().
 *
 * Fields that are not visible to the player are left as std::nullopt.
 * Using std::optional makes it explicit at the call-site which fields
 * are actually present, rather than silently returning zeroed-out data.
 */
struct StateView {
	int                 		round;
	Visibility          		visibility;
	std::optional<int>  		total_score;
	std::optional<std::size_t> 	player_count;
};


// ─── Environment ─────────────────────────────────────────────────────────────
/**
 * Environment<N> comprises of N players and a routing table
 * The main simulation is done via the run(periods) method
 */
template <std::size_t N>
	requires (N > 0)
class Environment {
public:
	static constexpr std::size_t player_count = N;


	// ── Construction ─────────────────────────────────────────────────────
	explicit constexpr Environment(std::array<Player, N> players) noexcept
        	: players_{ std::move(players)... }
        	, state_{ .round = 0, .total_score = 0, .player_count = player_count } 
	{
		build_routing_table();
	}


	// ── Mutators ─────────────────────────────────────────────────────────
	constexpr void advance_round() noexcept { ++state_.round; }
	constexpr void add_score(int delta) noexcept { state_.total_score += delta; }


	// ── Global state (privileged, unrestricted) ──────────────────────────
	/**
 	 * Returns the full GlobalState.
 	 * Intended for system-level observers: logger, referee, renderer, AI trainer.
 	 */
	[[nodiscard]] constexpr GlobalState get_global_state() const noexcept {
    		return state_;
	}


	// ── Per-player state ─────────────────────────────────────────────────
	/**
 	 * Returns a StateView for player I, filtered by that player's Visibility.
 	 *
 	 *   Visibility::Full    → all optional fields are populated
 	 *   Visibility::Partial → restricted fields are std::nullopt
 	 *
 	 * The index bound I < N is enforced at compile time via requires.
 	 * The Visibility branch is a single runtime switch — no virtual dispatch,
 	 * no heap, no RTTI.
 	 *
 	 * param I  Player index in [0, N).  Out-of-range → compile error.
 	 */
	template <std::size_t I>
    		requires (I < player_count)
	[[nodiscard]] constexpr StateView get_player_state() const noexcept {
    		return build_view(std::get<I>(players_).visibility);
	}


	// ── Player accessor ──────────────────────────────────────────────────
	/** Read-only access to player I.  Index checked at compile time. */
	template <std::size_t I>
    		requires (I < player_count)
	[[nodiscard]] constexpr auto& get_player() const noexcept {
    		return std::get<I>(players_);
	}


	// ── Thread Functions ─────────────────────────────────────────────────
	/**
 	 * Run all pipelines for approximately some periods, then shut down.
 	 */
	void run(std::uint32_t periods) noexcept 
	{
		for (auto& player : players_)
		{
			player.thread = std::thread(&Player::run, &player
					, std::ref(*this), periods);
		}

		for (auto& player : players_)
		{
			player.thread.join();
		}
	}


	// ── Player public API ─────────────────────────────────────────────────
	[[nodiscard]] bool route_order(PlayerID target, const Order& order) noexcept {
		auto it = routing_table_.find(target);
		if (it == routing_table_.end())
			// TODO: unknown suppliers
			return false;
		return it->second->try_push(order);
	}

private:

	// ── Helpers ──────────────────────────────────────────────────────────
	[[nodiscard]] constexpr StateView build_view(Visibility v) const noexcept {
    		switch (v) {
        		case Visibility::Full:
        	    		return StateView{
        	        		.visibility   = Visibility::Full,
        	        		.round        = state_.round,
        	        		.total_score  = state_.total_score,
        	        		.player_count = state_.player_count,
        	    		};
        		case Visibility::Partial:
        	    		return StateView{
        	        		.visibility   = Visibility::Partial,
        	        		.round        = state_.round,
        	        		.total_score  = std::nullopt,
        	        		.player_count = std::nullopt,
        	    		};
    		}
    		// Unreachable — silences -Wreturn-type on some compilers.
    		__builtin_unreachable();
	}

	void build_routing_table() noexcept {
		for (auto& p : players_)
			routing_table_[p.id] = &p.inbox;
	}

	// Completion function: runs exactly once per period, on whichever thread
	// happends to be last to arrive, and *before* anyone is released from the barrier.
	struct RoundCompletion {
		Environment*	env;
		void operator()() noexcept { env->advance_round(); }
	};

	// ── Data members ─────────────────────────────────────────────────────
	std::array<Player, N>				players_;
	GlobalState       				state_;
	std::barrier<RoundCompletion>			period_barrier;
	std::unordered_map<PlayerID, MPSC<Order>*> 	routing_table_;

};

// ─── Deduction helper ────────────────────────────────────────────────────────
// Environment env{ std::array{ p0, p1, p2 } };
// and have N deduced automatically.

template <std::size_t N>
Environment(std::array<Player, N>) -> Environment<N>;

} // namespace RyuuSC


#include "RyuuSC/player.tpp"
