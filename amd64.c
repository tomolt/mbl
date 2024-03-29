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

RtLoc alloc_stack(EmitState * es)
{
	es->stackTop += 8;
	return (RtLoc) { LOC_STACK, es->stackTop };
}

void free_rtloc(EmitState * es, RtLoc loc)
{
	if (loc.kind == LOC_STACK) {
		assert(loc.number == es->stackTop);
		es->stackTop -= 8;
	}
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
	printf("\tret\n");
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
			free_rtloc(es, rhs);
			free_rtloc(es, lhs);
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

unsigned int emit_label(EmitState * es)
{
	unsigned int label = es->nextLabel++;
	printf(".l%08x:\n", label);
	return label;
}

void emit_branch(RtLoc cond, unsigned int label)
{
	emit2("mov", RAX, cond);
	emit2("cmp", RAX, (RtLoc) { LOC_LITERAL, 0 });
	printf("\tjne\t.l%08x\n", label);
}

void emit_jump(unsigned int label)
{
	printf("\tjmp\t.l%08x\n", label);
}

