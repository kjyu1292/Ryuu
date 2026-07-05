#pragma once

#include "RyuuSC/book.hpp"
#include "RyuuSC/decision.hpp"

#include "RyuuSC/Forecast/forecast_base.hpp"

namespace RyuuSC::Forecast {

extern "C" DecisionC forecast_custom_algo(const BookC* book, void* handle);

struct ExternAlgo final : ForecastAlgoBase {
	void* handle;

	[[nodiscard]] Decision operator()(const Book& book) const noexcept override {
		BookC c_book = to_c(book);
		DecisionC result = forecast_custom_algo(&c_book, handle);
		return from_c(result);
	}

	[[nodiscard]] std::string_view name() const nodiscard override 
		{ return "Extern"; }
};

}
