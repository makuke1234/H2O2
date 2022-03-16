#pragma once

#include <string>

namespace argsys
{
	class parser
	{
	private:
		int m_argc{ 0 };
		char ** m_argv{ nullptr };

		std::string infile;

	public:
		parser() noexcept = default;
		parser(int argc, char ** argv);

		[[nodiscard]] std::string getInputFile() const;
	};
}
