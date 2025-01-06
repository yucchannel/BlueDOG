#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dlfcn.h>

#define MAX_TOKENS 100
#define MAX_PLUGIN_NAME 50
#define MAX_MODULE_NAME 50

typedef enum {
    TOKEN_INT, TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_PLUS, TOKEN_EOF, TOKEN_UNKNOWN, TOKEN_PLUGIN, TOKEN_MODULE
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
} Token;

typedef enum {
    AST_DECLARATION, AST_ASSIGNMENT, AST_ADDITION, AST_NUMBER, AST_PLUGIN, AST_MODULE
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char value[256];
    struct ASTNode *left, *right;
} ASTNode;

typedef struct Plugin {
    char name[MAX_PLUGIN_NAME];
    void (*execute)(const char *, const char *);
} Plugin;

typedef struct Module {
    char name[MAX_MODULE_NAME];
    void (*load)(void);
} Module;

// 字句解析
Token get_next_token(const char *source, int *pos) {
    Token token = {TOKEN_UNKNOWN, ""};
    while (source[*pos] != '\0' && isspace(source[*pos])) (*pos)++;

    if (isdigit(source[*pos])) {
        token.type = TOKEN_NUMBER;
        int i = 0;
        while (isdigit(source[*pos])) {
            token.value[i++] = source[*pos];
            (*pos)++;
        }
        token.value[i] = '\0';
        return token;
    }

    if (source[*pos] == '+') {
        token.type = TOKEN_PLUS;
        token.value[0] = source[*pos];
        token.value[1] = '\0';
        (*pos)++;
        return token;
    }

    if (source[*pos] == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }

    if (isalpha(source[*pos])) {
        token.type = TOKEN_IDENTIFIER;
        int i = 0;
        while (isalnum(source[*pos])) {
            token.value[i++] = source[*pos];
            (*pos)++;
        }
        token.value[i] = '\0';
        return token;
    }

    if (source[*pos] == '%') {
        token.type = TOKEN_PLUGIN;
        (*pos)++;
        return token;
    }

    if (source[*pos] == '&') {
        token.type = TOKEN_MODULE;
        (*pos)++;
        return token;
    }

    token.type = TOKEN_UNKNOWN;
    return token;
}

// ASTノード作成
ASTNode* create_ast_node(ASTNodeType type, char *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    strcpy(node->value, value);
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 演算式の解析
ASTNode* parse_expression(Token *tokens, int *index) {
    ASTNode *left = create_ast_node(AST_NUMBER, tokens[*index].value);
    (*index)++;

    if (tokens[*index].type == TOKEN_PLUS) {
        (*index)++;  // 次のトークンに進む
        ASTNode *right = create_ast_node(AST_NUMBER, tokens[*index].value);
        (*index)++;
        ASTNode *node = create_ast_node(AST_ADDITION, "+");
        node->left = left;
        node->right = right;
        return node;
    }

    return left;
}

// AST評価（簡単な足し算）
int evaluate_ast(ASTNode *node) {
    if (node->type == AST_NUMBER) {
        return atoi(node->value);
    }

    if (node->type == AST_ADDITION) {
        int left_val = evaluate_ast(node->left);
        int right_val = evaluate_ast(node->right);
        return left_val + right_val;
    }

    return 0;  // 不明なノードの場合は0を返す
}

// プラグイン実行関数
void load_and_execute_plugin(const char *plugin_name, const char *filename) {
    printf("Plugin: %s\n", plugin_name);
    printf("Executing BlueDOG code from file: %s\n", filename);
}

// モジュール実行関数
void load_and_execute_module(const char *module_name, const char *filename) {
    printf("Module: %s\n", module_name);
    printf("Loading module from file: %s\n", filename);

    // モジュールを動的にロード
    void *handle = dlopen("my_module.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "Error loading module: %s\n", dlerror());
        return;
    }

    // モジュール内の関数を取得して実行
    void (*module_func)(void) = (void (*)())dlsym(handle, "module_function");
    if (module_func) {
        module_func();
    } else {
        fprintf(stderr, "Function not found in module: %s\n", dlerror());
    }

    // モジュールを閉じる
    dlclose(handle);
}

// 実行関数
void execute_bluedog(const char *filename, Plugin *plugins, int plugin_count, Module *modules, int module_count) {
    // 仮のソースコードを読む（実際のコードはファイルから読み込む）
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }

    char source[1000];
    fread(source, sizeof(char), 1000, file);
    fclose(file);

    int pos = 0;
    Token tokens[MAX_TOKENS];
    int token_count = 0;

    while (1) {
        Token token = get_next_token(source, &pos);
        if (token.type == TOKEN_EOF) break;
        tokens[token_count++] = token;
    }

    // プラグインとモジュールの処理
    for (int i = 0; i < plugin_count; i++) {
        if (strstr(filename, plugins[i].name)) {
            plugins[i].execute(plugins[i].name, filename);
            break;
        }
    }

    for (int i = 0; i < module_count; i++) {
        if (strstr(filename, modules[i].name)) {
            modules[i].load();
            break;
        }
    }

    // 式解析とAST生成
    int index = 0;
    ASTNode *ast = parse_expression(tokens, &index);

    // AST評価
    int result = evaluate_ast(ast);
    printf("Result: %d\n", result);

    // メモリ解放
    free(ast);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Plugin plugins[] = {
        {"print_plugin", load_and_execute_plugin}
    };

    Module modules[] = {
        {"web_server_module", load_and_execute_module}
    };

    execute_bluedog(argv[1], plugins, sizeof(plugins) / sizeof(Plugin), modules, sizeof(modules) / sizeof(Module));

    return 0;
}
