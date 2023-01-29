#pragma once

void interpret(Node **nodes, int stmts_size) {
    for (int i = 0; i < stmts_size; i++) {
        switch (nodes[i]->type) {
            case AST_Print:;
                ASSERT((nodes[i]->left->type == AST_Literal), "Only literals can be printed\n");
                switch (nodes[i]->left->value->type) {
                    case Tok_String:
                        printf("%.*s\n", nodes[i]->left->value->length - 2, nodes[i]->left->value->start + 1);
                        break;
                    case Tok_Number:
                        printf("%.*s\n", nodes[i]->left->value->length, nodes[i]->left->value->start);
                        break;
                    default:
                        ERR("Unknown type `%s`\n", find_tok_type(nodes[i]->left->value->type));
                }
                break;
            case AST_End:
                return;
            default: ERR("Unsupported statement type `%s`\n", find_ast_type(nodes[i]->type));
        }
    }
}
