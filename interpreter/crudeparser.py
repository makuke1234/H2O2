import sys

pc, variable, source = 0, {}, ""


def main():
	global source

	if len(sys.argv) < 2:
		print("Usage crudeparser.py [sourcefile]")
		exit(1)
	try:
		f = open(sys.argv[1], "r")
	except:
		print("ERROR: Can't find source file \"" + sys.argv[1] + "\".")
		exit(1)
	
	source = f.read() + '\0'
	f.close()

	Program()

def Error(text):
	s = source[:pc].rfind("\n") + 1
	e = source.find("\n", pc)
	print("\nERROR " + text + " in line " + str(source[:pc].count("\n") + 1) + ": '" + source[s:pc] + "_" + source[pc:e] + "'\n")
	exit(1)

# Program code
def Program():
	act, cont = [True], [False]
	while Next() != '\0':
		Block(act, cont)

def Next():
	while Look() == ' ' or Look() == '\t' or Look() == '\n' or Look() == '\r':
		Take()
	return Look()

def Look():
	global pc
	if source[pc] == '#':
		while source[pc] != '\n' and source[pc] != '\0':
			pc += 1
	return source[pc]

def Take():
	global pc
	c = Look()
	pc += 1
	return c

def IsBinaryDigit(c):
	return c == '0' or c == '1'
def IsOctalDigit(c):
	return c >= '0' and c <= '7'
def IsDigit(c):
	return c >= '0' and c <= '9'
def IsHexDigit(c):
	c = c.lower()
	return IsDigit(c) or (c >= 'a' and c <= 'f')

def GetHexDigit(c):
	if c >= '0' and c <= '9':
		return ord(c) - ord('0')
	else:
		return ord(c.lower()) - ord('a') + 10

def IsLowerAlpha(c):
	return c >= 'a' and c <= 'z'
def IsUpperAlpha(c):
	return c >= 'A' and c <= 'Z'
def IsAlpha(c):
	return IsLowerAlpha(c) or IsUpperAlpha(c)
def IsAlNum(c):
	return IsDigit(c) or IsAlpha(c) or c == '_'

def IsAddOp(c):
	return c == '+' or c == '-'
def IsMulOp(c):
	return c == '*' or c == '/'

def TakeNextAlNum():
	alnum = ""
	if IsAlpha(Next()):
		while IsAlNum(Look()):
			alnum += Take()
	return alnum

def BooleanFactor(act, cont):
	inv = TakeNext('!')
	e = Expression(act, cont)
	b = e[1]
	Next()

	if e[0] == 'i':
		if TakeString("=="):
			b = (b == MathExpression(act, cont))
		elif TakeString("!="):
			b = (b != MathExpression(act, cont))
		elif TakeString("<="):
			b = (b <= MathExpression(act, cont))
		elif TakeString("<"):
			b = (b < MathExpression(act, cont))
		elif TakeString(">="):
			b = (b >= MathExpression(act, cont))
		elif TakeString(">"):
			b = (b > MathExpression(act, cont))
	elif e[0] == 'b':
		b = b == True
	else:
		if TakeString("=="):
			b = (b == StringExpression(act, cont))
		elif TakeString("!="):
			b = (b != StringExpression(act, cont))
		else:
			b = (b != "")
	return act[0] and not cont[0] and (b != inv)

def BooleanTerm(act, cont):
	b = BooleanFactor(act, cont)
	while TakeNext('&'):
		b = b & BooleanFactor(act, cont)
	return b

def BooleanExpression(act, cont):
	b = BooleanTerm(act, cont)
	while TakeNext('|'):
		b = b | BooleanTerm(act, cont)
	return b

def MathFactor(act, cont):
	global variable
	m = 0
	frac = 0.1

	notdec = False
	if TakeNext('('):
		m = MathExpression(act, cont)
		if not TakeNext(')'):
			Error("missing ')'")
	elif IsDigit(Next()):
		if (Look() == '0'):
			notdec = True
			Take()
			ch = Look()
			if ch == 'b':
				Take()
				while IsBinaryDigit(Look()):
					m = 2 * m + ord(Take()) - ord('0')
			elif ch == 'x' or ch == 'X':
				Take()
				while IsHexDigit(Look()):
					m = 16 * m + GetHexDigit(Take())
			elif IsOctalDigit(ch):
				while IsOctalDigit(Look()):
					m = 8 * m + ord(Take()) - ord('0')
			elif ch == '.':
				notdec = False
		if notdec == False:
			while IsDigit(Look()):
				m = 10 * m + ord(Take()) - ord('0')
			if (Look() == '.'):
				Take()
				while IsDigit(Look()):
					m = m + frac * (ord(Take()) - ord('0'))
					frac = 0.1 * frac
	elif TakeString("val("):
		s = String(act, cont)
		if act[0] and not cont[0] and s.isdigit():
			m = int(s)
		if not TakeNext(')'):
			Error("missing ')'")
	else:
		ident = TakeNextAlNum()
		if ident not in variable or variable[ident][0] != 'i':
			Error("unknown variable")
		elif act[0] and not cont[0]:
			m = variable[ident][1]
	
	return m

def MathTerm(act, cont):
	m = MathFactor(act, cont)
	while IsMulOp(Next()):
		c = Take()
		m2 = MathFactor(act, cont)
		if c == '*':
			m = m * m2
		else:
			m = m / m2
	return m

def MathExpression(act, cont):
	c = Next()
	if IsAddOp(c):
		c = Take()

	m = MathTerm(act, cont)
	if c == '-':
		m = -m
	
	while IsAddOp(Next()):
		c = Take()
		m2 = MathTerm(act, cont)
		if c == '+':
			m = m + m2
		else:
			m = m - m2
	
	return m

