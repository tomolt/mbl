#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

#include "lexer.h"
#include "expr.h"
#include "target.h"
#include "parser.h"

#if 0
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
#endif

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
	EmitState es = { 0 };

	jmp_buf jmp;
	if (setjmp(jmp) == 0) {
		ls.recovery = &jmp;
		ls.nextChar = fgetc(ls.file);
		advance(&ls);
		parse_file(&ls, &es);
	}

	fclose(ls.file);
	return EXIT_SUCCESS;
}

