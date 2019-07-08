/* depends on expr.h */

#ifdef MBL_TARGET_H
#error multiple inclusion
#endif
#define MBL_TARGET_H

typedef enum {
	LOC_STACK,
	LOC_REGISTER,
	LOC_LITERAL,
} RtLocKind;

typedef struct {
	RtLocKind kind;
	unsigned long long number;
} RtLoc;

typedef struct {
	unsigned int stackTop;
	unsigned int nextLabel;
} EmitState;

/* TODO correct integer values */
#define RAX ((RtLoc) { LOC_REGISTER, 0 })
#define RCX ((RtLoc) { LOC_REGISTER, 1 })
#define RDX ((RtLoc) { LOC_REGISTER, 2 })
#define RBX ((RtLoc) { LOC_REGISTER, 3 })
#define RSI ((RtLoc) { LOC_REGISTER, 4 })
#define RDI ((RtLoc) { LOC_REGISTER, 5 })
#define RSP ((RtLoc) { LOC_REGISTER, 6 })
#define RBP ((RtLoc) { LOC_REGISTER, 7 })

RtLoc alloc_stack(EmitState * es);
void free_rtloc(EmitState * es, RtLoc loc);
void emit1(char const *mnem, RtLoc arg);
void emit2(char const *mnem, RtLoc dst, RtLoc src);
void emit_preamble(char const *name);
void emit_postamble(void);
RtLoc emit_expr(EmitState * es, Expr * expr);
unsigned int emit_label(EmitState * es);
void emit_branch(RtLoc cond, unsigned int label);
void emit_jump(unsigned int label);
