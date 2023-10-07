#pragma once

typedef enum {
    Tok_Eof,
    Tok_Assign,
    Tok_Not,
    Tok_Not_Equal,
    Tok_Greater,
    Tok_Greater_Equal,
    Tok_Less,
    Tok_Less_Equal,
    Tok_At,
    Tok_Semicolon,
    Tok_Left_Paren,
    Tok_Right_Paren,
    Tok_Left_Bracket,
    Tok_Right_Bracket,
    Tok_Comma,
    Tok_Identifier,
    Tok_String,
    Tok_Number,
    Tok_And,
    Tok_If,
    Tok_Else,
    Tok_Function,
    Tok_Or,
    Tok_Print,
    Tok_Return,
    Tok_While,
    Tok_Add,
    Tok_Sub,
    Tok_Mult,
    Tok_Div,
    Tok_Let,
    Tok_Continue,
    Tok_Break,
    Tok_Comment,
    Tok_Is_Equal,
    Tok_Len,
    Tok_Exit,
    Tok_Modulo,
    Tok_Elif,
    Tok_Bit_And,
    Tok_Bit_Or,
    Tok_Bit_Xor,
    Tok_Bit_Not,
    Tok_Lshift,
    Tok_Rshift,
    Tok_Include,
} Token_Type;

char *find_tok_type(int type) {
    switch (type) {
        case Tok_Eof: return "Tok_Eof";
        case Tok_Assign: return "Tok_Assign";
        case Tok_Not: return "Tok_Not";
        case Tok_Not_Equal: return "Tok_Not_Equal";
        case Tok_Greater: return "Tok_Greater";
        case Tok_Greater_Equal: return "Tok_Greater_Equal";
        case Tok_Less: return "Tok_Less";
        case Tok_Less_Equal: return "Tok_Less_Equal";
        case Tok_At: return "Tok_At";
        case Tok_Semicolon: return "Tok_Semicolon";
        case Tok_Left_Paren: return "Tok_Left_Paren";
        case Tok_Right_Paren: return "Tok_Right_Paren";
        case Tok_Left_Bracket: return "Tok_Left_Bracket";
        case Tok_Right_Bracket: return "Tok_Right_Bracket";
        case Tok_Comma: return "Tok_Comma";
        case Tok_Identifier: return "Tok_Identifier";
        case Tok_String: return "Tok_String";
        case Tok_Number: return "Tok_Number";
        case Tok_And: return "Tok_And";
        case Tok_If: return "Tok_If";
        case Tok_Else: return "Tok_Else";
        case Tok_Function: return "Tok_Function";
        case Tok_Or: return "Tok_Or";
        case Tok_Print: return "Tok_Print";
        case Tok_Return: return "Tok_Return";
        case Tok_While: return "Tok_While";
        case Tok_Add: return "Tok_Add";
        case Tok_Sub: return "Tok_Sub";
        case Tok_Mult: return "Tok_Mult";
        case Tok_Div: return "Tok_Div";
        case Tok_Let: return "Tok_Let";
        case Tok_Continue: return "Tok_Continue";
        case Tok_Break: return "Tok_Break";
        case Tok_Comment: return "Tok_Comment";
        case Tok_Is_Equal: return "Tok_Is_Equal";
        case Tok_Len: return "Tok_Len";
        case Tok_Exit: return "Tok_Exit";
        case Tok_Modulo: return "Tok_Modulo";
        case Tok_Elif: return "Tok_Elif";
        case Tok_Bit_And: return "Tok_Bit_And";
        case Tok_Bit_Or: return "Tok_Bit_Or";
        case Tok_Bit_Xor: return "Tok_Bit_Xor";
        case Tok_Bit_Not: return "Tok_Bit_Not";
        case Tok_Lshift: return "Tok_Lshift";
        case Tok_Rshift: return "Tok_Rshift";
        case Tok_Include: return "Tok_Include";
        default: ERR("unknown token type `%d`\n", type)
    }
    return "unreachable";
}

typedef struct Lexer {
    const char *start;
    const char *current;
    int line;
} Lexer;

typedef struct Token {
    Token_Type type;
    const char *start;
    int length;
    int line;
    int jump_index;
} Token;

static Token make_token(Token_Type type, Lexer *l) {
    Token tok;
    tok.type = type;
    tok.start = l->start;
    tok.length = (int)(l->current - l->start);
    tok.line = l->line;
    tok.jump_index = 0;
    return tok;
}

static bool at_end(Lexer *l) {
    return *l->current == '\0';
}

