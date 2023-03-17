#pragma once

typedef enum {
    Tok_Eof,
    Tok_Equal,
    Tok_Bang,
    Tok_Bang_Equal,
    Tok_Greater,
    Tok_Greater_Equal,
    Tok_Less,
    Tok_Less_Equal,
    Tok_At,
    Tok_Semicolon,
    Tok_Left_Paren,
    Tok_Right_Paren,
    Tok_Identifier,
    //tokens for literals
    Tok_String,
    Tok_Number,
    Tok_And,
    Tok_If,
    Tok_Else,
    Tok_Func,
    Tok_Or,
    Tok_Print,
    Tok_Return,
    Tok_While,
    Tok_Add,
    Tok_Sub,
    Tok_Mult,
    Tok_Div,
    //tokens for variable type
    Tok_Num,
    Tok_Str,
    Tok_Arr,
    Tok_Set,
    Tok_Continue,
    Tok_Break,
    Tok_Comment,
} Token_Type;

char *find_tok_type(int type) {
    switch (type) {
        case Tok_Eof: return "Tok_Eof";
        case Tok_Equal: return "Tok_Equal";
        case Tok_Bang: return "Tok_Bang";
        case Tok_Bang_Equal: return "Tok_Bang_Equal";
        case Tok_Greater: return "Tok_Greater";
        case Tok_Greater_Equal: return "Tok_Greater_Equal";
        case Tok_Less: return "Tok_Less";
        case Tok_Less_Equal: return "Tok_Less_Equal";
        case Tok_At: return "Tok_At";
        case Tok_Semicolon: return "Tok_Semicolon";
        case Tok_Left_Paren: return "Tok_Left_Paren";
        case Tok_Right_Paren: return "Tok_Right_Paren";
        case Tok_Identifier: return "Tok_Identifier";
        case Tok_String: return "Tok_String";
        case Tok_Number: return "Tok_Number";
        case Tok_And: return "Tok_And";
        case Tok_If: return "Tok_If";
        case Tok_Else: return "Tok_Else";
        case Tok_Func: return "Tok_Func";
        case Tok_Or: return "Tok_Or";
        case Tok_Print: return "Tok_Print";
        case Tok_Return: return "Tok_Return";
        case Tok_While: return "Tok_While";
        case Tok_Add: return "Tok_Add";
        case Tok_Sub: return "Tok_Sub";
        case Tok_Mult: return "Tok_Mult";
        case Tok_Div: return "Tok_Div";
        case Tok_Num: return "Tok_Num";
        case Tok_Str: return "Tok_Str";
        case Tok_Arr: return "Tok_Arr";
        case Tok_Set:return "Tok_Set";
        case Tok_Continue: return "Tok_Continue";
        case Tok_Break: return "Tok_Break";
        case Tok_Comment: return "Tok_Comment";
        default: ERR("unreachable\n");
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
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '@';
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
        ERR("Unclosed string on line %d\n", l->line);
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
        case 'a':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'n':
                        return check_keyword(2, 1, "d", Tok_And, l);
                    case 'r':
                        return check_keyword(2, 1, "r", Tok_Arr, l);
                }
            }
            break;
        case 'b':
            return check_keyword(1, 4, "reak", Tok_Break, l);
        case 'c':
            return check_keyword(1, 3, "ont", Tok_Continue, l);
        case 'e':
            return check_keyword(1, 3, "lse", Tok_Else, l);
        case 'f':
            return check_keyword(1, 3, "unc", Tok_Func, l);
        case 'i':
            return check_keyword(1, 1, "f", Tok_If, l);
        case 'n':
            return check_keyword(1, 2, "um", Tok_Num, l);
        case 'o':
            return check_keyword(1, 1, "r", Tok_Or, l);
        case 'p':
            return check_keyword(1, 4, "rint", Tok_Print, l);
        case 'r':
            return check_keyword(1, 5, "eturn", Tok_Return, l);
        case 's':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'e':
                        return check_keyword(2, 1, "t", Tok_Set, l);
                    case 't':
                        return check_keyword(2, 1, "r", Tok_Str, l);
                }
            }
            break;
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
            return make_token(Tok_Equal, l);
        case '+':
            return make_token(Tok_Add, l);
        case '-':
            return make_token(Tok_Sub, l);
        case '*':
            return make_token(Tok_Mult, l);
        case '/':
            return make_token(Tok_Div, l);
        case '!':
            return make_token(match('=', l) ? Tok_Bang_Equal : Tok_Bang, l);
        case '>':
            return make_token(match('=', l) ? Tok_Greater_Equal : Tok_Greater, l);
        case '<':
            return make_token(match('=', l) ? Tok_Less_Equal : Tok_Less, l);
        case '"':
            return lex_string(l);
        case ';':
            return make_token(Tok_Semicolon, l);
        case '(':
            return make_token(Tok_Left_Paren, l);
        case ')':
            return make_token(Tok_Right_Paren, l);
        default:
            ERR("ERROR: Unknown character on line %d:   %c\n", l->line, c);
    }
    ERR("Unknown character on line %d:   %c\n", l->line, c);
    exit(1);
}