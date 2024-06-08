typedef struct Location {
    const char *file;
    int64_t line;
} Location;

define_array(Locations, Location);

Value add1(Value *args, Location loc) {
    ASSERT(args[0].type == Value_Number, "ERROR in %s on line %ld: cant add 1 to type %s\n", loc.file, loc.line, find_value_type(args[0].type))
    return (Value){
        Value_Number,
        .val.num=args[0].val.num + 1,
        true,
        0
    };
}

Value input() {
    int input_capacity = 10;
    int input_index = 0;
    char *input = calloc(input_capacity, sizeof(char));
    char c;
    while ((c = fgetc(stdin))) {
        if (c == '\n') break;
        append_verbose(input, c, input_index, input_capacity)
    }

    return (Value){
        Value_String,
        .val.str={input, input_index},
        true,
        0
    };
}

typedef Value (*Native)(Value*, Location);

#define NATIVES \
    X(add1, 1)\
    X(input, 0)\

#define X(name, arity) name,
Native natives[] = {
    NATIVES
};
#undef X

#define X(name, arity) #name,
char* native_names[] = {
    NATIVES
};
#undef X

#define X(name, arity) arity,
int native_arities[] = {
    NATIVES
};
#undef X


int is_native(char *func) {
    for (u_int32_t i = 0; i < (sizeof(native_names) / 8); i++)
        if (!strcmp(func, native_names[i]))
            return i;
    return -1;
}
