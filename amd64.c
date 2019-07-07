#include <stdio.h>
#include <assert.h>

#include "expr.h"
#include "target.h"

static char const * RegisterNames[] = {
	"rax", "rcx", "rdx", "rbx", "rsi", "rdi", "rsp", "rbp",
};

static void emit_rtloc(RtLoc loc)
{
	switch (loc.kind) {
		case LOC_STACK:
			printf("[rbp - %llu]", loc.number);
			break;
		case LOC_REGISTER:
			printf("%s", RegisterNames[loc.number]);
			break;
		case LOC_LITERAL:
			printf("%llu", loc.number);
			break;
	}
}

static RtLoc push_stack(EmitState * es)
{
	es->stackTop += 8;
	return (RtLoc) { LOC_STACK, es->stackTop };
}

static void pop_stack(EmitState * es)
{
	es->stackTop -= 8;
}

void emit1(char const *mnem, RtLoc arg)
{
	printf("\t%s\t", mnem);
	emit_rtloc(arg);
	printf("\n");
}

void emit2(char const *mnem, RtLoc dst, RtLoc src)
{
	printf("\t%s\t", mnem);
	emit_rtloc(dst);
	printf(",\t");
	emit_rtloc(src);
	printf("\n");
}

void emit_preamble(char const *name)
{
	printf("%s:\n", name);
	emit1("push", RBP);
	emit2("mov", RBP, RSP);
}

void emit_postamble(void)
{
	emit2("mov", RSP, RBP);
	emit1("pop", RBP);
}

RtLoc emit_expr(EmitState * es, Expr * expr)
{
	RtLoc loc, lhs, rhs;
	switch (expr->info.kind) {
		case EXPR_INTEGER:
			return (RtLoc) { LOC_LITERAL, expr->integer.value };
		case EXPR_BINOP:
			lhs = emit_expr(es, expr->binop.lhs);
			rhs = emit_expr(es, expr->binop.rhs);
			if (rhs.kind == LOC_STACK) {
				pop_stack(es);
			}
			if (lhs.kind == LOC_STACK) {
				pop_stack(es);
			}
			loc = push_stack(es);
			switch (expr->binop.op) {
				case BIN_ADD:
					emit2("mov", RAX, lhs);
					emit2("add", RAX, rhs);
					emit2("mov", loc, RAX);
					break;
				case BIN_SUB:
					emit2("mov", RAX, lhs);
					emit2("sub", RAX, rhs);
					emit2("mov", loc, RAX);
					break;
				case BIN_MUL:
					emit2("mov", RAX, lhs);
					emit2("imul", RAX, rhs);
					emit2("mov", loc, RAX);
					break;
				case BIN_DIV:
					emit2("mov", RAX, lhs);
					emit2("xor", RDX, RDX);
					emit1("div", rhs);
					emit2("mov", loc, RAX);
					break;
				case BIN_MOD:
					emit2("mov", RAX, lhs);
					emit2("xor", RDX, RDX);
					emit1("div", rhs);
					emit2("mov", loc, RDX);
					break;
				default:
					assert(0);
					break;
			}
			return loc;
	}
}

