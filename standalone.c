#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <setjmp.h>

typedef enum {
	op_mul,
	op_div,
	op_mod,

	op_add,
	op_sub,

#if 0
	op_logic_shift,
	op_arith_shift,
	op_left_shift,
	/* TODO bit rotate */

	op_less_than,
	op_less_equal,
	op_more_than,
	op_more_equal,
	op_equal,
	op_not_equal,

	op_bit_and,
	op_bit_or,
	op_bit_xor,

	op_logic_and,
	op_logic_or,
	op_logic_xor,
#endif

	not_a_binop
} BinOp;

int const PrecedenceTable[] = {
	[op_mul] = 0,
	[op_div] = 0,
	[op_mod] = 0,

	[op_add] = 1,
	[op_sub] = 1,
};

typedef enum {
	ex_integer,
	ex_binop,
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
	tk_end_of_file,
	tk_identifier,
	tk_integer,
	tk_op_mul,
	tk_op_div,
	tk_op_mod,
	tk_op_add,
	tk_op_sub,
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
		case tk_op_mul: return op_mul;
		case tk_op_div: return op_div;
		case tk_op_mod: return op_mod;
		case tk_op_add: return op_add;
		case tk_op_sub: return op_sub;
		default: return not_a_binop;
	}
}

static void error(LexState * ls)
{
	ls->bad = 1;
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
		ls->token = tk_integer;
	} else if (c == '+') {
		ls->nextChar = fgetc(ls->file);
		ls->token = tk_op_add;
	} else if (c == '-') {
		ls->nextChar = fgetc(ls->file);
		ls->token = tk_op_sub;
	} else if (c == '*') {
		ls->nextChar = fgetc(ls->file);
		ls->token = tk_op_mul;
	} else if (c == '/') {
		ls->nextChar = fgetc(ls->file);
		ls->token = tk_op_div;
	} else if (c == '%') {
		ls->nextChar = fgetc(ls->file);
		ls->token = tk_op_mod;
	} else if (c == EOF) {
		/* Intentionally don't update nextChar */
		ls->token = tk_end_of_file;
	} else {
		error(ls);
	}
}

static void expect(LexState * ls, TokenType type)
{
	if (ls->token != type) {
		error(ls);
	}
}

static Expr * parse_integer(LexState * ls)
{
	expect(ls, tk_integer);
	Expr * expr = malloc(sizeof(Expr));
	expr->integer = (IntegerExpr) { ex_integer, ls->tokenInteger };
	advance(ls);
	return expr;
}

static Expr * parse_expr(LexState * ls, int maxPrecedence)
{
	Expr * lhs = parse_integer(ls);
	for (;;) {
		BinOp op = as_binop(ls->token);
		if (op == not_a_binop) return lhs;
		int prec = PrecedenceTable[op];
		if (prec > maxPrecedence) return lhs;
		advance(ls);

		Expr * rhs = parse_expr(ls, prec);
		Expr * expr = malloc(sizeof(Expr));
		expr->binop = (BinOpExpr) { ex_binop, op, lhs, rhs };
		lhs = expr;
	}
	return lhs;
}

static void parse_file(LexState * ls)
{
	while (ls->token != tk_end_of_file) {
		Expr * expr = parse_expr(ls, 100);
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

