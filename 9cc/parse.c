#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// -- 現在の関数レベル --
int parse_level = 0; // 0:top, 1: in func

// 現在着目しているトークン 
Token *token;

// 入力プログラム 
char *user_input;

// --- コード全体 ---
Node *code[CODE_LINE_MAX];

// --- 整数型の代表値  --
Type type_int = {INT, NULL};


// -- ログ ---
//int log_level = 0; // 指定したレベル以下のログを出力する
//int log_level = 1; // 指定したレベル以下のログを出力する
int log_level = 2; // 指定したレベル以下のログを出力する
//int log_level = 3; // 指定したレベル以下のログを出力する


// ログを出す
void report_log(int level, char *fmt, ...) {
  if (level > log_level)
    return;

  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

void report_type(int level, Type* type) {
  if (level > log_level)
    return;

  if (type->ty == ARRAY) {
    fprintf(stderr, "Array of ");
    report_type(level, type->ptr_to);
    fprintf(stderr, ", size=%ld ", type->array_size);
    return;
  }

  Type* t = type;
  while(t->ty == PTR) {
    fprintf(stderr, "*");
    t = t->ptr_to;
  }
  fprintf(stderr, "INT");
}

void report_lvar(int level, LVar *lvar) {
  if (level > log_level)
    return;

  char buf[128];
  strncpy(buf, lvar->name, lvar->len);
  buf[lvar->len] = '\0';

  fprintf(stderr, "LVar: name=%s, offset=%d Type=", buf, lvar->offset);
  report_type(level, lvar->type);
  report_log(level, "\n");
}

// エラーを報告する
void report_error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)
  ) {
    return false;
  }

  token = token->next;
  return true;
}

void dump_token() {
  report_log(1, "token kind=%d, len=%d str=%s", token->kind, token->len, token->str);
}



// // 次のトークンが変数かどうかをチェック
// bool is_variable() {
//   if (token->kind == TK_IDENT) {
//     return true;
//   }

//   return false;
// }


// 次のトークンが期待してい種類かどうか？
// - 期待した種類の場合はトークンを1つ読み進めて真を返す。
// - それ以外の場合には偽を返す。
bool consume_kind(TokenKind kind) {
  if (token->kind != kind)
    return false;

  token = token->next;
  return true;
}

// 次のトークンがアイデンティファイヤーかどうかをチェック
// 変数の場合は、トークンを1つ読み進めて元のトークンを返す
// それ以外はNULLを返す
Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }

  Token *current = token;
  token = token->next;
  return current;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)
   ) {
    //error("'%c'ではありません", op);
    error_at(token->str, "'%s'ではありません", op);
  }

  token = token->next;
}


// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM) {
    //error("数ではありません");
    //error_at(token->kind, "数ではありません");
    error_at(token->str, "数ではありません");
  }

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// アイデンティファイヤーとして有効な文字か？
bool is_ident_char(char c) {
  if ( ('A' <= c) && (c <= 'Z') ) {
    return true;
  }

  if ( ('a' <= c) && (c <= 'z') ) {
    return true;
  }

  if (c == '_') {
    return true;
  }

  return false;
}

// アイデンティファイヤーの長さを調べる
int count_ident_len(char *p) {
  int len = 0;
  while (is_ident_char(*p)) {
    len++;
    p++;
  }

  return len;
}

// 文字列トークンの文字か？
int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    ('0' <= c && c <= '9') ||
    (c == '_');
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// アイデンティファイヤーのトークンを作る
Token *new_token_ident(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = count_ident_len(str);
  cur->next = tok;
  return tok;
}


// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int len;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // --- 比較演算子 ---
    if ( (memcmp(p, "==", 2) == 0) || (memcmp(p, "!=", 2) == 0)
      || (memcmp(p, "<=", 2) == 0) || (memcmp(p, ">=", 2) == 0)
    ) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // --- 四則演算子、記号 ---
    if (strchr("+-*/()<>=;{},&[]", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // --- sizeof演算子 ---
    len = 6;
    if (strncmp(p, "sizeof", len) == 0 && !is_alnum(p[len])) {
      report_log(2, "sizeof発見");
      cur = new_token(TK_SIZEOF, cur, p, len);
      p += len;
      continue;
    }


    // ==== 型宣言 =====
    // -- int --
    len = 3;
    if (strncmp(p, "int", len) == 0 && !is_alnum(p[len])) {
      report_log(3, "int発見");
      cur = new_token(TK_TYPE_INT, cur, p, len);
      p += len;
      continue;
    }

    // ==== 予約語 =====
    // -- return --
    len = 6;
    if (strncmp(p, "return", len) == 0 && !is_alnum(p[len])) {
      cur = new_token(TK_RETURN, cur, p, len);
      p += len;
      continue;
    }

    len = 2;
    if (strncmp(p, "if", len) == 0 && !is_alnum(p[len])) {
      report_log(3, "if発見");
      cur = new_token(TK_IF, cur, p, len);
      p += len;
      report_log(3, "if切り出し");
      continue;
    }
    len = 4;
    if (strncmp(p, "else", len) == 0 && !is_alnum(p[len])) {
      report_log(3, "else発見");
      cur = new_token(TK_ELSE, cur, p, len);
      p += len;
      continue;
    }
    len=5;
    if (strncmp(p, "while", len) == 0 && !is_alnum(p[len])) {
      report_log(3, "while発見");
      cur = new_token(TK_WHILE, cur, p, len);
      p += len;
      continue;
    }

    len = 3;
    if (strncmp(p, "for", len) == 0 && !is_alnum(p[len])) {
      report_log(3, "for発見");
      cur = new_token(TK_FOR, cur, p, len);
      p += len;
      continue;
    }
    // ==== 予約語 =====

    // -- アイデンティファイヤー
    if(is_ident_char(*p)) {
      cur = new_token_ident(TK_IDENT, cur, p);
      p += cur->len;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    //error("トークナイズできません");
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// ---- token ----


// --- local variable ---
// ローカル変数
//LVar *top_locals = NULL;


// --- local variable ---
// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok, LVar **locals_ptr) {
  LVar *locals = *locals_ptr;
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }

  return NULL;
}

// ローカル変数の数を返す
int count_lvar(LVar *locals) {
  int count = 0;
  for (LVar *var = locals; var; var = var->next) {
    count++;
  }

  return count;
}

// 新しい変数を宣言する
LVar *decl_new_lvar_0(Token *tok, Type* type, LVar **locals_ptr) {
  if (find_lvar(tok, locals_ptr))
    error_at(tok->str, "すでに変数が宣言されています");
  
  LVar *locals = *locals_ptr;
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;
  if (locals) {
    // 前の変数がある場合
    report_log(2, "--next variable--");
    if (locals->type->ty == ARRAY) {
      report_log(1, "--after array--");
      lvar->offset = locals->offset + 8*locals->type->array_size;
    }
    else 
      lvar->offset = locals->offset + 8;
  }
  else {
    // 最初の変数の場合
    report_log(4, "--1st variable--");
    lvar->offset = 8; // NG0;
  }

  locals = lvar;
  *locals_ptr = lvar;
  report_log(3, "--locals=%d, *locals_ptr=%d", locals, *locals_ptr);
  report_lvar(3, lvar);

  return lvar;
}

// 新しい変数/配列を宣言する
LVar *decl_new_lvar(Token *tok, Type* type, LVar **locals_ptr) {
  if (find_lvar(tok, locals_ptr))
    error_at(tok->str, "すでに変数/配列が宣言されています");
  
  LVar *locals = *locals_ptr;
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;
  if (locals) {
    // 前の変数がある場合
    report_log(2, "--next variable--");
    if (locals->type->ty == ARRAY) {
      report_log(2, "--after array--");
      lvar->offset = locals->offset + 8*locals->type->array_size;
    }
    else 
      lvar->offset = locals->offset + 8;
  }
  else {
    // 最初の変数の場合
    report_log(2, "--1st variable--");
    lvar->offset = 8; // NG0;
  }

  locals = lvar;
  *locals_ptr = lvar;
  report_log(3, "--locals=%d, *locals_ptr=%d", locals, *locals_ptr);
  report_lvar(3, lvar);

  return lvar;
}


// --- node ---
Node *expr(LVar **locals_ptr);
Node *stmt(LVar **locals_ptr);
Node *mul(LVar **locals_ptr);
Node *add(LVar **locals_ptr);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_lvar(Token *tok, LVar **locals_ptr) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *locals = *locals_ptr;
  LVar *lvar = find_lvar(tok, locals_ptr);
  if (lvar) {
    node->offset = lvar->offset;
    node->lvar = lvar;
    return node;
  }

  // -- 変数が未定義 --
  error_at(tok->str, "変数が未定義です");
}

