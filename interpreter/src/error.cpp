#include "error.hpp"

#include <iostream>

void h2o2::err(error code) noexcept
{
	return err(errStr(code));
}
void h2o2::err(std::string_view msg) noexcept
{
	std::cerr << msg << std::flush;
}
