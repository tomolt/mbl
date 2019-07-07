#ifdef MBL_EXPR_H
#error multiple inclusion
#endif
#define MBL_EXPR_H

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

typedef enum {
	TYPE_UNKNOWN,
	TYPE_INTEGER,
} Type;

typedef enum {
	EXPR_INTEGER,
	EXPR_BINOP,
} ExprKind;

typedef struct {
	ExprKind kind;
	Type type;
} ExprInfo;

typedef union Expr Expr;

typedef struct {
	ExprInfo info;
	unsigned long long value;
} IntegerExpr;

typedef struct {
	ExprInfo info;
	BinOp op;
	Expr * lhs;
	Expr * rhs;
} BinOpExpr;

union Expr {
	ExprInfo info;
	IntegerExpr integer;
	BinOpExpr binop;
};

