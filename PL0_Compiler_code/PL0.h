#ifndef __PL0_H__
#define __PL0_H__

#include <bits/stdc++.h>

typedef long long LL;

#define RAWCNT 15               // 保留字数量
#define NUMLEN 14               // 数字最大长度
#define IDLEN 20                // 标识符最大长度

// 处理上为各种类型和符号以类似魔数的宏定义
// 方便处理
#define NIL 0x1                 // 空
#define IDT 0x2                 // 标识符id
#define NUM 0x4                 // 数值
#define PLUS 0x8                // +
#define MINUS 0x10              // -
#define MUL 0x20                // *
#define DIV 0x40                // /
#define ODDSYM 0x80             // 奇数
#define EQL 0x100               // =
#define NEQ 0x200               // <>
#define LES 0x400               // <
#define LEQ 0x800               // <=
#define GTR 0x1000              // >
#define GEQ 0x2000              // >=
#define LPAREN 0x4000           // (
#define RPAREN 0x8000           // )
#define COMMA 0x10000           // ,
#define SEMICOLON 0x20000       // ;
#define READSYM 0x40000         // read
#define BECOME 0x80000          // :=
#define BEGINSYM 0x100000       // begin
#define ENDSYM 0x200000         // end
#define IFSYM 0x400000          // if
#define THENSYM 0x800000        // then
#define WHILESYM 0x1000000      // while
#define DOSYM   0x2000000       // do
#define CALLSYM 0x4000000       // call
#define CONSTSYM 0x8000000      // const
#define VARSYM 0x10000000       // var
#define PROCSYM 0x20000000      // procedure
#define ELSESYM 0x40000000      // else
#define PROGSYM 0x80000000      // program
#define WRITESYM 0x100000000    // write

// 所有报错的提示信息
const char* error_msg[] =
{
    /* 00 */ "",
    /* 01 */ "Found '=' but expecting ':='.",
    /* 02 */ "Need a number to follow ':='.",
    /* 03 */ "Need a '=' to follow the identifier.",
    /* 04 */ "Need an identifier to follow or unexpected ','.",
    /* 05 */ "Missing ',' or ';'.",
    /* 06 */ "Wrong procedure name.",
    /* 07 */ "Statement expected.",
    /* 08 */ "Wrong symbol follow the statement.",
    /* 09 */ "'.' expected.",
    /* 10 */ "';' expected.",
    /* 11 */ "Undeclared identifier or incorrect type.",
    /* 12 */ "Illegal assignment.",
    /* 13 */ "Missing ':=' or ':=' expected.",
    /* 14 */ "Need an identifier to follow the 'call'.",
    /* 15 */ "Cannot call a constant or variable.",
    /* 16 */ "'then' expected.",
    /* 17 */ "';' or 'end' expected.",
    /* 18 */ "'do' expected.",
    /* 19 */ "Wrong symbol.",
    /* 20 */ "Relative operators expected.",
    /* 21 */ "Procedure identifier cannot in an expression.",
    /* 22 */ "Missing ')'.",
    /* 23 */ "The symbol cannot be followed by a factor.",
    /* 24 */ "The symbol cannot be the begin of an expression.",
    /* 25 */ "Unexpected unrecognized characters.",
    /* 26 */ "Missing 'program' or ID after it.",
    /* 27 */ "Unsupported non-integer.",
    /* 28 */ "Unqualified identifier with numeric begin.",
    /* 29 */ "Found ':' but expecting ':='.",
    /* 30 */ "Need an identifier to follow 'const'.",
    /* 31 */ "Too great number.",
    /* 32 */ "Spell error in 'program'.",
    /* 33 */ "Missing 'begin'.",
    /* 34 */ "Extra ending.",
    /* 35 */ "Unexpected ';'.",
    /* 36 */ "Unexpected ',' or missing identifier to follow ','.",
    /* 37 */ "Unexpected ',' or missing expression to follow ','.",
    /* 38 */ "Found number but expecting identifier.",
    /* 39 */ "Missing statement part.",
    /* 40 */ "Missing '('.",
    /* 41 */ "Need an expression to follow or unexpected ','.",
    /* 42 */ "The symbol cannot be the begin of a statement.",
    /* 43 */ "Missing 'procedure'.",
    /* 44 */ "Unexpected constant follow the variable.",
    /* 45 */ "Unexpected constant or variable.",
    /* 46 */ "Multiple definition.",
    /* 47 */ "Too many symbols.",
    /* 48 */ "Incorrect number of the parameters.",
    /* 49 */ "Unknown error.",
};

