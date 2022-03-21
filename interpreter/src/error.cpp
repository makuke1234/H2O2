#include "error.hpp"

#include <iostream>

void h2o2::err(error code, const char * what) noexcept
{
	return err(errStr(code, what));
}
void h2o2::err(std::string_view msg) noexcept
{
	std::cerr << msg << std::endl;
}
