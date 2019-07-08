/* depends on stdio.h */
/* depends on setjmp.h */

#ifdef MBL_LEXER_H
#error multiple inclusion
#endif
#define MBL_LEXER_H

typedef enum {
	TK_END_OF_FILE,
	TK_IDENTIFIER,
	TK_INTEGER,
	TK_ASTERISK,
	TK_SLASH,
	TK_PERCENT,
	TK_PLUS,
	TK_MINUS,
	TK_SEMICOLON,
	TK_LEFT_PAREN,
	TK_RIGHT_PAREN,
	TK_LEFT_BRACE,
	TK_RIGHT_BRACE,
	TK_KEYWORD_IF,
	TK_KEYWORD_ELSE,
} TokenType;

typedef struct {
	FILE * file;
	char nextChar;
	TokenType token;
	unsigned long long tokenInteger;
	jmp_buf * recovery;
	int bad;
} LexState;

void error(LexState * ls, char const * msg);
void advance(LexState * ls);
void expect(LexState * ls, TokenType type);

