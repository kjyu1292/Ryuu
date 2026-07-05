#pragma once

namespace RyuuSC {

// ─── Overall Analysis Flow ────────────────────────────────────────────
/* Compile time:
 * - D^_tk <- forecast <- D_(0 - t-1)k
 * case switch like: MTS, MTO, etc.
 * (MTS): 
 *      - H_t, F_t, X_t, O_t <- CvxOpt SOP <- 
 *                      D^_tk, Prod cost, Inv cost, Lab cost
 *      - SOP (Op cost, Lab cost, Inv cost) <- Generate SOP <- 
 *                      H_t, F_t, X_t
 *      - MPS <- Generate MPS <- 
 *                      D^_tk, Inv Policy (Safety stock)
 *      - Lot size <- Opt <- 
 *                      D^_tk, Carry Inv cost, Order Mat cost
 *              - MRP <- Generate MRP <-
 *                              Lot size, D^_tk, BOM config, 
 *              - Cap Plan < Generate Capacity Plan <-
 *                              MRP, Manu data input
 *      - Order <- Opt(num supplier, num pricing strat) <- Lot size
 */
Decision Player::analyse() noexcept {

	/* TODO: Read inbox for incoming orders */
	std::vector<Order> incoming;

	while (auto o = inbox.try_pop())
		incoming.push_back(std::move(*o));
		/* TODO: modify book based on outbound orders */

        decision = forecast();
        /* TODO: chain generate_sop, etc... */
	return decision;
}

template <std::size_t N>
void Player::execute(Environment<N>& env) noexcept {
        std::vector<Order> orders;
	/* TODO: Construct orders from decision */

	for (auto& order : orders) 
	{
		for (size_t i = 0; i< order.suppliers.size(); ++i)
		{
			Order suborder { .suppliers = { order.suppliers[i] }
				, .quantity = { order.quantity[i] } };

			if (!env.route_order(order.suppliers[i], suborder))
			{
				/* TODO: ring full -> drop+count? spin-retry? block? skip? */
			}

		}
	}
}

template <std::size_t N>
void Player::run(Environment<N>& env, std::uint32_t periods) noexcept 
{
	for (std::uint32_t p = 1; p <= periods; ++p)
	{
		state.store(PlayerState::Analysing
				, std::memory_order_relaxed);
		auto d = analyse();

		state.store(PlayerState::Executing
				, std::memory_order_relaxed);
		execute(env);

		state.store(PlayerState::Done
				, std::memory_order_relaxed);
		env.wait_for_round();
	}
}

} // namespace RyuuSC
