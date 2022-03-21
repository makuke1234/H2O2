#pragma once

#include <cstdint>
#include <string_view>
#include <cassert>

namespace h2o2
{
	enum class error : std::uint8_t
	{
		ok,
		unknown,
		noInput,
		inputFile,

		error_enum_size
	};

	constexpr std::string_view g_errStrings[]
	{
		"No error",
		"Unknown error",
		"No input file!",
		"Cannot open input file!"
	};

	void err(error code, const char * what) noexcept;
	void err(std::string_view msg) noexcept;
	[[nodiscard]] constexpr std::string_view errStr(error code, const char * what) noexcept
	{
		using underT = std::underlying_type_t<error>;
		assert(underT(code) < underT(error::error_enum_size));
		
		if (code == error::unknown)
		{
			return what;
		}
		else
		{
			return g_errStrings[underT(code)];
		}
	}

}
