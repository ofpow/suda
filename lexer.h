#pragma once

void shunt(char* dest, char* src) {
  while (*dest) {
    *dest = *src;
    ++dest;
    ++src;
  }
}

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
    Tok_Power,
    Tok_Append,
    Tok_Cast_Str,
    Tok_Cast_Num,
    Tok_Println,
    Tok_For,
    Tok_In,
    Tok_Input,
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
        case Tok_Power: return "Tok_Power";
        case Tok_Append: return "Tok_Append";
        case Tok_Cast_Str: return "Tok_Cast_Str";
        case Tok_Cast_Num: return "Tok_Cast_Num";
        case Tok_Println: return "Tok_Println";
        case Tok_For: return "Tok_For";
        case Tok_In: return "Tok_In";
        case Tok_Input: return "Tok_Input";
        default: ERR("unknown token type `%d`\n", type)
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

Token *lex_file(const char *file_path, Programs *programs) {
    debug("LEX FILE %s\n", file_path)
    char **include_paths = calloc(2, sizeof(char*));
    int64_t include_paths_index = 0;
    int64_t include_paths_capacity = 2;

    char *program = read_file(file_path);
    Lexer lexer = { program, program, 1, file_path };
    int64_t tokens_index = 0;
    int64_t tokens_capacity = 10;

    Token *tokens = calloc(tokens_capacity, sizeof(struct Token));

    while (1) {
        if (tokens_index >= tokens_capacity) {
            tokens_capacity *= 2;
            tokens = realloc(tokens, tokens_capacity * sizeof(struct Token));
        }
        Token tok = scan_token(&lexer);
        if (tok.type == Tok_Eof) {
            tokens[tokens_index] = tok;
            tokens_index++;
            break;
        } else if (tok.type == Tok_Comment) {
        } else if (tok.type == Tok_Include) {
            tok = scan_token(&lexer);
            char *include_path = format_str(tok.length - 1, "%.*s", tok.length, tok.start + 1);
            int64_t included = 0;
            for (int i = 0; i < include_paths_index; i++) {
                if (!strcmp(include_path, include_paths[i])) {
                    included = 1;
                }
            }

            if (included) {
                free(include_path);
                continue;
            }

            append(include_paths, include_path, include_paths_index, include_paths_capacity);

            int64_t new_tokens_index = 0;
            int64_t new_tokens_capacity = 10;

            Token *new_tokens = calloc(new_tokens_capacity, sizeof(struct Token));
            Token *to_include = lex_file(include_path, programs);

            for (int i = 0; to_include[i].type != Tok_Eof; i++) {
                append(new_tokens, to_include[i], new_tokens_index, new_tokens_capacity)
            }
            for (int i = 0; i < tokens_index; i++) {
                append(new_tokens, tokens[i], new_tokens_index, new_tokens_capacity)
            }

            free(to_include);
            free(tokens);

            tokens = new_tokens;
            tokens_index = new_tokens_index;
            tokens_capacity = new_tokens_capacity;
        } else {
            tokens[tokens_index] = tok;
            tokens_index++;
        }
        debug("TOKEN ( `%s` | '%.*s' )\n", find_tok_type(tok.type), (int)tok.length, tok.start)
    } 
    for (int i = 0; i < include_paths_index; i++) free(include_paths[i]);
    free(include_paths);

    append(programs->progs, program, programs->progs_index, programs->progs_capacity)

    return tokens;
}