static bool is_num(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alphabet(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static char peek(Lexer *l) {
    return *l->current;
}

static char advance(Lexer *l) {
    l->current++;
    return l->current[-1];
}

static bool match(char c, Lexer *l) {
    if (at_end(l)) {
        return false;
    }
    if (*l->current != c) return false;
    l->current++;
    return true;
}

static char peek_next(Lexer *l) {
    if (at_end(l)) return 0;
    return l->current[1];
}

static void skip_whitespace(Lexer *l) {
    while (1) {
        char c = peek(l);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(l);
                break;
            case '\n':
                l->line++;
                advance(l);
                break;
            default:
                return;
        }
    }
}

static Token lex_string(Lexer *l) {
    while (peek(l) != '"' && !at_end(l)) {
        if (peek(l) == '\n') {
            l->line++;
        }
        advance(l);
    }
    if (at_end(l)) {
        ERR("Unclosed string on line %d\n", l->line)
    }
    advance(l);
    return make_token(Tok_String, l);
}

static Token_Type check_keyword(int start, int length, const char *rest, Token_Type type, Lexer *l) {
    if (l->current - l->start == start + length && memcmp(l->start + start, rest, length) == 0) {
        return type;
    }
    return Tok_Identifier;
}

static Token_Type id_type(Lexer *l) {
    switch (l->start[0]) {
        case 'b':
            return check_keyword(1, 4, "reak", Tok_Break, l);
        case 'c':
            return check_keyword(1, 7, "ontinue", Tok_Continue, l);
        case 'e':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'x':
                        return check_keyword(2, 2, "it", Tok_Exit, l);
                    case 'l':
                        if (l->current - l->start > 2) {
                            switch (l->start[2]) {
                                case 's':
                                    return check_keyword(3, 1, "e", Tok_Else, l);
                                case 'i':
                                    return check_keyword(3, 1, "f", Tok_Elif, l);
                            }
                        }
                        return check_keyword(2, 2, "se", Tok_Else, l);
                }
            }
            break;
        case 'f':
            return check_keyword(1, 1, "n", Tok_Function, l);
        case 'i':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'f':
                        return Tok_If;
                    case 'n':
                    return check_keyword(2, 5, "clude", Tok_Include, l);
                }
            }
            break;
            return check_keyword(1, 1, "f", Tok_If, l);
        case 'l':
            if (l->current - l->start > 2) {
                switch (l->start[2]) {
                    case 'n':
                        return check_keyword(2, 1, "n", Tok_Len, l);
                    case 't':
                        return check_keyword(2, 1, "t", Tok_Let, l);
                }
            }
            break;
        case 'p':
            return check_keyword(1, 4, "rint", Tok_Print, l);
        case 'r':
            return check_keyword(1, 5, "eturn", Tok_Return, l);
        case 'w':
            return check_keyword(1, 4, "hile", Tok_While, l);

    }
    return Tok_Identifier;
}

static Token lex_number(Lexer *l) {
    while (is_num(peek(l))) advance(l);

    if (peek(l) == '.' && is_num(peek_next(l))) {
        advance(l);
        while (is_num(peek(l))) advance(l);
    }
    return make_token(Tok_Number, l);
}

static Token lex_identifier(Lexer *l) {
    while (is_alphabet(peek(l)) || is_num(peek(l))) advance(l);
    return make_token(id_type(l), l);
}

Token scan_token(Lexer *l) {
    //skip whitespace
    skip_whitespace(l);

    l->start = l->current;
    //at end?
    if (at_end(l)) {
        return make_token(Tok_Eof, l);
    }

    //lex token
    char c = advance(l);

    //token is number?
    if (is_num(c)) {
        return lex_number(l);
    }

    //token is alphabetical?
    if (is_alphabet(c)) {
        return lex_identifier(l);
    }
    switch (c) {
        case '#':
            while (!at_end(l) && *l->current != '\n') advance(l);
            return make_token(Tok_Comment, l);
        case '=':
            return make_token(match('=', l) ? Tok_Is_Equal : Tok_Assign, l);
        case '+':
            return make_token(Tok_Add, l);
        case '-':
            return make_token(Tok_Sub, l);
        case '*':
            return make_token(Tok_Mult, l);
        case '/':
            return make_token(Tok_Div, l);
        case '!':
            return make_token(match('=', l) ? Tok_Not_Equal : Tok_Not, l);
        case '>':
            c = advance(l);
            switch (c) {
                case '=':
                    return make_token(Tok_Greater_Equal, l);
                case '>':
                    return make_token(Tok_Rshift, l);
                default:
                    return make_token(Tok_Greater, l);
            }
        case '<':
            c = advance(l);
            switch (c) {
                case '=':
                    return make_token(Tok_Less_Equal, l);
                case '<':
                    return make_token(Tok_Lshift, l);
                default:
                    return make_token(Tok_Less, l);
            }
        case '"':
            return lex_string(l);
        case ';':
            return make_token(Tok_Semicolon, l);
        case '(':
            return make_token(Tok_Left_Paren, l);
        case ')':
            return make_token(Tok_Right_Paren, l);
        case '[':
            return make_token(Tok_Left_Bracket, l);
        case ']':
            return make_token(Tok_Right_Bracket, l);
        case ',':
            return make_token(Tok_Comma, l);
        case '@':
            return make_token(Tok_At, l);
        case '&':
            return make_token(match('&', l) ? Tok_And : Tok_Bit_And, l);
        case '|':
            return make_token(match('|', l) ? Tok_Or : Tok_Bit_Or, l);
        case '^':
            return make_token(Tok_Bit_Xor, l);
        case '~':
            return make_token(Tok_Bit_Not, l);
        case '%':
            return make_token(Tok_Modulo, l);
        default:
            ERR("ERROR: Unknown character on line %d:   %c\n", l->line, c)
    }
    ERR("Unknown character on line %d:   %c\n", l->line, c)
    exit(1);
}
