typedef enum {
    Eof,
    Equal,
    Bang,
    Bang_Equal,
    Greater,
    Greater_Equal,
    Less,
    Less_Equal,
    At,
    Semicolon,
    Left_Paren,
    Right_Paren,
    Identifier,
    String,
    Number,
    And,
    If,
    Else,
    Func,
    Or,
    Print,
    Return,
    While,
    Add,
    Sub,
    Mult,
    Div,
    Num,
    Str,
    Arr,
    Set,
    Continue,
    Break,
    Comment,
} Token_Type;

typedef struct Lexer {
    const char *start;
    const char *current;
    int line;
} Lexer;

typedef struct Token {
    int type;
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

static Token parse_string(Lexer *l) {
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
    return make_token(String, l);
}

static Token_Type check_keyword(int start, int length, const char *rest, Token_Type type, Lexer *l) {
    if (l->current - l->start == start + length && memcmp(l->start + start, rest, length) == 0) {
        return type;
    }
    return Identifier;
}

static Token_Type id_type(Lexer *l) {
    switch (l->start[0]) {
        case 'a':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'd':
                        return check_keyword(2, 1, "d", Add, l);
                    case 'n':
                        return check_keyword(2, 1, "d", And, l);
                    case 'r':
                        return check_keyword(2, 1, "r", Arr, l);
                }
            }
            break;
        case 'b':
            return check_keyword(1, 4, "reak", Break, l);
        case 'c':
            return check_keyword(1, 3, "ont", Continue, l);
        case 'd':
            return check_keyword(1, 2, "iv", Div, l);
        case 'e':
            return check_keyword(1, 3, "lse", Else, l);
        case 'f':
            return check_keyword(1, 3, "unc", Func, l);
        case 'i':
            return check_keyword(1, 1, "f", If, l);
        case 'm':
            return check_keyword(1, 3, "ult", Mult, l);
        case 'n':
            return check_keyword(1, 2, "um", Num, l);
        case 'o':
            return check_keyword(1, 1, "r", Or, l);
        case 'p':
            return check_keyword(1, 4, "rint", Print, l);
        case 'r':
            return check_keyword(1, 5, "eturn", Return, l);
        case 's':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'u':
                        return check_keyword(2, 1, "b", Sub, l);
                    case 'e':
                        return check_keyword(2, 1, "t", Set, l);
                    case 't':
                        return check_keyword(2, 1, "r", Str, l);
                }
            }
            break;
        case 'w':
            return check_keyword(1, 4, "hile", While, l);

    }
    return Identifier;
}

static Token parse_number(Lexer *l) {
    while (is_num(peek(l))) advance(l);

    if (peek(l) == '.' && is_num(peek_next(l))) {
        advance(l);
        while (is_num(peek(l))) advance(l);
    }
    return make_token(Number, l);
}

static Token parse_identifier(Lexer *l) {
    while (is_alphabet(peek(l)) || is_num(peek(l))) advance(l);
    return make_token(id_type(l), l);
}

Token scan_token(Lexer *l) {
    //skip whitespace
    skip_whitespace(l);

    l->start = l->current;
    //at end?
    if (at_end(l)) {
        return make_token(Eof, l);
    }

    //parse token
    char c = advance(l);

    //token is number?
    if (is_num(c)) {
        return parse_number(l);
    }

    //token is alphabetical?
    if (is_alphabet(c)) {
        return parse_identifier(l);
    }
    switch (c) {
        case '#':
            while (!at_end(l) && *l->current != '\n') advance(l);
            return make_token(Comment, l);
        case '=':
            return make_token(Equal, l);
        case '!':
            return make_token(match('=', l) ? Bang_Equal : Bang, l);
        case '>':
            return make_token(match('=', l) ? Greater_Equal : Greater, l);
        case '<':
            return make_token(match('=', l) ? Less_Equal : Less, l);
        case '"':
            return parse_string(l);
        case ';':
            return make_token(Semicolon, l);
        case '(':
            return make_token(Left_Paren, l);
        case ')':
            return make_token(Right_Paren, l);
        default:
            ERR("ERROR: Unknown character on line %d:   %c\n", l->line, c);
    }
    ERR("Unknown character on line %d:   %c\n", l->line, c);
    exit(1);
}
//print out tokens
//if (tok.line != line) {
//    printf("%4d ", tok.line);
//    line = tok.line;
//} else {
//    printf("   | ");
//}
//printf("%2d '%.*s' %d\n", tok.type, tok.length, tok.start, tok.jump_index);
