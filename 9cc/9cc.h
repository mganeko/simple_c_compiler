
// ---- token ----

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};


// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p);

// --- local variable ---

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

// ローカル変数
extern LVar *locals;
  
// --- node ---

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ, // ==
  ND_NE, // !=
  ND_GT, // >
  ND_LT, // <
  ND_GE, // >=
  ND_LE, // <=
  ND_ASSIGN, // = (代入)
  ND_LVAR,   // ローカル変数
  ND_NUM, // 整数
} NodeKind;


typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
};


//Node *expr();
void program();

// 現在着目しているトークン （外部を参照）
extern Token *token;

// 入力プログラム （外部を参照）
extern char *user_input;

// --- コード全体 ---
extern Node *code[100];

// ---- code generator ---

void gen(Node *node);


