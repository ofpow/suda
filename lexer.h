#pragma once

void shunt(char* dest, char* src) {
  while (*dest) {
    *dest = *src;
    ++dest;
    ++src;
  }
}

#define toks\
    X(Tok_Eof)\
    X(Tok_Assign)\
    X(Tok_Not)\
    X(Tok_Not_Equal)\
    X(Tok_Greater)\
    X(Tok_Greater_Equal)\
    X(Tok_Less)\
    X(Tok_Less_Equal)\
    X(Tok_At)\
    X(Tok_Semicolon)\
    X(Tok_Left_Paren)\
    X(Tok_Right_Paren)\
    X(Tok_Left_Bracket)\
    X(Tok_Right_Bracket)\
    X(Tok_Comma)\
    X(Tok_Identifier)\
    X(Tok_String)\
    X(Tok_Number)\
    X(Tok_And)\
    X(Tok_If)\
    X(Tok_Else)\
    X(Tok_Function)\
    X(Tok_Or)\
    X(Tok_Print)\
    X(Tok_Return)\
    X(Tok_While)\
    X(Tok_Add)\
    X(Tok_Sub)\
    X(Tok_Mult)\
    X(Tok_Div)\
    X(Tok_Let)\
    X(Tok_Continue)\
    X(Tok_Break)\
    X(Tok_Comment)\
    X(Tok_Is_Equal)\
    X(Tok_Len)\
    X(Tok_Exit)\
    X(Tok_Modulo)\
    X(Tok_Elif)\
    X(Tok_Bit_And)\
    X(Tok_Bit_Or)\
    X(Tok_Bit_Xor)\
    X(Tok_Bit_Not)\
    X(Tok_Lshift)\
    X(Tok_Rshift)\
    X(Tok_Include)\
    X(Tok_Power)\
    X(Tok_Append)\
    X(Tok_Cast_Str)\
    X(Tok_Cast_Num)\
    X(Tok_Println)\
    X(Tok_For)\
    X(Tok_In)\
    X(Tok_Input)\
    X(Tok_True)\
    X(Tok_False)\

typedef enum {
#define X(x) x,
    toks
#undef X
} Token_Type;

char *find_tok_type(int type) {
    switch (type) {
#define X(x) case x: return #x;
    toks
#undef X
    }
    return "unreachable";
}

typedef struct Lexer {
    char *start;
    char *current;
    int64_t line;
    const char *file;
} Lexer;

typedef struct Token {
    Token_Type type;
    const char *start;
    int64_t length;
    int64_t line;
    const char *file;
} Token;

define_array(Token_Array, Token);

static Token make_token(Token_Type type, Lexer *l) {
    Token tok;
    tok.type = type;
    tok.start = l->start;
    tok.length = (int)(l->current - l->start);
    tok.line = l->line;
    tok.file = l->file;
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
    int64_t i = 0;
    while (peek(l) != '"' && !at_end(l)) {
        if (peek(l) == '\n') {
            l->line++;
        } else if (peek(l) == '\\') {
            i++;
            switch (*(l->current + 1)) {
                case 'a': l->start[i] = '\a'; break;
                case 'b': l->start[i] = '\b'; break;
                case 'f': l->start[i] = '\f'; break;
                case 'n': l->start[i] = '\n'; break;
                case 'r': l->start[i] = '\r'; break;
                case 't': l->start[i] = '\t'; break;
                case 'v': l->start[i] = '\v'; break;
                case '\\': l->start[i] = '\\'; break;
                case '\'': l->start[i] = '\''; break;
                case '"': l->start[i] = '\"'; break;
                case '?': l->start[i] = '\?'; break;
            }
            // TODO: this is really slow, 
            // maybe introduce seperate keyword for printing with newline
            shunt(&l->start[i + 1], &l->start[i + 2]);
            i--;
        }
        advance(l);
        i++;
    }
    if (at_end(l)) {
        ERR("Unclosed string on line %ld\n", l->line)
    }
    advance(l);
    return make_token(Tok_String, l);
}

