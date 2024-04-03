#pragma once

typedef struct string {
    char *chars;
    size_t len;
} string;

#define Print(_s) (int)_s.len, _s.chars
