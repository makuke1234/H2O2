#include "app.hpp"
#include <iostream>

int main(int argc, char ** argv)
{
	h2o2::app application;
	if (application.init(argc, argv) == false)
	{
		h2o2::err(application.lastErr(), "Unknown error!");
		return 1;
	}

	try
	{
		return application.run();
	}
	catch (const std::exception & e)
	{
		h2o2::err(application.lastErr(), e.what());
		return 1;
	}
}
