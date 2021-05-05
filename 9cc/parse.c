#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"



// 現在着目しているトークン 
Token *token;

// 入力プログラム 
char *user_input;

// --- コード全体 ---
Node *code[CODE_LINE_MAX];


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
  //fprintf(stderr, "\n");
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
  report_log(1, "token kind=%d, len=%d str=%s \n", token->kind, token->len, token->str);
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
    if (strchr("+-*/()<>=;{},", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // ==== 予約語 =====
    // -- return --
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      // tokens[i].ty = TK_RETURN;
      // tokens[i].str = p;
      // i++;
  
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      report_log(3, "if発見\n");
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      report_log(3, "if切り出し\n");
      continue;
    }
    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      report_log(3, "else発見\n");
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }
    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      report_log(3, "while発見\n");
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      report_log(2, "for発見\n");
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
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
LVar *locals = NULL;


// --- local variable ---
// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }

  return NULL;
}

// ローカル変数の数を返す
int count_lvar() {
  int count = 0;
  for (LVar *var = locals; var; var = var->next) {
    count++;
  }

  return count;
}


// --- node ---
Node *expr();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// Node *new_node_lvar(char val_name) {
//   Node *node = calloc(1, sizeof(Node));
//   node->kind = ND_LVAR;
//   node->offset = (val_name - 'a' + 1) * 8;
//   return node;
// }

Node *new_node_lvar(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar) {
    node->offset = lvar->offset;
  } else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals) {
      lvar->offset = locals->offset + 8;
    }
    else {
      lvar->offset = 0;
    }
    node->offset = lvar->offset;
    locals = lvar;
  }

  return node;
}

Node *new_node_func_call(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_CALL;
  report_log(3, "- Node FUNC_CALL len=%d ident=%s\n", tok->len, tok->str);
  node->func_name = calloc(tok->len+1, sizeof(char));
  strncpy(node->func_name, tok->str, tok->len);
  report_log(3, "- Node FUNC_CALL funcname=%s\n", node->func_name);

  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // 変数（アイデンティファイヤー）か？
  Token *tok = consume_ident();
  if (tok) {
    // --- 次が"("なら、関数 --
    if (consume("(")) {
      if (consume(")")) {
        // まずは引数なし
        report_log(2, "--parse Node FUNC_CALL, no args\n");
        Node *node = new_node_func_call(tok);
        return node;
      }

      // --- 引数あり --
      report_log(2, "--parse Node FUNC_CALL with args\n");
      Node *node = new_node_func_call(tok);
      node->args = calloc(FUNC_ARG_MAX, sizeof(Node*));
      node->args_count = 0;
      while(1) {
        if (node->args_count > FUNC_ARG_MAX) {
          error_at(token->str, "TOO MANY func args");
        }
        node->args[node->args_count] = expr();
        node->args_count++;
        if (consume(")")) break;
        expect(",");
      }
      return node;
    }

    // -- そうでない場合は、変数 --
    report_log(3, "--parse Node IDENT\n");
    Node *node = new_node_lvar(tok);
    return node;
  }

  // そうでなければ数値のはず
  report_log(3, "--parse Node NUM\n");
  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_GT, node, add());
    else if (consume(">="))
      node = new_node(ND_GE, node, add());
    else
      return node;
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) 
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    report_log(3, "--Node =\n");
    node = new_node(ND_ASSIGN, node, equality());
  }

  return node;
}


Node *expr() {
  Node *node = assign();
  return node;
}

Node *stmt() {
  // Node *node = expr();
  // expect(";");

  Node *node = NULL;

  if (consume_kind(TK_RETURN)) {
    report_log(3, "--Node RETURN\n");
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();

    if (!consume(";"))
      error_at(token->str, "';'ではないトークンです");
  } else if (consume_kind(TK_IF)) {
    report_log(3, "--Node IF\n");

    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = expr();
    expect(")");
    node->body = stmt();

    // -- check else --
    if (consume_kind(TK_ELSE)) {
      report_log(3, "--Node ELSE\n");

      node->elsebody = stmt();
    }
    else {
      node->elsebody = NULL;
    }
  } else if (consume_kind(TK_WHILE)) {
    report_log(3, "--Node WHILE\n");
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = expr();
    expect(")");
    node->body = stmt();
  } 
  else if (consume_kind(TK_FOR)) {
    report_log(2, "--Node FOR\n");
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;

    if(consume(";")) {
      node->init = NULL; // init expr がないケースもある
    }
    else {
      node->init = expr(); 
      expect(";");
    }

    if(consume(";")) {
      node->cond = NULL; // cond expr がないケースもある
    }
    else {
      node->cond = expr();
      expect(";");
    }

    if(consume(")")) {
      node->post = NULL; // post expr がないケースもある
    }
    else {
      node->post = expr(); // post expr がないケースもある
      expect(")");
    }

    node->body = stmt();
    report_log(2, "--Node FOR end\n");
  }
  else if (consume("{")) {
    report_log(2, "--Node BLOCK\n");
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = calloc(BLOCK_LINE_MAX, sizeof(Node*));
    node->stmts_count = 0;
    while(! consume("}")) {
      if ( (node->stmts_count) >= BLOCK_LINE_MAX) {
        report_error("TOO MANY LINES(%d) in block", (node->stmts_count+1));
      }
      node->stmts[node->stmts_count] = stmt();
      node->stmts_count++;
    }
    report_log(2, "--Node BLOCK end, %d lines\n", node->stmts_count);
  } 
  else {
    node = expr();
    if (!consume(";"))
      error_at(token->str, "';'ではないトークンです");
  }

  //dump_token();
  //if (!consume(";"))
  //  error_at(token->str, "';'ではないトークンです");

  return node;
}

void program() {
  int i = 0;
  while (!at_eof()) {
    if (i > CODE_LINE_MAX) {
      report_error("TOO MANY LINES %d", i);
    }

    code[i++] = stmt();
  }
  code[i] = NULL;
}