Node *new_node_func_call(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_CALL;
  report_log(3, "- Node FUNC_CALL len=%d ident=%s", tok->len, tok->str);
  node->func_name = calloc(tok->len+1, sizeof(char));
  strncpy(node->func_name, tok->str, tok->len);
  report_log(3, "- Node FUNC_CALL funcname=%s", node->func_name);

  return node;
}

Node *new_node_func_def(Token *tok, Type* type) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DEF;
  report_log(3, "- Node FUNC_DEF len=%d ident=%s", tok->len, tok->str);
  node->func_name = calloc(tok->len+1, sizeof(char));
  strncpy(node->func_name, tok->str, tok->len);
  report_log(2, "- Node FUNC_DEF funcname=%s", node->func_name);
  node->func_type = type;
  report_type(2, type);
  report_log(2, "\n");

  return node;
}

Node *convert_func_call_to_def(Node* node) {
  node->kind = ND_FUNC_DEF;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// -- 型のポインターの深さを返す --
// int type_ptr_depth(Type* type) {
//   if (type->ty == INT)
//     return 0;
  
//   int depth = type_ptr_depth(type->ptr_to) + 1;
//   return depth;
// }

// -- 型を判定 --
Type *type_of(Node *node) {
  Type *tp_child;
  Type *tp_new;
  Type *tp_left;
  Type *tp_right;
  int depth_left;
  int depth_right;
  switch(node->kind) {
    case ND_NUM:
      return &type_int;

    case ND_ADDR:
      tp_child = type_of(node->lhs);
      tp_new = calloc(1, sizeof(Type));
      tp_new->ty = PTR;
      tp_new->ptr_to = tp_child;
      return tp_new;

    case ND_LVAR:
      return node->lvar->type;

    case ND_DEREF:
      tp_child = type_of(node->lhs);
      return tp_child->ptr_to; // 参照先

    case ND_FUNC_CALL: // 今のところ、関数の戻り値はintのみ
      return &type_int;

    case ND_MUL: // *
    case ND_DIV: // /
      // 今のところ、結果はintのみ
      return &type_int;

    case ND_ADD: // +
    case ND_SUB: // -
      tp_left = type_of(node->lhs);
      tp_right = type_of(node->rhs);
      if ((tp_left->ty == PTR) && (tp_right->ty == PTR))
          report_error("type_of() ポインター同志の演算はできません");

      if (tp_left->ty == PTR)
        return tp_left;
 
      if (tp_right->ty == PTR)
        return tp_right;
      
      // 整数同志のはず
      return tp_left;

    case ND_EQ: // ==
    case ND_NE: // !=
    case ND_GT: // >
    case ND_LT: // <
    case ND_GE: // >=
    case ND_LE: // <=
      // 今のところ、boolはint
      return &type_int;

    case ND_ASSIGN: // = (代入)
      report_log(2, "Typeof ND_ASSIGN");
      tp_left = type_of(node->lhs);
      tp_right = type_of(node->rhs);
      report_type(2, tp_left);
      report_log(2, "\n");
      report_type(2, tp_right);
      report_log(2, "\n");
      if (tp_left->ty != tp_right->ty)
        report_error("代入の型が一致していません");
      return tp_left;

    default:
      report_error("UNKNOWN Type");
  }
}

// -- サイズを判定 --
int calc_size(Node *node) {
  Type* type = type_of(node);
  if (type->ty == INT)
    return 4;
  else if (type->ty == PTR)
    return 8;

  report_error("UNKNOWN Type for sizeof()");
}

Node *primary(LVar **locals_ptr) {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr(locals_ptr);
    expect(")");
    return node;
  }

  // 変数（アイデンティファイヤー）か？
  Token *tok = consume_ident();
  if (tok) {
    // --- 次が"("なら、関数 --
    if (consume("(")) {
      if (parse_level == 1) {
        // -- 関数内部 --
        if (consume(")")) {
          // 引数なし、関数呼び出し
          report_log(3, "--parse Node FUNC_CALL, no args");
          Node *node = new_node_func_call(tok);
          return node;
        }

        // --- 引数あり --
        report_log(3, "--parse Node FUNC_CALL with args");
        Node *node = new_node_func_call(tok);
        node->args = calloc(FUNC_ARG_MAX, sizeof(Node*));
        node->args_count = 0;
        while(1) {
          if (node->args_count >= FUNC_ARG_MAX) {
            error_at(token->str, "TOO MANY func args");
          }
          node->args[node->args_count] = expr(locals_ptr);
          node->args_count++;
          if (consume(")")) break;
          expect(",");
        }
        report_log(3, "--parse Node FUNC_CALL end, with args=%d", node->args_count);
        return node;
      }

      if (parse_level == 0) {
        // -- 関数定義 --
        error_at(tok->str, "関数定義はここには来ないはず");
      }

      // -- 関数定義 --
      error_at(tok->str, "引数あり関数定義はここには来ないはず");
    }

    // -- そうでない場合は、変数 --
    report_log(2, "--parse Node IDENT");
    Node *node = new_node_lvar(tok, locals_ptr);
    if (! consume("[")) 
      return node; // 普通の変数


    // -- 配列 --
    report_log(2, "--parse Array index");

    // 足し算
    Node* add = new_node(ND_ADD, node, expr(locals_ptr));
    expect("]");

    // *(a + x) に変換
    Node *deref = new_node(ND_DEREF, add, NULL);
    return deref;
  }

  // そうでなければ数値のはず
  report_log(3, "--parse Node NUM");
  return new_node_num(expect_number());
}

