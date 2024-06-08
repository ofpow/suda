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

typedef Value (*Native)(Value*, Location);

Native natives[] = {
    add1
};

char* native_names[] = {
    "add1",
};

int native_arities[] = {
    1, // add1
};

int is_native(char *func) {
    for (u_int32_t i = 0; i < (sizeof(native_names) / 8); i++)
        if (!strcmp(func, native_names[i]))
            return i;
    return -1;
}
