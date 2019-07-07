#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

#include "lexer.h"
#include "expr.h"
#include "parser.h"
#include "target.h"

static Type check_expr(LexState * ls, Expr * expr)
{
	Type lhs, rhs;
	switch (expr->info.kind) {
		case EXPR_INTEGER:
			return expr->info.type = TYPE_INTEGER;
		case EXPR_BINOP:
			lhs = check_expr(ls, expr->binop.lhs);
			rhs = check_expr(ls, expr->binop.rhs);
			if (lhs != rhs) {
				error(ls, "type mismatch");
			}
			return expr->info.type = lhs;
	}
}

static void handle_expr(Expr * expr)
{
	EmitState es = { 0 };
	RtLoc loc = emit_expr(&es, expr);
	emit2("mov", RAX, loc);
	printf("\tcall\twrite_integer\n");
}

static void parse_file(LexState * ls)
{
	printf("bits 64\n");
	printf("section .text\n");
	printf("extern write_integer\n");
	printf("extern exit\n");
	printf("global _start\n");
	emit_preamble("_start");
	printf("\tsub\trsp,\t100\n");
	while (ls->token != TK_END_OF_FILE) {
		Expr * expr = parse_expr(ls);
		check_expr(ls, expr);
		handle_expr(expr);
	}
	printf("\tcall\texit\n");
	emit_postamble();
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