static Token_Type check_keyword(int start, int64_t length, const char *rest, Token_Type type, Lexer *l) {
    if (l->current - l->start == start + length && memcmp(l->start + start, rest, length) == 0) {
        return type;
    }
    return Tok_Identifier;
}

static Token_Type id_type(Lexer *l) {
    switch (l->start[0]) {
        case 'a':
            return check_keyword(1, 5, "ppend", Tok_Append, l);
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
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'o':
                        return check_keyword(2, 1, "r", Tok_For, l);
                    case 'a':
                        return check_keyword(2, 3, "lse", Tok_False, l);
                    case 'n':
                        return Tok_Function;
                }
            }
            break;
        case 'i':
            if (l->current - l->start > 1) {
                switch (l->start[1]) {
                    case 'f':
                        return Tok_If;
                    case 'n':
                        if (l->start[2] == 'c')
                            return check_keyword(3, 4, "lude", Tok_Include, l);
                        if (l->start[2] == 'p')
                            return check_keyword(3, 2, "ut", Tok_Input, l);
                        else if (l->start[2] == ' ')
                            return Tok_In;
                }
            }
            break;
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
        case 'n':
            return check_keyword(1, 2, "um", Tok_Cast_Num, l);
        case 'p':
            if (l->start[1] == 'r' && l->start[2] == 'i' && l->start[3] == 'n' && l->start[4] == 't' && l->start[5] == 'l' && l->start[6] == 'n')
                return Tok_Println;
            return check_keyword(1, 4, "rint", Tok_Print, l);
        case 'r':
            return check_keyword(1, 5, "eturn", Tok_Return, l);
        case 's':
            return check_keyword(1, 2, "tr", Tok_Cast_Str, l);
        case 't':
            return check_keyword(1, 3, "rue", Tok_True, l);
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
            return make_token(match('*', l) ? Tok_Power : Tok_Mult, l);
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
            ERR("ERROR: Unknown character on line %ld:   %c\n", l->line, c)
    }
    ERR("Unknown character on line %ld:   %c\n", l->line, c)
    exit(1);
}

Token *lex_file(const char *file_path, String_Array *programs, String_Array *include_paths) {
    debug("LEX FILE %s\n", file_path)

    char *program = read_file(file_path);
    Lexer lexer = { program, program, 1, file_path };

    Token_Array tokens = {
        calloc(10, sizeof(Token)),
        0,
        10
    };

    while (1) {
        Token tok = scan_token(&lexer);
        if (tok.type == Tok_Eof) {
            append(tokens, tok);
            break;
        } else if (tok.type == Tok_Comment) {
        } else if (tok.type == Tok_Include) {
            tok = scan_token(&lexer);
            char *include_path = format_str(tok.length - 1, "%.*s", tok.length, tok.start + 1);
            bool included = false;
            for (int i = 0; i < include_paths->index; i++) {
                if (!strcmp(include_path, include_paths->data[i])) {
                    included = true;
                }
            }

            if (included) {
                free(include_path);
                continue;
            }

            append((*include_paths), include_path);

            Token_Array new_tokens = {
                calloc(10, sizeof(Token)),
                0, 
                10,
            };

            Token *to_include = lex_file(include_path, programs, include_paths);

            for (int i = 0; to_include[i].type != Tok_Eof; i++) {
                append(new_tokens, to_include[i]);
            }
            for (int i = 0; i < tokens.index; i++) {
                append(new_tokens, tokens.data[i]);
            }

            free(to_include);
            free(tokens.data);

            tokens.data = new_tokens.data;
            tokens.index = new_tokens.index;
            tokens.capacity = new_tokens.capacity;
        } else {
            append(tokens, tok);
        }
        debug("TOKEN ( `%s` | '%.*s' )\n", find_tok_type(tok.type), (int)tok.length, tok.start)
    } 

    append((*programs), program);

    return tokens.data;
}
