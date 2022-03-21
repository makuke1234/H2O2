#pragma once

#include "error.hpp"
#include "argsys.hpp"

#include <istream>
#include <ostream>
#include <iostream>

namespace h2o2
{
	class app
	{
	private:
		bool initialised{ false };
		error err{ error::ok };

		argsys::parser arguments;

		std::istream & m_ioInp{ std::cin };
		std::ostream & m_ioOut{ std::cout };
		std::ostream & m_ioErr{ std::cerr };

	public:
		[[nodiscard]] constexpr error lastErr() const noexcept
		{
			return this->err;
		}

		app() noexcept;
		[[nodiscard]] bool init(int argc, char ** argv) noexcept;

		[[nodiscard]] int run();

		[[nodiscard]] static std::string readFile(const std::string & fname);
	};

}