def String(act, cont):
	global variable
	s = ""
	if TakeNext('\"'):
		while not TakeString("\""):
			if Look() == '\0':
				Error("unexpected EOF")
			elif TakeString("\\n"):
				s += '\n'
			elif TakeString("\\\\"):
				s += '\\'
			elif TakeString("\\t"):
				s += '\t'
			elif TakeString("\\\""):
				s += '\"'
			elif TakeString("\\\'"):
				s += '\''
			else:
				s += Take()
	elif TakeString("str("):
		s = str(MathExpression(act, cont))
		if not TakeNext(')'):
			Error("missing ')'")
	elif TakeString("input()"):
		if act[0] and not cont[0]: s = input()
	else:
		ident = TakeNextAlNum()
		if ident in variable and variable[ident][0] == 's':
			s = variable[ident][1]
		else:
			Error("not a string")
	
	return s

def StringExpression(act, cont):
	s = String(act, cont)
	while TakeNext('+'):
		s += String(act, cont)
	return s

def Expression(act, cont):
	global pc
	copypc = pc
	ident = TakeNextAlNum()
	pc = copypc
	if Next() == '\"' or ident == "str" or ident == "input" or (ident in variable and variable[ident][0] == 's'):
		return ('s', StringExpression(act, cont))
	else:
		return ('i', MathExpression(act, cont))

def TakeNext(c):
	if Next() == c:
		Take()
		return True
	else:
		return False

def TakeString(word):
	global pc
	copypc = pc
	for c in word:
		if Take() != c:
			pc = copypc
			return False
	return True

def Block(act, cont):
	if TakeNext('{'):
		while not TakeNext('}'):
			Block(act, cont)
	else:
		Statement(act, cont)

def Statement(act, cont):
	if TakeString("print"):
		DoPrint(act, cont)
	elif TakeString("if"):
		DoIfElse(act, cont)
	elif TakeString("while"):
		DoWhile(act, cont)
	elif TakeString("for"):
		DoFor(act, cont)
	elif TakeString("break"):
		DoBreak(act)
	elif TakeString("continue"):
		DoContinue(act, cont)
	elif TakeString("sub"):
		DoSubDef()
	else:
		sub = [False]
		DoSubCall(act, cont, sub)
		if (sub[0] == False):
			DoVarOp(act, cont)


def DoIfElse(act, cont):
	b = BooleanExpression(act, cont)
	if act[0] and not cont[0] and b:
		Block(act, cont)
	else:
		Block([False], cont)
	Next()

	if TakeString("else"):
		if act[0] and not cont[0] and not b:
			Block(act, cont)
		else:
			Block([False], cont)

def DoWhile(act, cont):
	global pc
	local = [act[0]]
	localCont = [cont[0]]
	pc_while = pc
	while BooleanExpression(local, localCont):
		localCont[0] = True
		Block(local, localCont)
		pc = pc_while
	Block([False], cont)


def DoFor(act, cont):
	Next()
	if not TakeNext(','):
		DoVarOp(act, cont)
		TakeNext(',')
	
	Next()

	global pc
	local = [act[0]]
	localCont = [cont[0]]
	pc_for = pc


	bexp = True
	if TakeNext(','):
		bexp = False


	while not TakeNext(','):
		Take()
	

	Next()
	local2 = [act[0]]
	local2Cont = [cont[0]]
	pc_assign = pc

	DoAssign([False], [False])
	pc_block = pc

	pc = pc_for

	while (bexp and BooleanExpression(local, localCont)) or not bexp:
		localCont[0] = False
		pc = pc_block

		Block(local, localCont)
		
		pc = pc_assign
		DoAssign(local2, local2Cont)
		
		pc = pc_for

	pc = pc_block

	Block([False], cont)


def DoBreak(act):
	if act[0]:
		act[0] = False

def DoContinue(act, cont):
	if act[0]:
		cont[0] = True

def DoSubOp(act, cont):
	if not TakeString("sub"):
		sub = [False]
		DoSubCall(act, cont, sub)
		if sub[0] == False:
			Error("Unknown function call")
	else:
		DoSubDef()

def DoSubDef():
	global pc, variable
	ident = TakeNextAlNum()
	if ident == "":
		Error("missing subroutine identifier")
	variable[ident] = ('p', pc)
	Block([False], [False])

def DoSubCall(act, cont, sub):
	global pc, variable
	prevpc = pc
	funident = TakeNextAlNum()
	if funident not in variable or variable[funident][0] != 'p':
		sub[0] = False
		pc = prevpc
		return
	sub[0] = True

	localvariable = variable.copy()

	# Set up function argument as variables
	while TakeNext(','):
		ident = TakeNextAlNum()
		if not TakeNext('=') or ident == "":
			Error("No variable name given")

		variable[ident] = Expression(act, cont)
	
	ret = pc
	pc = variable[funident][1]

	Block(act, cont)
	
	pc = ret
	variable = localvariable.copy()

def DoVarOp(act, cont):
	global variable
	if not TakeString("var"):
		DoAssign(act, cont)
	else:
		ident = TakeNextAlNum()
		e = ('i', 0)
		if ident == "":
			Error("not variable name")
		elif TakeNext('='):
			e = Expression(act, cont)

		if (act[0] and not cont[0]) or ident not in variable:
			variable[ident] = e
		
def DoAssign(act, cont):
	global variable
	ident = TakeNextAlNum()
	if not TakeNext('=') or ident == "":
		Error("uknown statement")
	e = Expression(act, cont)
	if (act[0] and not cont[0]) or ident not in variable:
		variable[ident] = e

def DoPrint(act, cont):
	while True:
		e = Expression(act, cont)
		if act[0] and not cont[0]:
			print(e[1], end="")
		if not TakeNext(','):
			return


main()