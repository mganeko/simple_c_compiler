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
Node *code[100];

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

// // 次のトークンが変数かどうかをチェック
// bool is_variable() {
//   if (token->kind == TK_IDENT) {
//     return true;
//   }

//   return false;
// }


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

    if ( (memcmp(p, "==", 2) == 0) || (memcmp(p, "!=", 2) == 0)
      || (memcmp(p, "<=", 2) == 0) || (memcmp(p, ">=", 2) == 0)
    ) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    //if ('a' <= *p && *p <= 'z') {
    //  cur = new_token(TK_IDENT, cur, p++, 0);
    //  cur->len = 1;
    //  continue;
    //}
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
    Node *node = new_node_lvar(tok);
    return node;
  }

  // そうでなければ数値のはず
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
  if (consume("=")) 
    node = new_node(ND_ASSIGN, node, equality());

  return node;
}


Node *expr() {
  Node *node = assign();
  return node;
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}