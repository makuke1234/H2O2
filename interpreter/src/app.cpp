#include "app.hpp"
#include "parser.hpp"

#include <exception>
#include <stdexcept>
#include <fstream>
#include <iostream>

h2o2::app::app() noexcept
{

}

[[nodiscard]] bool h2o2::app::init(int argc, char ** argv) noexcept
{
	try
	{
		this->arguments = { argc, argv };
	}
	catch (const std::runtime_error &)
	{
		this->err = error::noInput;
		return false;
	}
	catch (const std::exception &)
	{
		this->err = error::unknown;
		return false;
	}

	this->initialised = true;
	return true;
}

[[nodiscard]] int h2o2::app::run()
{
	// Read data from input file
	std::string data;

	try
	{
		data.assign(app::readFile(this->arguments.getInputFile()));
	}
	catch (const std::exception & e)
	{
		this->err = error::inputFile;
		throw e;
	}

	// Parse input
	return parser::parse(data, this->err, this->m_ioInp, this->m_ioOut, this->m_ioErr);
}

[[nodiscard]] std::string h2o2::app::readFile(const std::string & fname)
{
	std::ifstream in{ fname, std::ios::in | std::ios::binary };
	if (in)
	{
		std::string contents;

		in.seekg(0, std::ios::end);
		contents.resize(std::size_t(in.tellg()) + 1);
		in.seekg(0, std::ios::beg);
		in.read(contents.data(), contents.size());
		in.close();

		return contents;
	}
	throw std::runtime_error("File error!");
}

