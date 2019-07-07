/* depends on lexer.h */
/* depends on expr.h */

#ifdef MBL_PARSER_H
#error multiple inclusion
#endif
#define MBL_PARSER_H

Expr * parse_expr(LexState * ls);

