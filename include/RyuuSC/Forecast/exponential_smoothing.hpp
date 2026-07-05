#pragma once

#include "RyuuSC/book.hpp"
#include "RyuuSC/decision.hpp"

#include "RyuuSC/Forecast/forecast_base.hpp"

namespace RyuuSC::Forecast {

struct ExponentialSmoothing final : ForecastAlgoBase {
	float				alpha;

	template <typename T>
	[[nodiscard]] Decision operator()(const Book& book) const noexcept override {
		Decision d;
		/* */
		return d;
	}

	[[nodiscard]] std::string_view name() const noexcept override
		{ return "ES"; }
};

}

