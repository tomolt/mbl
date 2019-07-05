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

struct State {
	FILE * file;
	char nextChar;
	TokenType token;
	unsigned long long tokenInteger;
	jmp_buf * recovery;
	int bad;
};

static struct State State;

static BinOp as_binop(void)
{
	switch (State.token) {
		case tk_op_mul: return op_mul;
		case tk_op_div: return op_div;
		case tk_op_mod: return op_mod;
		case tk_op_add: return op_add;
		case tk_op_sub: return op_sub;
		default: return not_a_binop;
	}
}

static void error(void)
{
	State.bad = 1;
	// longjmp(*State.recovery, 1);
	assert(0);
}

static void advance(void)
{
	while (isspace(State.nextChar)) {
		State.nextChar = fgetc(State.file);
	}
	char c = State.nextChar;
	if (isdigit(c)) {
		State.tokenInteger = 0;
		do {
			State.tokenInteger = State.tokenInteger * 10 + (State.nextChar - '0');
			State.nextChar = fgetc(State.file);
		} while (isdigit(State.nextChar));
		State.token = tk_integer;
	} else if (c == '+') {
		State.nextChar = fgetc(State.file);
		State.token = tk_op_add;
	} else if (c == '-') {
		State.nextChar = fgetc(State.file);
		State.token = tk_op_sub;
	} else if (c == '*') {
		State.nextChar = fgetc(State.file);
		State.token = tk_op_mul;
	} else if (c == '/') {
		State.nextChar = fgetc(State.file);
		State.token = tk_op_div;
	} else if (c == '%') {
		State.nextChar = fgetc(State.file);
		State.token = tk_op_mod;
	} else if (c == EOF) {
		/* Intentionally don't update nextChar */
		State.token = tk_end_of_file;
	} else {
		error();
	}
}

static void expect(TokenType type)
{
	if (State.token != type) {
		error();
	}
}

static Expr * parse_integer(void)
{
	expect(tk_integer);
	Expr * expr = malloc(sizeof(Expr));
	expr->integer = (IntegerExpr) { ex_integer, State.tokenInteger };
	advance();
	return expr;
}

static Expr * parse_expr(int maxPrecedence)
{
	Expr * lhs = parse_integer();
	for (;;) {
		BinOp op = as_binop();
		if (op == not_a_binop) return lhs;
		int prec = PrecedenceTable[op];
		if (prec > maxPrecedence) return lhs;
		advance();

		Expr * rhs = parse_expr(prec);
		Expr * expr = malloc(sizeof(Expr));
		expr->binop = (BinOpExpr) { ex_binop, op, lhs, rhs };
		lhs = expr;
	}
	return lhs;
}

static void parse_file(void)
{
	while (State.token != tk_end_of_file) {
		Expr * expr = parse_expr(100);
		printf("finished.\n");
	}
}

int main(int argc, char const * argv[])
{
	if (argc != 2) {
		printf("usage: %s <file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	State.file = fopen(argv[1], "r");
	if (State.file == NULL) {
		printf("can't open file '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	jmp_buf jmp;
	if (setjmp(jmp) == 0) {
		State.recovery = &jmp;
		State.nextChar = fgetc(State.file);
		advance();
		parse_file();
	}

	fclose(State.file);
	return EXIT_SUCCESS;
}

