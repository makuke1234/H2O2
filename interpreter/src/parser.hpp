#pragma once

#include "error.hpp"

#include <string>
#include <string_view>
#include <istream>
#include <ostream>
#include <iostream>
#include <variant>

namespace h2o2::parser
{
	namespace inner
	{
		struct variable
		{
			enum Type
			{
				tString  = 's',
				tNumber  = 'i',
				tProc    = 'p',
				tBoolean = 'b',

			};
			char type;

			std::variant<std::string, double, std::size_t, bool> val;
		};

		void Error(error & ec, const char * errstr);


		bool IsBinaryDigit(char c);
		bool IsOctalDigit(char c);
		bool IsDigit(char c);
		bool IsHexDigit(char c);
		std::uint8_t GetHexDigit(char c);
		bool IsNumberString(std::string_view str);

		bool IsLowerAlpha(char c);
		bool IsUpperAlpha(char c);
		bool IsAlpha(char c);
		bool IsAlNum(char c);

		bool IsAddOp(char c);
		bool IsMulOp(char c);


		char Look();
		char Next();
		char Take();
		bool TakeNext(char c);
		bool TakeString(std::string_view word);
		std::string TakeNextAlNum();


		void Program();
		void Block(bool & act, bool & cont);
		void Statement(bool & act, bool & cont);
		variable Expression(bool & act, bool & cont);

		std::string String(bool & act, bool & cont);
		std::string StringExpression(bool & act, bool & cont);


		bool BooleanFactor(bool & act, bool & cont);
		bool BooleanTerm(bool & act, bool & cont);
		bool BooleanExpression(bool & act, bool & cont);


		double MathFactor(bool & act, bool & cont);
		double MathTerm(bool & act, bool & cont);
		double MathExpression(bool & act, bool & cont);


		void DoIfElse(bool & act, bool & cont);
		void DoWhile(bool & act, bool & cont);
		void DoFor(bool & act, bool & cont);
		void DoContinue(bool & act, bool & cont);
		void DoBreak(bool & act);
		void DoSubOp(bool & act, bool & cont);
		void DoSubDef();
		void DoSubCall(bool & act, bool & cont, bool & sub);
		void DoVarOp(bool & act, bool & cont);
		void DoAssign(bool & act, bool & cont);
		void DoPrint(bool & act, bool & cont);

	}

	int parse(
		const std::string & input,
		error & ec,
		std::istream & istream = std::cin,
		std::ostream & ostream = std::cout,
		std::ostream & errstream = std::cerr
	);
}
