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
    if (strchr("+-*/()<>=;{},&", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // ==== 型宣言 =====
    // -- int --
    if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
      report_log(2, "int発見");
      cur = new_token(TK_TYPE_INT, cur, p, 3);
      p += 3;
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
      report_log(3, "if発見");
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      report_log(3, "if切り出し");
      continue;
    }
    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      report_log(3, "else発見");
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }
    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      report_log(3, "while発見");
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      report_log(3, "for発見");
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
LVar *decl_new_lvar(Token *tok, LVar **locals_ptr) {
  if (find_lvar(tok, locals_ptr))
    error_at(tok->str, "すでに変数が宣言されています");
  
  LVar *locals = *locals_ptr;
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  if (locals) {
    report_log(4, "--next variable--");
    lvar->offset = locals->offset + 8;
  }
  else {
    report_log(4, "--1st variable--");
    lvar->offset = 8; // NG0;
  }

  locals = lvar;
  *locals_ptr = lvar;
  report_log(3, "--locals=%d, *locals_ptr=%d", locals, *locals_ptr);

  return lvar;
}


// --- node ---
Node *expr(LVar **locals_ptr);
Node *stmt(LVar **locals_ptr);

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

Node *new_node_lvar(Token *tok, LVar **locals_ptr) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *locals = *locals_ptr;
  LVar *lvar = find_lvar(tok, locals_ptr);
  if (lvar) {
    node->offset = lvar->offset;
    return node;
  }
  
  /*
   else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals) {
      report_log(4, "--next variable--");
      lvar->offset = locals->offset + 8;
    }
    else {
      report_log(4, "--1st variable--");
      lvar->offset = 8; // NG0;
    }
    node->offset = lvar->offset;
    locals = lvar;
    *locals_ptr = lvar;
    report_log(3, "--locals=%d, *locals_ptr=%d", locals, *locals_ptr);
  }

  return node;
  */

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

Node *new_node_func_def(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DEF;
  report_log(3, "- Node FUNC_DEF len=%d ident=%s", tok->len, tok->str);
  node->func_name = calloc(tok->len+1, sizeof(char));
  strncpy(node->func_name, tok->str, tok->len);
  report_log(3, "- Node FUNC_DEF funcname=%s", node->func_name);

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
        if (consume(")")) {
          // --- 引数なし 関数定義 --- 
          report_log(3, "--parse Node FUNC_DEF, no args");
          parse_level = 1;
          expect("{");

          Node *node = new_node_func_def(tok);
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

        else if (parse_level == 1) {
          // 関数内部
          // 引数なし、関数呼び出し
          report_log(3, "--parse Node FUNC_CALL, no args");
          Node *node = new_node_func_call(tok);
          return node;
        }
      }

      // --- 引数あり 関数定義 ---  
      report_log(3, "--parse Node  FUNC_DEF with args");
      Node *node = new_node_func_def(tok);
      //report_error("NOT SUPPORTED YET");
      node->args = calloc(FUNC_ARG_MAX, sizeof(Node*));
      node->args_count = 0;
      while(1) {
        if (node->args_count >= FUNC_ARG_MAX) {
          error_at(token->str, "TOO MANY func args");
        }

        // 変数（アイデンティファイヤー）か？
        Token *tok = consume_ident();
        if (! tok) report_error(token->str, "NOT ARG var");

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

    // -- そうでない場合は、変数 --
    report_log(3, "--parse Node IDENT");
    Node *node = new_node_lvar(tok, locals_ptr);
    return node;
  }

  // そうでなければ数値のはず
  report_log(3, "--parse Node NUM");
  return new_node_num(expect_number());
}

Node *unary(LVar **locals_ptr) {
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
    report_log(2, "stmt() int");

    // 変数（アイデンティファイヤー）か？
    Token *tok = consume_ident();
    if (! tok)
      error_at(token->str, "intの後が変数名ではありません");
  

    if (!consume(";"))
      error_at(token->str, "';'ではないトークンです");

    // -- 変数宣言 --
    report_log(2, "stmt() decl_new_lvar");
    decl_new_lvar(tok, locals_ptr);
    report_log(2, "stmt() new_node_lvar");
    node = new_node_lvar(tok, locals_ptr);
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
      error_at(token->str, "';'ではないトークンです");
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