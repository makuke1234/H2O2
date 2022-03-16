#include "app.hpp"
#include <iostream>

int main(int argc, char ** argv)
{
	h2o2::app application;
	if (application.init(argc, argv) == false)
	{
		h2o2::err(application.lastErr());
		return 1;
	}

	try
	{
		return application.run();
	}
	catch (const std::exception &)
	{
		h2o2::err(application.lastErr());
		return 1;
	}
}
