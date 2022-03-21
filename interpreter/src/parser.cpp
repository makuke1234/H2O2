#include "parser.hpp"

#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace h2o2::parser::inner
{
	static error s_ec = error::ok;

	static std::string_view source;
	static error * ec = &s_ec;
	static std::size_t pc;

	static std::unordered_map<std::string, variable> variables;

	static std::istream * is = &std::cin;
	static std::ostream * os = &std::cout;
	static std::ostream * errs = &std::cerr;


	void Error(const char * errstr)
	{
		*ec = error::unknown;
		auto s = source.substr(0, pc).rfind('\n') + 1;
		auto e = source.find('\n', pc);
		throw std::runtime_error(
			(
				"\nERROR " + std::string(errstr) + " in line " +
				std::to_string(std::count(source.begin(), source.begin() + pc, '\n') + 1) + ": '" +
				std::string(source.substr(s, pc - s)) + '_' + std::string(source.substr(pc, e - pc)) + "'\n"
			).c_str()
		);
	}


	bool IsBinaryDigit(char c)
	{
		return c == '0' || c == '1';
	}
	bool IsOctalDigit(char c)
	{
		return c >= '0' && c <= '7';
	}
	bool IsDigit(char c)
	{
		return c >= '0' && c <= '9';
	}
	bool IsHexDigit(char c)
	{
		c = char(tolower(c));
		return IsDigit(c) || (c >= 'a' && c <= 'f');
	}
	std::uint8_t GetHexDigit(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return std::uint8_t(c - '0');
		}
		else
		{
			return std::uint8_t(char(tolower(c)) - 'a' + 10);
		}
	}
	bool IsNumberString(std::string_view str)
	{
		for (auto c : str)
		{
			if (!IsDigit(c))
			{
				return false;
			}
		}
		return true;
	}

	bool IsLowerAlpha(char c)
	{
		return c >= 'a' && c <= 'z';
	}
	bool IsUpperAlpha(char c)
	{
		return c >= 'A' && c <= 'Z';
	}
	bool IsAlpha(char c)
	{
		return IsLowerAlpha(char(tolower(c)));
	}
	bool IsAlNum(char c)
	{
		return IsDigit(c) || IsAlpha(c) || c == '_';
	}

	bool IsAddOp(char c)
	{
		return c == '+' || c == '-';
	}
	bool IsMulOp(char c)
	{
		return c == '*' || c == '/';
	}


	char Look()
	{
		if (source[pc] == '#')
		{
			while (source[pc] != '\n' && source[pc] != '\0')
			{
				++pc;
			}
		}
		return source[pc];
	}
	char Next()
	{
		while (Look() == ' ' || Look() == '\t' || Look() == '\n' || Look() == '\r')
		{
			Take();
		}
		return Look();
	}
	char Take()
	{
		auto c = Look();
		++pc;
		return c;
	}
	bool TakeNext(char c)
	{
		if (Next() == c)
		{
			Take();
			return true;
		}
		else
		{
			return false;
		}
	}
	bool TakeString(std::string_view word)
	{
		auto copypc = pc;
		for (char c : word)
		{
			if (Take() != c)
			{
				pc = copypc;
				return false;
			}
		}
		return true;
	}
	std::string TakeNextAlNum()
	{
		std::string alnum;
		if (IsAlpha(Next()))
		{
			while (IsAlNum(Look()))
			{
				alnum += Take();
			}
		}
		return alnum;
	}


	void Program()
	{
		bool act = true, cont = false;
		while (Next() != '\0')
		{
			Block(act, cont);
		}
	}
	void Block(bool & act, bool & cont)
	{
		if (TakeNext('{'))
		{
			while (!TakeNext('}'))
			{
				Block(act, cont);
			}
		}
		else
		{
			Statement(act, cont);
		}
	}
	void Statement(bool & act, bool & cont)
	{
		if (TakeString("print"))
		{
			DoPrint(act, cont);
		}
		else if (TakeString("if"))
		{
			DoIfElse(act, cont);
		}
		else if (TakeString("while"))
		{
			DoWhile(act, cont);
		}
		else if (TakeString("for"))
		{
			DoFor(act, cont);
		}
		else if (TakeString("break"))
		{
			DoBreak(act);
		}
		else if (TakeString("continue"))
		{
			DoContinue(act, cont);
		}
		else if (TakeString("sub"))
		{
			DoSubDef();
		}
		else
		{
			bool sub = false;
			DoSubCall(act, cont, sub);
			if (sub == false)
			{
				DoVarOp(act, cont);
			}
		}
	}
	variable Expression(bool & act, bool & cont)
	{
		auto copypc = pc;
		auto ident = TakeNextAlNum();
		pc = copypc;

		if (auto it = variables.find(ident); (Next() == '"') || (ident == "str") || (ident == "input") || ((it != variables.end()) && (it->second.type == variable::tString)))
		{
			return { variable::tString, StringExpression(act, cont) };
		}
		else
		{
			return { variable::tNumber, MathExpression(act, cont) };
		}
	}

	std::string String(bool & act, bool & cont)
	{
		std::string s;
		if (TakeNext('"'))
		{
			while (!TakeString("\""))
			{
				if (Look() == '\0')
				{
					Error("Unexpected EOF");
				}
				else if (TakeString("\\n"))
				{
					s += '\n';
				}
				else if (TakeString("\\\\"))
				{
					s += '\\';
				}
				else if (TakeString("\\t"))
				{
					s += '\t';
				}
				else if (TakeString("\\\""))
				{
					s += '"';
				}
				else if (TakeString("\\'"))
				{
					s += '\'';
				}
				else
				{
					s += Take();
				}
			}
		}
		else if (TakeString("str("))
		{
			std::stringstream ss;
			ss << std::scientific << std::setprecision(16);
			ss.unsetf(std::ios::floatfield);
			ss << MathExpression(act, cont);
			s = ss.str();
			if (!TakeNext(')'))
			{
				Error("Missing ')'");
			}
		}
		else if (TakeString("input()"))
		{
			if (act && !cont)
			{
				std::getline(*is, s);
			}
		}
		else
		{
			auto ident = TakeNextAlNum();
			if (auto it = variables.find(ident); (it != variables.end()) && (it->second.type == variable::tString))
			{
				s = std::get<std::string>(it->second.val);
			}
			else
			{
				Error("Not a string");
			}
		}

		return s;
	}
	std::string StringExpression(bool & act, bool & cont)
	{
		auto s = String(act, cont);
		while (TakeNext('+'))
		{
			s += String(act, cont);
		}
		return s;
	}


	bool BooleanFactor(bool & act, bool & cont)
	{
		auto inv = TakeNext('!');
		auto e = Expression(act, cont);
		auto & b = e.val;
		Next();

		bool ret;

		if (e.type == variable::tNumber)
		{
			double num = std::get<double>(b);
			if (TakeString("=="))
			{
				ret = (num == MathExpression(act, cont));
			}
			else if (TakeString("!="))
			{
				ret = (num != MathExpression(act, cont));
			}
			else if (TakeString("<="))
			{
				ret = (num <= MathExpression(act, cont));
			}
			else if (TakeString("<"))
			{
				ret = (num < MathExpression(act, cont));
			}
			else if (TakeString(">="))
			{
				ret = (num >= MathExpression(act, cont));
			}
			else if (TakeString(">"))
			{
				ret = (num > MathExpression(act, cont));
			}
		}
		else if (e.type == variable::tBoolean)
		{
			ret = std::get<bool>(b);
		}
		else
		{
			std::string & s = std::get<std::string>(b);
			if (TakeString("=="))
			{
				ret = (s == StringExpression(act, cont));
			}
			else if (TakeString("!="))
			{
				ret = (s != StringExpression(act, cont));
			}
			else
			{
				ret = !s.empty();
			}
		}

		return act && !cont && (ret != inv);
	}
	bool BooleanTerm(bool & act, bool & cont)
	{
		auto b = BooleanFactor(act, cont);
		while (TakeNext('&'))
		{
			b = b & BooleanFactor(act, cont);
		}
		return b;
	}
	bool BooleanExpression(bool & act, bool & cont)
	{
		auto b = BooleanTerm(act, cont);
		while (TakeNext('|'))
		{
			b = b | BooleanTerm(act, cont);
		}
		return b;
	}


	double MathFactor(bool & act, bool & cont)
	{
		double m = 0.0;
		double frac = 0.1;

		bool notdec = false;

		if (TakeNext('('))
		{
			m = MathExpression(act, cont);
			if (!TakeNext(')'))
			{
				Error("Missing ')'");
			}
		}
		else if (IsDigit(Next()))
		{
			if (Look() == '0')
			{
				notdec = true;
				Take();
				auto ch = Look();
				if (ch == 'b')
				{
					Take();
					while (IsBinaryDigit(Look()))
					{
						m = 2.0 * m + double(Take() - '0');
					}
				}
				else if (ch == 'x' || ch == 'X')
				{
					Take();
					while (IsHexDigit(Look()))
					{
						m = 16.0 * m + double(GetHexDigit(Take()));
					}
				}
				else if (IsOctalDigit(ch))
				{
					while (IsOctalDigit(Look()))
					{
						m = 8.0 * m + double(Take() - '0');
					}
				}
				else if (ch == '.')
				{
					notdec = false;
				}
			}
			if (!notdec)
			{
				while (IsDigit(Look()))
				{
					m = 10.0 * m + double(Take() - '0');
				}
				if (Look() == '.')
				{
					Take();
					while (IsDigit(Look()))
					{
						m = m + frac * double(Take() - '0');
						frac *= 0.1;
					}
				}
			}
		}
		else if (TakeString("val("))
		{
			auto s = String(act, cont);
			if (act && !cont && IsNumberString(s))
			{
				m = double(std::strtoll(s.c_str(), nullptr, 10));
			}
			if (!TakeNext(')'))
			{
				Error("Missing ')'");
			}
		}
		else
		{
			auto ident = TakeNextAlNum();
			if (auto it = variables.find(ident); (it == variables.end()) || (it->second.type != variable::tNumber))
			{
				Error("Unknown variable");
			}
			else if (act && !cont)
			{
				m = std::get<double>(it->second.val);
			}
		}

		return m;
	}
	double MathTerm(bool & act, bool & cont)
	{
		auto m = MathFactor(act, cont);
		while (IsMulOp(Next()))
		{
			auto c = Take();
			auto m2 = MathFactor(act, cont);
			if (c == '*')
			{
				m *= m2;
			}
			else
			{
				m /= m2;
			}
		}
		
		return m;
	}
	double MathExpression(bool & act, bool & cont)
	{
		auto c = Next();
		if (IsAddOp(c))
		{
			c = Take();
		}

		auto m = MathTerm(act, cont);
		if (c == '-')
		{
			m = -m;
		}

		while (IsAddOp(Next()))
		{
			c = Take();
			auto m2 = MathTerm(act, cont);
			if (c == '+')
			{
				m += m2;
			}
			else
			{
				m -= m2;
			}
		}

		return m;
	}


	void DoIfElse(bool & act, bool & cont)
	{
		auto b = BooleanExpression(act, cont);
		if (act && !cont && b)
		{
			Block(act, cont);
		}
		else
		{
			bool newact = false;
			Block(newact, cont);
		}
		Next();

		if (TakeString("else"))
		{
			if (act && !cont && !b)
			{
				Block(act, cont);
			}
			else
			{
				bool newact = false;
				Block(newact, cont);
			}
		}
	}
	void DoWhile(bool & act, bool & cont)
	{
		auto localAct = act;
		auto localCont = cont;
		auto pc_while = pc;

		while (BooleanExpression(localAct, localCont))
		{
			localCont = true;
			Block(localAct, localCont);
			pc = pc_while;
		}
		localAct = false;
		Block(localAct, cont);
	}
	void DoFor(bool & act, bool & cont)
	{
		Next();
		if (!TakeNext(','))
		{
			DoVarOp(act, cont);
			TakeNext(',');
		}

		Next();

		auto localAct1 = act;
		auto localCont1 = cont;
		auto pc_for = pc;

		auto bexp = !TakeNext(',');

		while (!TakeNext(','))
		{
			Take();
		}

		Next();
		auto localAct2 = act;
		auto localCont2 = cont;
		auto pc_assign = pc;

		{
			bool temp1 = false;
			bool temp2 = false;
			DoAssign(temp1, temp2);
		}
		auto pc_block = pc;

		pc = pc_for;

		while ((bexp && BooleanExpression(localAct1, localCont1)) || !bexp)
		{
			localCont1 = false;
			pc = pc_block;

			Block(localAct1, localCont1);

			pc = pc_assign;
			DoAssign(localAct2, localCont2);

			pc = pc_for;
		}

		pc = pc_block;

		localAct1 = false;
		Block(localAct1, cont);
	}
	void DoContinue(bool & act, bool & cont)
	{
		if (act)
		{
			cont = true;
		}
	}
	void DoBreak(bool & act)
	{
		act = false;
	}
	void DoSubOp(bool & act, bool & cont)
	{
		if (!TakeString("sub"))
		{
			bool sub = false;
			DoSubCall(act, cont, sub);
			if (sub == false)
			{
				Error("Unknown function call");
			}
		}
		else
		{
			DoSubDef();
		}
	}
	void DoSubDef()
	{
		auto ident = TakeNextAlNum();
		if (ident.empty())
		{
			Error("Missing subroutine identifier");
		}
		variables.emplace(ident, variable{ variable::tProc, pc });
		bool act = false, cont = false;
		Block(act, cont);
	}
	void DoSubCall(bool & act, bool & cont, bool & sub)
	{
		auto prevpc = pc;
		auto funident = TakeNextAlNum();
		auto it = variables.find(funident);
		if (it == variables.end() || it->second.type != variable::tProc)
		{
			sub = false;
			pc = prevpc;
			return;
		}
		sub = true;

		auto localVariables = variables;

		while (TakeNext(','))
		{
			auto ident = TakeNextAlNum();
			if (!TakeNext('=') || ident.empty())
			{
				Error("No variable name given");
			}

			variables[ident] = Expression(act, cont);
		}

		auto ret = pc;
		pc = std::get<std::size_t>(it->second.val);

		Block(act, cont);

		pc = ret;
		variables = std::move(localVariables);
	}
	void DoVarOp(bool & act, bool & cont)
	{
		if (!TakeString("var"))
		{
			DoAssign(act, cont);
		}
		else
		{
			auto ident = TakeNextAlNum();
			variable e = { variable::tNumber, 0.0 };
			if (ident.empty())
			{
				Error("Not a variable name");
			}
			else if (TakeNext('='))
			{
				e = Expression(act, cont);
			}

			if (auto it = variables.find(ident); (act && !cont) || (it == variables.end()))
			{
				variables[ident] = e;
			}
		}
	}
	void DoAssign(bool & act, bool & cont)
	{
		auto ident = TakeNextAlNum();
		if (!TakeNext('=') || ident.empty())
		{
			Error("Unknown statement");
		}
		auto e = Expression(act, cont);
		if (auto it = variables.find(ident); (act && !cont) || (it == variables.end()))
		{
			variables[ident] = e;
		}
	}
	void DoPrint(bool & act, bool & cont)
	{
		do
		{
			auto e = Expression(act, cont);
			if (act && !cont)
			{
				(*os) << std::get<std::string>(e.val);
			}
		} while (TakeNext(','));
	}

}

int h2o2::parser::parse(
	const std::string & input,
	error & ec,
	std::istream & istream,
	std::ostream & ostream,
	std::ostream & errstream
)
{
	// Reset all states
	inner::source = input;
	inner::ec = &ec;
	inner::pc = 0;
	inner::variables.clear();

	inner::is = &istream;
	inner::os = &ostream;
	inner::errs = &errstream;

	ostream << std::scientific << std::setprecision(16);
	errstream << std::scientific << std::setprecision(16);
	ostream.unsetf(std::ios::floatfield);
	errstream.unsetf(std::ios::floatfield);

	inner::Program();

	ostream << std::flush;
	errstream << std::flush;

	return 0;
}
