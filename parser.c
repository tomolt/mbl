#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "lexer.h"
#include "expr.h"
#include "parser.h"

Expr * parse_expr(LexState * ls);

static Expr * parse_integer(LexState * ls)
{
	expect(ls, TK_INTEGER);
	Expr * expr = malloc(sizeof(Expr));
	expr->integer = (IntegerExpr) { { EXPR_INTEGER, TYPE_UNKNOWN }, ls->tokenInteger };
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

static int const PrecedenceTable[] = {
	[BIN_MUL] = 0,
	[BIN_DIV] = 0,
	[BIN_MOD] = 0,

	[BIN_ADD] = 1,
	[BIN_SUB] = 1,
};

static Expr * parse_binary(LexState * ls, int maxPrecedence)
{
	Expr * lhs = parse_unary(ls);
	for (;;) {
		BinOp op = as_binop(ls->token);
		if (op == NOT_A_BINOP) return lhs;
		int prec = PrecedenceTable[op];
		if (prec >= maxPrecedence) return lhs;
		advance(ls);

		Expr * rhs = parse_binary(ls, prec);
		Expr * expr = malloc(sizeof(Expr));
		expr->binop = (BinOpExpr) { { EXPR_BINOP, TYPE_UNKNOWN }, op, lhs, rhs };
		lhs = expr;
	}
}

Expr * parse_expr(LexState * ls)
{
	return parse_binary(ls, 100);
}