Node *unary(LVar **locals_ptr) {
  if (consume_kind(TK_SIZEOF)) {
    // 対象をパース
    Node *contents = primary(locals_ptr);

    // -- サイズを判定 --
    int size = calc_size(contents);

    // 定数に変換
    return new_node_num(size);
  }

  if (consume("+"))
    return primary(locals_ptr);
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary(locals_ptr));
  if (consume("*"))
    return new_node(ND_DEREF, unary(locals_ptr), NULL);
  if (consume("&"))
    return new_node(ND_ADDR, unary(locals_ptr), NULL);

  return primary(locals_ptr);
}

Node *mul(LVar **locals_ptr) {
  Node *node = unary(locals_ptr);

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary(locals_ptr));
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary(locals_ptr));
    else
      return node;
  }
}

Node *add(LVar **locals_ptr) {
  Node *node = mul(locals_ptr);

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul(locals_ptr));
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul(locals_ptr));
    else
      return node;
  }
}

Node *relational(LVar **locals_ptr) {
  Node *node = add(locals_ptr);

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add(locals_ptr));
    else if (consume("<="))
      node = new_node(ND_LE, node, add(locals_ptr));
    else if (consume(">"))
      node = new_node(ND_GT, node, add(locals_ptr));
    else if (consume(">="))
      node = new_node(ND_GE, node, add(locals_ptr));
    else
      return node;
  }
}

Node *equality(LVar **locals_ptr) {
  Node *node = relational(locals_ptr);

  for (;;) {
    if (consume("==")) 
      node = new_node(ND_EQ, node, relational(locals_ptr));
    else if (consume("!="))
      node = new_node(ND_NE, node, relational(locals_ptr));
    else
      return node;
  }
}

Node *assign(LVar **locals_ptr) {
  Node *node = equality(locals_ptr);
  if (consume("=")) {
    report_log(3, "--Node =");
    node = new_node(ND_ASSIGN, node, equality(locals_ptr));
  }

  return node;
}


Node *expr(LVar **locals_ptr) {
  Node *node = assign(locals_ptr);
  return node;
}

