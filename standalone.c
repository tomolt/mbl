#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <setjmp.h>

typedef enum {
	BIN_MUL,
	BIN_DIV,
	BIN_MOD,

	BIN_ADD,
	BIN_SUB,

#if 0
	BIN_LOGIC_SHIFT,
	BIN_ARITH_SHIFT,
	BIN_LEFT_SHIFT,
	/* TODO bit rotate */

	BIN_LESS_THAN,
	BIN_LESS_EQUAL,
	BIN_MORE_THAN,
	BIN_MORE_EQUAL,
	BIN_EQUAL,
	BIN_NOT_EQUAL,

	BIN_BIT_AND,
	BIN_BIT_OR,
	BIN_BIT_XOR,

	BIN_LOGIC_AND,
	BIN_LOGIC_OR,
	BIN_LOGIC_XOR,
#endif

	NOT_A_BINOP
} BinOp;

int const PrecedenceTable[] = {
	[BIN_MUL] = 0,
	[BIN_DIV] = 0,
	[BIN_MOD] = 0,

	[BIN_ADD] = 1,
	[BIN_SUB] = 1,
};

typedef enum {
	EXPR_INTEGER,
	EXPR_BINOP,
} ExprType;

typedef union Expr Expr;

typedef struct {
	ExprType type;
	unsigned long long value;
} IntegerExpr;

typedef struct {
	ExprType type;
	BinOp op;
	Expr * lhs;
	Expr * rhs;
} BinOpExpr;

union Expr {
	ExprType type;
	IntegerExpr integer;
	BinOpExpr binop;
};

typedef enum {
	TK_END_OF_FILE,
	TK_IDENTIFIER,
	TK_INTEGER,
	TK_ASTERISK,
	TK_SLASH,
	TK_PERCENT,
	TK_PLUS,
	TK_MINUS,
	TK_LEFT_PAREN,
	TK_RIGHT_PAREN,
} TokenType;

typedef struct {
	FILE * file;
	char nextChar;
	TokenType token;
	unsigned long long tokenInteger;
	jmp_buf * recovery;
	int bad;
} LexState;

static BinOp as_binop(TokenType token)
{
	switch (token) {
		case TK_ASTERISK: return BIN_MUL;
		case TK_SLASH:    return BIN_DIV;
		case TK_PERCENT:  return BIN_MOD;
		case TK_PLUS:     return BIN_ADD;
		case TK_MINUS:    return BIN_SUB;
		default:          return NOT_A_BINOP;
	}
}

static void error(LexState * ls, char const * msg)
{
	ls->bad = 1;
	fprintf(stderr, "%s\n", msg);
	longjmp(*ls->recovery, 1);
}

static void advance(LexState * ls)
{
	while (isspace(ls->nextChar)) {
		ls->nextChar = fgetc(ls->file);
	}
	char c = ls->nextChar;
	if (isdigit(c)) {
		ls->tokenInteger = 0;
		do {
			ls->tokenInteger = ls->tokenInteger * 10 + (ls->nextChar - '0');
			ls->nextChar = fgetc(ls->file);
		} while (isdigit(ls->nextChar));
		ls->token = TK_INTEGER;
	} else if (c == '+') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_PLUS;
	} else if (c == '-') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_MINUS;
	} else if (c == '*') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_ASTERISK;
	} else if (c == '/') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_SLASH;
	} else if (c == '%') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_PERCENT;
	} else if (c == '(') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_LEFT_PAREN;
	} else if (c == ')') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_RIGHT_PAREN;
	} else if (c == EOF) {
		/* Intentionally don't update nextChar */
		ls->token = TK_END_OF_FILE;
	} else {
		error(ls, "invalid character");
	}
}

static void expect(LexState * ls, TokenType type)
{
	if (ls->token != type) {
		error(ls, "unexpected token");
	}
}

static Expr * parse_expr(LexState * ls);

static Expr * parse_integer(LexState * ls)
{
	expect(ls, TK_INTEGER);
	Expr * expr = malloc(sizeof(Expr));
	expr->integer = (IntegerExpr) { EXPR_INTEGER, ls->tokenInteger };
	advance(ls);
	return expr;
}

static Expr * parse_unary(LexState * ls)
{
	switch (ls->token) {
		case TK_INTEGER:
			return parse_integer(ls);
		case TK_LEFT_PAREN:
			advance(ls);
			Expr * expr = parse_expr(ls);
			expect(ls, TK_RIGHT_PAREN);
			advance(ls);
			return expr;
		default:
			error(ls, "invalid expression");
			return NULL;
	}
}

static Expr * parse_binary(LexState * ls, int maxPrecedence)
{
	Expr * lhs = parse_unary(ls);
	for (;;) {
		BinOp op = as_binop(ls->token);
		if (op == NOT_A_BINOP) return lhs;
		int prec = PrecedenceTable[op];
		if (prec > maxPrecedence) return lhs;
		advance(ls);

		Expr * rhs = parse_expr(ls);
		Expr * expr = malloc(sizeof(Expr));
		expr->binop = (BinOpExpr) { EXPR_BINOP, op, lhs, rhs };
		lhs = expr;
	}
	return lhs;
}

static Expr * parse_expr(LexState * ls)
{
	return parse_binary(ls, 100);
}

static void parse_file(LexState * ls)
{
	while (ls->token != TK_END_OF_FILE) {
		Expr * expr = parse_expr(ls);
		(void) expr;
		printf("finished.\n");
	}
}

int main(int argc, char const * argv[])
{
	if (argc != 2) {
		printf("usage: %s <file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	LexState ls = { 0 };
	ls.file = fopen(argv[1], "r");
	if (ls.file == NULL) {
		printf("can't open file '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	jmp_buf jmp;
	if (setjmp(jmp) == 0) {
		ls.recovery = &jmp;
		ls.nextChar = fgetc(ls.file);
		advance(&ls);
		parse_file(&ls);
	}

	fclose(ls.file);
	return EXIT_SUCCESS;
}