// 枚举表示假象目标代码类型
enum obj_code
{
    LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, RED, WRT
};

// 定义每条目标机代码格式
typedef struct
{
    enum obj_code F;            // 伪操作码
    int L;                      // 层次差
    int A;                      // 位移量（相对地址）
}command;

// 方便输出
char codename[10][5] =
{
    "LIT","OPR","LOD","STO","CAL","INT","JMP","JPC","RED","WRT"
};

// 符号表相关
#define TABLELEN 50000          // 符号表最大长度
#define DPLLEN 5000             // 层次表最大长度 

// 枚举定义符号类型
enum symtype
{
    con, var, pro, cav          // cav = con + var
};

// 符号表所需结构 静态表
struct TABLE
{
    symtype kind;
    char name[IDLEN];
    int val;
    int level;
    int addr;
    int size;
    int pre;
};

TABLE table[TABLELEN];

int display[DPLLEN];            // 层次表 控制层次关系
int dpl_top;                    // 层次表栈顶寄存器
// 这里的层次表是用来维护在分析代码的过程中
// 记录当前层次下(此表对应的下标)范围内最后一个注册登记的符号
// 对于链式维护的符号表可以实现在一个固定范围内快速查找对应层次的过程符号
// 进而避免了全局符号查询带来的效率负担

int table_top = 0;              // 符号表栈顶寄存器 可以理解为就是一个控制下标
int table_sp;                   // 同层符号表基地址寄存器
// 使用两个顶层指针和底层指针维护层次区间的符号表
// 限定范围的符号查找加快查找效率

int last;                       // 指向previous
int llast;                      // 指向previous of previous

char ch;                        // 词法分析 当前读出的字符
LL sym;                         // 当前要分析的词法单元的符号类型
char id[IDLEN];                 // 当前要分析的词法单元的标识符
int num;                        // 当前要分析的数值

int row;                        // 指示行数
int col;                        // 指示列数
int cur_col;                    // 当前行缓冲区的列指针
int cur_len;                    // 当前行缓冲区的长度
int err_cnt;                    // 记录出错总次数
int word_len;                   // 当前分析的单词长度
int cur_lev;                    // 当前分析的层深度                    
int rowlast;                    // 记录上一个单词的位置用于报错
int collast;

int d_bias;                     // 记录当前变量运行栈中相对于基地址的偏移量

char line[100];                 // 所说的行缓冲区 读取代码文件时读出一行进行分析
char word[IDLEN];               // 当前正在分析的词

// 保留字表
char raw_word[RAWCNT][IDLEN] =
{
    "begin","call","const","do","else","end","if","odd",
    "procedure","program","read","then","var","while","write"
};

// 保留字对应的符号类型表
LL raw_word_sym[RAWCNT] =
{
    BEGINSYM,CALLSYM,CONSTSYM,DOSYM,ELSESYM,ENDSYM,IFSYM,ODDSYM,
    PROCSYM,PROGSYM,READSYM,THENSYM,VARSYM,WHILESYM,WRITESYM
};

// 其他符号对应的符号类型表
LL ssym[300];

LL decl_begin_sym;              // 声明开始 表达式开始 项开始符号集合 用作错误处理
LL stat_begin_sym;
LL factor_begin_sym;
LL lop_begin_sym;

// 输入的代码文件
char infilename[300];
FILE* infile;

// 假象目标机的结构
#define CODLEN 5000
#define STACKSIZE 4096

// 两个存储器
command code[CODLEN];           // 存放p代码的code数组存储器
int datastack[STACKSIZE];       // 数据存储器datastack

// 四个寄存器
command ins;                    // 指令寄存器
int top;                        // 栈顶指示器寄存器
int sp;                         // 基地址寄存器
int pc;                         // 程序地址寄存器

#endif