Node *stmt(LVar **locals_ptr) {
  // Node *node = expr();
  // expect(";");

  Node *node = NULL;

  if (consume_kind(TK_TYPE_INT)) {
    report_log(3, "stmt() int");

    // int型
    Type *type = calloc(1, sizeof(Type));
    type->ty = INT;

    // ポインターの扱い
    while(consume("*")) {
      report_log(2, "stmt() pointer(*) for variable");
      Type* new_tp = calloc(1, sizeof(Type));
      new_tp->ty = PTR;
      new_tp->ptr_to = type;
      type = new_tp;
    } 

    // 変数（アイデンティファイヤー）か？
    Token *tok = consume_ident();
    if (! tok)
      error_at(token->str, "intの後がアイデンティファイアーではありません");
  
    // 配列
    if (consume("[")) {
      report_log(4, "Find Array [");
      int val = expect_number();
      expect("]");
      expect(";");
      report_log(4, "Close Array ] size=%d", val);
   
      Type* new_tp = calloc(1, sizeof(Type));
      new_tp->ty = ARRAY;
      new_tp->ptr_to = type;
      new_tp->array_size = val;
      type = new_tp;

      report_log(2, "stmt() decl_new_lvar (array)");
      LVar *array = decl_new_lvar(tok, type, locals_ptr);
      report_lvar(2, array);
      report_log(2, "stmt() new_node_lvar for array");
      node = new_node_lvar(tok, locals_ptr);
      report_lvar(2, node->lvar);

      // TODO
      return node;
    }

    // 変数
    if (consume(";")) {
      // -- 変数宣言 --
      report_log(3, "stmt() decl_new_lvar");
      decl_new_lvar(tok, type, locals_ptr);
      report_log(3, "stmt() new_node_lvar");
      node = new_node_lvar(tok, locals_ptr);
    }
    else if (consume("(")) {
      // --- 関数定義 ---
      if (consume(")")) {
        // --- 引数なし 関数定義 --- 
        report_log(3, "--parse Node FUNC_DEF, no args");
        if (parse_level != 0)
          report_error("関数定義がトップレベルではありません");

        parse_level = 1;
        expect("{");

        Node *node = new_node_func_def(tok, type);
        report_log(3, "--fuction body BLOCK");
        Node *body = calloc(1, sizeof(Node));
        node->body = body;
        body->kind = ND_BLOCK;
        body->stmts = calloc(BLOCK_LINE_MAX, sizeof(Node*));
        body->stmts_count = 0;
        while(! consume("}")) {
          if ( (body->stmts_count) >= BLOCK_LINE_MAX) {
            report_error("TOO MANY LINES(%d) in func body block", (body->stmts_count+1));
          }
          report_log(4, "node addr=%d  node->func_locals=%d, &node->func_locals=%d, &(node->func_locals)=%d", node, node->func_locals, &node->func_locals, &(node->func_locals));
          body->stmts[body->stmts_count] = stmt(&(node->func_locals));
          body->stmts_count++;
          report_log(3, "--func body line %d", body->stmts_count);
        }
        report_log(3, "--func body BLOCK end, %d lines", body->stmts_count);

        parse_level = 0;
        return node;
      }
      else {
        // --- 引数あり 関数定義 ---  
        report_log(2, "--parse Node  FUNC_DEF with args");
        Node *node = new_node_func_def(tok, type);
        //report_error("NOT SUPPORTED YET");
        node->args = calloc(FUNC_ARG_MAX, sizeof(Node*));
        node->args_count = 0;
        while(1) {
          if (node->args_count >= FUNC_ARG_MAX) {
            error_at(token->str, "TOO MANY func args");
          }

          // 型宣言か？
          if (! consume_kind(TK_TYPE_INT))
            error_at(token->str, "NO int for arg");

          // int型
          Type *arg_type = calloc(1, sizeof(Type));
          arg_type->ty = INT;

          // ポインターの扱い
          while(consume("*")) {
            report_log(2, "pointer(*) for func arg");
            Type* new_tp = calloc(1, sizeof(Type));
            new_tp->ty = PTR;
            new_tp->ptr_to = arg_type;
            arg_type = new_tp;
          } 

          // 変数（アイデンティファイヤー）か？
          Token *tok = consume_ident();
          if (! tok) report_error(token->str, "NOT ARG var");

          decl_new_lvar(tok, arg_type, &(node->func_locals));
          report_log(3, "stmt() new_node_lvar for func def arg");
          Node *arg = new_node_lvar(tok, &(node->func_locals));
          report_log(3, "find Arg offset=%d", arg->offset);
          node->args[node->args_count] = arg;
          node->args_count++;
          if (consume(")")) break;
          expect(",");
        }

        parse_level = 1;
        expect("{");
        report_log(3, "--fuction body BLOCK");
        Node *body = calloc(1, sizeof(Node));
        node->body = body;
        body->kind = ND_BLOCK;
        body->stmts = calloc(BLOCK_LINE_MAX, sizeof(Node*));
        body->stmts_count = 0;
        while(! consume("}")) {
          if ( (body->stmts_count) >= BLOCK_LINE_MAX) {
            report_error("TOO MANY LINES(%d) in func body block", (body->stmts_count+1));
          }
          report_log(4, "node addr=%d  node->func_locals=%d, &node->func_locals=%d, &(node->func_locals)=%d", node, node->func_locals, &node->func_locals, &(node->func_locals));
          body->stmts[body->stmts_count] = stmt(&(node->func_locals));
          body->stmts_count++;
          report_log(3, "--func body line %d", body->stmts_count);
        }
        report_log(3, "--func body BLOCK end, %d lines", body->stmts_count);

        parse_level = 0;
        report_log(3, "--parse Node FUNC_DEF end, with args=%d", node->args_count);

        return node;
      }
    }
    else {
      error_at(token->str, "変数宣言/関数定義のどちらでもありません");
    }
  }
  else if (consume_kind(TK_RETURN)) {
    report_log(3, "--Node RETURN");
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr(locals_ptr);

    if (!consume(";"))
      error_at(token->str, "';'ではないトークンです");
  } else if (consume_kind(TK_IF)) {
    report_log(3, "--Node IF");

    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = expr(locals_ptr);
    expect(")");
    node->body = stmt(locals_ptr);

    // -- check else --
    if (consume_kind(TK_ELSE)) {
      report_log(3, "--Node ELSE");

      node->elsebody = stmt(locals_ptr);
    }
    else {
      node->elsebody = NULL;
    }
  } else if (consume_kind(TK_WHILE)) {
    report_log(3, "--Node WHILE");
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = expr(locals_ptr);
    expect(")");
    node->body = stmt(locals_ptr);
  } 
  else if (consume_kind(TK_FOR)) {
    report_log(3, "--Node FOR");
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;

    if(consume(";")) {
      node->init = NULL; // init expr がないケースもある
    }
    else {
      node->init = expr(locals_ptr); 
      expect(";");
    }

    if(consume(";")) {
      node->cond = NULL; // cond expr がないケースもある
    }
    else {
      node->cond = expr(locals_ptr);
      expect(";");
    }

    if(consume(")")) {
      node->post = NULL; // post expr がないケースもある
    }
    else {
      node->post = expr(locals_ptr); // post expr がないケースもある
      expect(")");
    }

    node->body = stmt(locals_ptr);
    report_log(3, "--Node FOR end");
  }
  else if (consume("{")) {
    report_log(3, "--Node BLOCK");
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = calloc(BLOCK_LINE_MAX, sizeof(Node*));
    node->stmts_count = 0;
    while(! consume("}")) {
      if ( (node->stmts_count) >= BLOCK_LINE_MAX) {
        report_error("TOO MANY LINES(%d) in block", (node->stmts_count+1));
      }
      node->stmts[node->stmts_count] = stmt(locals_ptr);
      node->stmts_count++;
    }
    report_log(3, "--Node BLOCK end, %d lines", node->stmts_count);
  } 
  else {
    node = expr(locals_ptr);
    if (node->kind == ND_FUNC_DEF) {
      // func def の場合は、";" は不要
      return node;
    }

    // -- それ以外は、";"が必要 --
    if (!consume(";"))
      error_at(token->str, "';'ではないトークンです stmt()");
  }

  //dump_token();
  //if (!consume(";"))
  //  error_at(token->str, "';'ではないトークンです");

  return node;
}

void program(LVar **locals_ptr) {
  int i = 0;
  while (!at_eof()) {
    if (i > CODE_LINE_MAX) {
      report_error("TOO MANY LINES %d", i);
    }

    code[i++] = stmt(locals_ptr);
  }
  code[i] = NULL;
}