#pragma once

#include "error.hpp"

#include <string>
#include <istream>
#include <ostream>
#include <iostream>

namespace h2o2::parser
{
	int parse(
		const std::string & input,
		error & ec,
		std::istream & istream = std::cin,
		std::ostream & ostream = std::cout,
		std::ostream & errstream = std::cerr
	);
}
