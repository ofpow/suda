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

Value clock_native() {
    return (Value){
        Value_Number,
        .val.num=clock() / 1000,
        false,
        0
    };
}

Value len(Value *args, Location loc) {
    if (args[0].type == Value_Array) {
        return (Value){
            Value_Number,
            .val.num=ARRAY_LEN(args[0].val.array[0].val.num) - 1,
            false,
            0
        };
    } else if (args[0].type == Value_String) {
        return (Value){
            Value_Number,
            .val.num=args[0].val.str.len,
            false,
            0
        };
    } else ERR("ERROR in %s on line %ld: cant do len of %s\n", loc.file, loc.line, find_value_type(args[0].type))

    return (Value) {0};
}

Value exit_native(Value *args, Location loc) {
    free_mem(args[0].val.num);

    printf("%s:%ld\n", loc.file, loc.line);

    return (Value) {0};
}

Value rand_native() {
    return (Value){
        Value_Number,
        .val.num=rand(),
        false,
        0
    };
}

Value pop(Value *args, Location loc) {
    if (args[0].type == Value_Array) {
        int arr_len = ARRAY_LEN(args[0].val.array[0].val.num);
        int arr_size = ARRAY_SIZE(args[0].val.array[0].val.num);

        Value val = args[0].val.array[arr_len - 1];

        args[0].val.array[arr_len - 1] = (Value) {0};
        args[0].val.array[0].val.num = MAKE_ARRAY_INFO(arr_size, arr_len - 1);

        return val;
    } else ERR("ERROR in %s on line %ld: cant pop from type %s\n", loc.file, loc.line, find_value_type(args[0].type))
    return (Value) {0};
}

typedef Value (*Native)(Value*, Location);

//    c function name     suda function name     number of arguments
#define NATIVES \
    X(add1,         add1,  1)\
    X(input,        input, 0)\
    X(clock_native, clock, 0)\
    X(len,          len,   1)\
    X(exit_native,  exit,  1)\
    X(rand_native,  rand,  0)\
    X(pop,          pop,   1)\

#define X(func, name, arity) func,
Native natives[] = {
    NATIVES
};
#undef X

#define X(func, name, arity) #name,
char* native_names[] = {
    NATIVES
};
#undef X

#define X(func, name, arity) arity,
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
