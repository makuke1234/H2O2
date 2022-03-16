#include "argsys.hpp"
#include "error.hpp"

#include <cassert>
#include <stdexcept>

argsys::parser::parser(int argc, char ** argv)
	: m_argc{ argc }, m_argv{ argv }
{
	assert(argc > 0);
	assert(argv != nullptr);
	assert(argv[0] != nullptr);

	if (argc < 2)
	{
		throw std::runtime_error("No input data!");
	}

	this->infile = argv[1];
}

[[nodiscard]] std::string argsys::parser::getInputFile() const
{
	return this->infile;
}

