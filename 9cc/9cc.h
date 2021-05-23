
// ---- token ----

// トークンの種類
typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_NUM,       // 整数トークン
  TK_TYPE_INT,  // int型
  TK_SIZEOF,    // sizeof演算子
  TK_EOF,       // 入力の終わりを表すトークン
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


// --- type ---

typedef struct Type Type;

struct Type {
  enum { INT, PTR } ty;
  struct Type *ptr_to;
};

// --- local variable ---

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  Type *type; // 型
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

// ローカル変数
//extern LVar *locals;
  
// --- node ---
#define BLOCK_LINE_MAX 100
#define FUNC_ARG_MAX 6

// 抽象構文木のノードの種類
typedef enum {
  ND_NONE, // None
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
  ND_RETURN, // return
  ND_IF, // if
  //ND_ELSE, // else
  ND_WHILE, // while
  ND_FOR, // for
  ND_BLOCK, // {} block
  ND_FUNC_CALL, // call function
  ND_FUNC_DEF, // define function
  ND_LVAR,   // ローカル変数
  ND_ADDR, // アドレス　&
  ND_DEREF, // アドレスの指す中身 *
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  Node *cond; // if/whileの条件, forの条件(2番目)
  Node *body; // 実行部 (ifのthen, while/forの実行部, func defの中身)
  Node *elsebody; // ifの場合、else部分
  Node *init; // forの初期化(1番目)
  Node *post; // forの続行処理(3番目)
  Node **stmts; // blockの場合に、連続するstmtsの配列を持つ(Node * 100);
  int stmts_count; // blockの場合に、含まれるstmtsの数
  char *func_name; // call/def funcion()の場合、function名
  Node **args; // call/def function()の場合の引数の配列 (MAX 6)
  int args_count; // call/def function()の場合の引数の数
  Type* func_type; // 関数の戻り値の型
  LVar *func_locals; // def function()の場合に、関数内のローカル変数 
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
  LVar *lvar;    // kindがND_LVARの場合のみ使う
};


//Node *expr();
void program(LVar **locals_ptr);

// ローカル変数の数を返す
int count_lvar(LVar *locals);
 
// -- 型を判定 --
Type *type_of(Node *node);

// -- サイズを判定 --
int calc_size(Node *node);

// 現在着目しているトークン （外部を参照）
extern Token *token;

// 入力プログラム （外部を参照）
extern char *user_input;

// --- コード全体 ---
#define CODE_LINE_MAX 100

extern Node *code[CODE_LINE_MAX];



// ---- code generator ---

void gen(Node *node);


