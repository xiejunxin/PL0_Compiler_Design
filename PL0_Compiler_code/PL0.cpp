#include "PL0.h"

using namespace std;

void error(int err)
{
    if (err == 5 || err == 35 || err == 39 || err == 32)
    {
        cout << "[" << rowlast << "," << collast << "]: ";
    }
    else
    {
        cout << "[" << row << "," << col << "]: ";
    }
    cout << error_msg[err] << '(' << err << ')' << endl;
    err_cnt++;
}

// 读取下一个字符
void getch()
{
    if (cur_col == cur_len)
    {
        // 行缓冲区读完的情况
        row++;
        col = 0;
        cur_col = 0;
        cur_len = 0;
        while ((!feof(infile)) && (ch = getc(infile)) != '\n')
        {
            cur_len++;
            line[cur_len] = ch;
        }
        if (feof(infile))
        {
            cur_len--;
        }
        cur_len++;
        line[cur_len] = ' ';            // 在行缓冲区末尾手动增加一个空格
    }

    col++;
    cur_col++;
    ch = line[cur_col];
}

// 词法分析
void getsym()
{
    if (feof(infile))
    {
        // 到文件尾了
        if (err_cnt == 0)
        {
            cout << "No error in the program!" << endl;
        }
        else
        {
            cout << err_cnt << " errors in the program!" << endl;
        }
        return;
    }

    rowlast = row;
    collast = col;

    while (ch == ' ' || ch == '\t')
    {
        getch();
    }

    word_len = 0;

    if (isalpha(ch))
    {
        // 字母开头的标识符的处理
        word_len = 0;
        memset(word, 0, sizeof word);
        while (isalpha(ch) || isdigit(ch))
        {
            if (word_len < IDLEN)
            {
                word[word_len] = ch;
                word_len++;
            }
            getch();
        }

        memset(id, 0, sizeof id);
        strcpy(id, word);

        int k = -1;
        for (int i = 0;i < RAWCNT;i++)
        {
            if (!strcmp(id, raw_word[i]))
            {
                k = i;
                break;
            }
        }
        if (k != -1)
        {
            sym = raw_word_sym[k];
        }
        else
        {
            sym = IDT;
        }
    }
    else if (isdigit(ch))
    {
        // 数字的处理
        word_len = 0;
        sym = NUM;
        num = 0;

        while (isdigit(ch))
        {
            num = num * 10 + (ch - '0');
            word_len++;
            getch();
        }

        if (isalpha(ch))
        {
            // 出现数字后跟字母的标识符错误
            // 省略数字重新进行标识符词法分析处理
            error(28);
            getsym();
        }

        if (ch == '.')
        {
            // 这种情况的处理有点多此一举
            // PL0中出现小数的情况
            error(27);
            getch();
            while (isdigit(ch))
            {
                getch();
            }
        }

        if (word_len > NUMLEN)
        {
            // 整数过长 相当于溢出处理
            error(31);
            num = 0;
        }
    }
    else if (ch == ':')
    {
        // 下面就是一些其他符号的处理
        word[word_len] = ch;
        getch();

        if (ch == '=')
        {
            word[++word_len] = ch;
            sym = BECOME;
            getch();
        }
        else
        {
            // 赋值号的 = 缺失
            error(29);
            word[++word_len] = '=';
            sym = BECOME;
        }
    }
    else if (ch == '<')
    {
        word[word_len] = ch;
        getch();

        if (ch == '=')
        {
            word[++word_len] = ch;
            sym = LEQ;
            getch();
        }
        else if (ch == '>')
        {
            word[++word_len] = ch;
            sym = NEQ;
            getch();
        }
        else
        {
            sym = LES;
        }
    }
    else if (ch == '>')
    {
        word[word_len] = ch;
        getch();

        if (ch == '=')
        {
            word[++word_len] == ch;
            sym = GEQ;
            getch();
        }
        else
        {
            sym = GTR;
        }
    }
    else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' || ch == '('
        || ch == ')' || ch == ',' || ch == ';')
    {
        word[word_len] = ch;
        sym = ssym[(unsigned char)ch];
        getch();
    }
    else
    {
        // 到这里的字符无法被识别了
        error(25);
        word[word_len] = ' ';
        getch();
        getsym();
    }
}

// 很关键的错误处理函数do_error
// 为了在语法分析过程中遇到错误能够继续下去
// 这里使用寻找同步符号的做法
// 对于识别到的字符和按照语法规则对应的字符不匹配的情况时
// 可以根据当前的出错情况去寻找可以来替代当前单词从而匹配语法规则
// 由于可以替代的单词组合不止一类
// 如果使用具体单词字符串来进行函数将变得非常难以设计和臃肿
// 体现了设计时使用常数宏定义唯一表示字符类型的精妙所在
// 可以利用位运算进行刻画集合关系
// s1是当前语法分析时单词应该属于的符号类型集合
// s2是需要找到可以进行下位替代从而能够继续进行语法分析的符号类型的集合
// error_id是当前人为定义的出错类型
void do_error(LL s1, LL s2, int error_id)
{
    if (!(sym & s1))
    {
        // 当前符号不在应该出现的符号类型中
        // 发出报错信息
        error(error_id);
        s1 = s1 | s2;

        // 寻找替代符号类型的集合
        while (!(sym & s1))
        {
            // 只要对应位中有一个1即可跳出循环
            // 也就是找到了这个可以替代的符号
            getsym();
        }
    }
}

// 声明相关需要的函数
// 符号表相关
bool enter(enum symtype k, char* sym_name);                                 // 登录符号表进行写入
int position(enum symtype k, char* sym_name, bool is_same_pro = false);     // 查询符号表
void exittable(int d);                                                      // 退出符号表

// 生成中间代码相关
int findbase(int sp, int l);                                                // 静态链查询基地址
void gen(enum obj_code x, int y, int z);                                    // 生成中间代码
void interpret();                                                           // 解释执行
void listpcode(int pc);                                                     // 展示p代码

// 递归下降语法分析相关
void program(LL fsys);
void block(LL fsys);
void constdeclaration();
void vardeclaration();
void proc(LL fsys);
void body(LL fsys);
void statement(LL fsys);
void condition(LL fsys);
void expression(LL fsys);
void term(LL fsys);
void factor(LL fsys);

// 递归下降进行语法分析
// <prog> → program <id>;<block>
void program(LL fsys)
{
    getsym();
    if (sym == PROGSYM)
    {
        getsym();
        if (sym == IDT)
        {
            getsym();
            if (sym == SEMICOLON)
            {
                getsym();
                block(fsys);
            }
            else if (sym & fsys)
            {
                error(5);
                block(fsys);
            }
            else
            {
                error(10);
                getsym();
                block(fsys);
            }
        }
        else
        {
            error(4);
            if (sym == SEMICOLON)
            {
                getsym();
                block(fsys);
            }
            else if (sym & fsys)
            {
                block(fsys);
            }
            else
            {
                getsym();
                if (sym == SEMICOLON)
                {
                    getsym();
                    block(fsys);
                }
                else if (sym & fsys)
                {
                    error(5);
                    block(fsys);
                }
                else
                {
                    error(10);
                    getsym();
                    block(fsys);
                }
            }
        }
    }
    else
    {
        // 对于program这一语句常出现的错误不采用同步符号或镇定规则去实现
        // 事实上程序中的一些简单场景的符号拼写问题如果使用do_error规则去进行同步匹配
        // 往往会丢失一些重要的程序信息从而造成不必要的错误匹配
        // 为了提高程序语法分析的正确性
        // 除了本处外编译器程序也会在其他一些常见的小型符号(尤其拼写上)的错误进行人工诊断和预测
        // 不调用do_error进行符号匹配而是直接人为条件判断即可
        if (sym == IDT)
        {
            // 如果是ID则继续获取一个词法单元查看类型决定是报拼写错误还是缺失
            // 而不是直接终止语法分析
            getsym();
            if (sym == IDT)
            {
                // 后面还是ID则认为是拼写错误
                error(32);
                getsym();
                if (sym == SEMICOLON)
                {
                    getsym();
                    block(fsys);
                }
                else if (sym & fsys)
                {
                    // 可以直接在此进行block分析
                    // 报分号缺失
                    error(5);
                    block(fsys);
                }
                else
                {
                    // getsym后进行block分析
                    // 报期望分号
                    error(10);
                    getsym();
                    block(fsys);
                }
            }
            else if (sym == SEMICOLON)
            {
                // 后面是分号则报缺失
                error(26);
                getsym();
                block(fsys);
            }
            // 其他情况默认不会出现
            // 不处理这种情况的错误 测试文件不应该如此编写
        }
    }
}

// <block> → [<condecl>][<vardecl>][<proc>]<body>
void block(LL fsys)
{
    int d_bias_temp = d_bias;

    int lastpro = -1;
    if (dpl_top > 1)
    {
        // dpl_top从1开始
        // 去找此block对应的前一个proc
        lastpro = display[dpl_top];
        while (table[lastpro].pre != 0)
        {
            lastpro = table[lastpro].pre;
        }
        d_bias = table[lastpro].size + 3;
    }
    else
    {
        d_bias = 3;
    }

    // 回填
    int pc_temp = pc;
    gen(JMP, 0, 0);

    if (sym == CONSTSYM)
    {
        getsym();
        constdeclaration();
    }

    // 设想的一些错误处理
    while (sym == CONSTSYM)
    {
        error(45);
        getsym();
        constdeclaration();
    }

    if (sym == VARSYM)
    {
        getsym();
        vardeclaration();
    }

    while (sym & (CONSTSYM | VARSYM))
    {
        if (sym == CONSTSYM)
        {
            error(44);
            getsym();
            constdeclaration();
        }
        else if (sym == VARSYM)
        {
            error(45);
            getsym();
            vardeclaration();
        }
    }

    if (sym == PROCSYM)
    {
        getsym();
        proc(fsys);
        cur_lev--;
    }

    while (sym & (CONSTSYM | VARSYM))
    {
        if (sym == CONSTSYM)
        {
            error(45);
            getsym();
            constdeclaration();
        }
        else if (sym == VARSYM)
        {
            error(45);
            getsym();
            vardeclaration();
        }
    }

    if (lastpro != -1)
    {
        // 参数传递生成中间代码的处理
        // 这里的3就是那3个连接数据单元
        // 参数传递是发生在过程调用时
        // 设计思想是当发生过程调用后程序会将传入参数的值入栈
        // 调用call之后会紧接着在入栈的参数往上开辟数据区
        // 此时开辟数据区时会将sp更新为新被调用过程的活动记录的正确位置(0)
        // 此时top还未更新 应该与sp-1相同
        // 然后使用sto中间代码将刚刚入栈的参数值传递到新活动记录的形参数据区
        // 有几个参数就会有几个sto指令
        // 而sto指令会使top--
        // 因而随后需要使用int开辟形参个数个数据空间回复top到sp-1的位置
        // 然后下面调用int开辟当前新的过程的活动记录所需要的数据区d_bias
        for (int i = 1;i <= table[lastpro].size;i++)
        {
            gen(STO, 0, 3 + table[lastpro].size - i);
        }
        gen(INT, 0, table[lastpro].size);
        table[lastpro].val = pc - table[lastpro].size - 1;
    }

    // 回填
    // 跳过过程定义抵达的跳转地址进行回填
    code[pc_temp].A = pc;

    // 为被调用的新过程需要开辟的数据空间
    gen(INT, 0, d_bias);

    if (sym == BEGINSYM)
    {
        getsym();
    }
    else
    {
        error(33);
    }

    body(fsys);

    // 过程结束了
    // opr 0 0 相当于return
    gen(OPR, 0, 0);

    d_bias = d_bias_temp;
}

// <condecl> → const <const>{,<const>};
// <const> → <id>:=<integer>
void const_h()
{
    if (sym == IDT)
    {
        getsym();
        if (sym == EQL || sym == BECOME)
        {
            if (sym == EQL)
            {
                error(1);
            }
            getsym();
            if (sym == NUM)
            {
                bool f = enter(con, id);
                if (f)
                {
                    // 已经在符号表注册过
                    error(46);
                }
                getsym();
            }
            else
            {
                error(2);
                // 人为考虑这里的情况不会是丢失
                // 非数字也会有一个id符进行占位
                // 直接跳过即可
                getsym();
            }
        }
        else
        {
            // 这里人为认为的错误就是丢失赋值号
            // 其他非丢失的逻辑语法错误有点过于荒谬了
            error(13);
            if (sym == NUM)
            {
                bool f = enter(con, id);
                if (f)
                {
                    // 已经在符号表注册过
                    error(46);
                }
                getsym();
            }
            else
            {
                error(2);
                getsym();
            }
        }
    }
    else
    {
        error(4);
    }
}

void constdeclaration()
{
    const_h();

    while (1)
    {
        if (sym == COMMA)
        {
            getsym();
            const_h();
        }
        else if (sym == IDT)
        {
            // 人为考虑为丢失','
            error(5);
            const_h();
        }
        else if (sym == SEMICOLON)
        {
            getsym();
            break;
        }
        else
        {
            // 其他情况就是人为丢失';'
            error(5);
            break;
        }
    }
}

// <vardecl> → var <id>{,<id>};
void var_h()
{
    if (sym == IDT)
    {
        bool f = enter(var, id);
        if (f)
        {
            error(46);
        }
        getsym();
    }
    else
    {
        // 这种情况只会是空占位才符合实际
        // 直接报缺失处理
        error(4);
    }
}

void vardeclaration()
{
    var_h();

    while (1)
    {
        if (sym == COMMA)
        {
            getsym();
            var_h();
        }
        else if (sym == IDT)
        {
            // 人为考虑丢失','
            error(5);
            var_h();
        }
        else if (sym == SEMICOLON)
        {
            getsym();
            break;
        }
        else
        {
            // 这种情况就是';'丢失符合实际
            error(5);
            break;
        }
    }
}

// <proc> → procedure <id>([<id>{,<id>}]);<block>{;<proc>}
void proc(LL fsys)
{
    int cnt = 0;
    // 暂时记录d_bias
    int d_bias_temp = d_bias;
    int dpl_top_temp = dpl_top;

    if (sym == IDT)
    {
        int i = position(pro, id);
        if (i != -1)
        {
            error(46);
            // 使用unknown作为过程名
            enter(pro, (char*)"unknown");
        }
        else
        {
            enter(pro, id);
        }
        getsym();
    }
    else
    {
        enter(pro, (char*)"unknown");
        error(4);
    }

    // 递归下降的过程中
    // 当执行到proc时层次更新
    // 符号表层次限定范围顶帝指针进行更新维护
    // 这里还需要保证过程不能重名
    table_sp = table_top;
    int table_top_temp = table_top;
    cur_lev++;

    // 这里检查左括号缺失
    // 如果缺失do_error可以在s2中匹配同步符号从而跳过错误继续分析
    do_error(LPAREN, fsys | IDT | RPAREN | COMMA | SEMICOLON, 40);
    if (sym == LPAREN)
    {
        getsym();
    }
    // 检查括号内有内容但开始内容不正确
    // 或者括号内无内容且缺失括号
    do_error(IDT | RPAREN, fsys | COMMA | SEMICOLON, 4);

    if (sym == IDT)
    {
        cnt++;
        d_bias = cnt + 3 - 1;
        enter(var, id);
        table[table_top_temp].size = cnt;
        getsym();
    }
    while (sym == COMMA)
    {
        getsym();
        if (sym == IDT)
        {
            cnt++;
            d_bias = cnt + 3 - 1;
            enter(var, id);
            table[table_top_temp].size = cnt;
            getsym();
        }
        else
        {
            error(36);
        }
    }

    if (sym == RPAREN)
    {
        getsym();
    }
    else
    {
        error(22);
    }

    if (sym == SEMICOLON)
    {
        getsym();
    }
    else
    {
        error(10);
    }

    block(fsys | SEMICOLON);

    if (dpl_top != dpl_top_temp)
    {
        // 说明此过程又嵌套定义了一个过程
        // 并且当前过程已经退出可以进行退表
        // 同时考虑本代码中符号表链式存储的结构
        // 退表时会将本过程作为子过程存入上一层符号表
        // 在if条件相等时是不需要退表的
        // 这样可以维护某一层次下的过程信息方便查表
        exittable(dpl_top_temp);
    }

    do_error(SEMICOLON | BEGINSYM, fsys | PROCSYM | stat_begin_sym | ENDSYM, 10);

    while (sym & (SEMICOLON | PROCSYM))
    {
        if (sym != PROCSYM)
        {
            getsym();
        }

        if (sym == PROCSYM)
        {
            getsym();
            proc(fsys);
        }
        else
        {
            error(43);
        }
    }

    d_bias = d_bias_temp;
}

// <body> → begin <statement>{;<statement>}end
void body(LL fsys)
{
    if (sym == ENDSYM)
    {
        error(39);
        getsym();
        return;
    }

    do_error(IDT | stat_begin_sym | ENDSYM, fsys | SEMICOLON, 42);
    if (sym == SEMICOLON)
    {
        getsym();
    }

    statement(fsys | SEMICOLON | ENDSYM);

    if (sym == ENDSYM)
    {
        getsym();
    }
    else
    {
        if (sym == SEMICOLON)
        {
            getsym();
            body(fsys | SEMICOLON | ENDSYM);
        }
        else
        {
            do_error(fsys, stat_begin_sym, 17);
            body(fsys | SEMICOLON | ENDSYM);
        }
    }
}

// <exp> → [+|-]<term>{<aop><term>}
// <aop> → +|-
void expression(LL fsys)
{
    LL addop;
    if (sym == PLUS || sym == MINUS)
    {
        addop = sym;
        getsym();
        term(fsys | PLUS | MINUS);
        if (addop == MINUS)
        {
            // 取反运算生成相应的opr中间代码
            gen(OPR, 0, 1);
        }
    }
    else
    {
        term(fsys | PLUS | MINUS);
    }

    while (sym == PLUS || sym == MINUS)
    {
        addop = sym;
        getsym();
        term(fsys | PLUS | MINUS);

        if (addop == PLUS)
        {
            // +运算生成相应的opr中间代码
            gen(OPR, 0, 2);
        }
        else
        {
            // -运算生成相应的opr中间代码
            gen(OPR, 0, 3);
        }
    }
}

// <factor>→<id>|<integer>|(<exp>)
void factor(LL fsys)
{
    int i;
    do_error(factor_begin_sym, fsys, 24);

    if (sym == IDT)
    {
        i = position(cav, id);
        if (i == -1)
        {
            error(11);
        }
        else
        {
            if (table[i].kind == con)
            {
                // 项为常数 直接从符号表获取相应的常数值入栈
                gen(LIT, 0, table[i].val);
            }
            else if (table[i].kind == var)
            {
                // 项为变量 
                // 直接从符号表中符号所在的层次和当前代码所在的层次计算层差
                // 对应符号中的addr属性记录符号所在层次的相对地址
                // 生成lod中间代码
                gen(LOD, cur_lev - table[i].level, table[i].addr);
            }
        }
        getsym();
    }
    else if (sym == NUM)
    {
        // 如果本身就是数字直接生成lit中间代码即可
        gen(LIT, 0, num);
        getsym();
    }
    else if (sym == LPAREN)
    {
        getsym();
        expression(RPAREN | fsys);
        if (sym == RPAREN)
        {
            getsym();
        }
        else
        {
            error(22);
        }
        do_error(fsys, LPAREN, 23);
    }
}

// <term> → <factor>{<mop><factor>}
// <mop> → *|/
void term(LL fsys)
{
    LL mulop;
    factor(fsys | MUL | DIV);
    while (sym == MUL || sym == DIV)
    {
        mulop = sym;
        getsym();
        factor(fsys | MUL | DIV);
        if (mulop == MUL)
        {
            // *运算直接生成对应的中间代码
            gen(OPR, 0, 4);
        }
        else
        {
            // /运算直接生成对应的中间代码
            gen(OPR, 0, 5);
        }
    }
}

// <lexp> → <exp> <lop> <exp>|odd <exp>
// <lop> → =|<>|<|<=|>|>=
void condition(LL fsys)
{
    LL relop;
    if (sym == ODDSYM)
    {
        getsym();
        expression(fsys);
        // odd运算直接生成对应的中间代码
        gen(OPR, 0, 6);
    }
    else
    {
        expression(fsys | lop_begin_sym);
        if (!(sym & lop_begin_sym))
        {
            error(20);
            relop = EQL;
            expression(fsys);
        }
        else
        {
            relop = sym;
            getsym();
            expression(fsys);
        }
        switch (relop)
        {
            // 以下六个条件运算直接生成对应的中间代码
        case EQL:
            gen(OPR, 0, 7);
            break;
        case NEQ:
            gen(OPR, 0, 8);
            break;
        case LES:
            gen(OPR, 0, 9);
            break;
        case LEQ:
            gen(OPR, 0, 10);
            break;
        case GTR:
            gen(OPR, 0, 11);
            break;
        case GEQ:
            gen(OPR, 0, 12);
            break;
        default:
            break;
        }
    }
}

/* <statement> → <id> := <exp>
| if <lexp> then <statement>[else <statement>]
| while <lexp> do <statement>
| call <id>（[<exp>{, <exp>}]）
| <body>
| read(<id>{，<id>})
| write(<exp>{, <exp>}) */
void statement(LL fsys)
{
    int i;
    if (sym == IDT)
    {
        i = position(var, id);
        if (i == -1)
        {
            error(11);
        }

        getsym();
        if (sym == BECOME)
        {
            getsym();
        }
        else
        {
            error(13);
        }

        expression(fsys | ENDSYM);
        if (i != -1)
        {
            // 要进行赋值运算
            // 需要将对应的值从栈中赋给对应的变量
            // 因此将栈顶值出栈传给变量即可
            // 此变量的层差和相对地址可以通过符号表进行获取
            gen(STO, cur_lev - table[i].level, table[i].addr);
        }
    }
    else if (sym == IFSYM)
    {
        int pc1;
        int pc2;

        getsym();
        condition(fsys | THENSYM | ELSESYM | ENDSYM);

        if (sym == THENSYM)
        {
            getsym();
        }
        else
        {
            error(16);
        }

        // 条件判断语句需要回填技术
        // pc1回填进入条件判断语句的语句块中
        pc1 = pc;
        gen(JPC, 0, 0);

        statement(fsys | ENDSYM);

        // pc2回填if条件判断语句块的下一条指令地址
        // 即跳过else部分跳出if语句块
        pc2 = pc;
        gen(JMP, 0, 0);

        // 回填
        // 知道了if发生跳转的地址
        code[pc1].A = pc;

        if (sym == ELSESYM)
        {
            getsym();
            statement(fsys | ENDSYM);
        }
        // 回填
        // 知道了if不发生时跳转的地址
        code[pc2].A = pc;
        // 此处不做else缺失的检查 如果缺失else 会合理转向body缺失分号的恢复 报错17
    }
    else if (sym == WHILESYM)
    {
        int pc1;
        int pc2;

        getsym();

        // pc1记录while条件判断的语句入口处
        // pc2记录while语句块执行的中间代码下标
        // 用来回填条件语句判断时要退出循环应该跳转到的地址
        pc1 = pc;
        condition(fsys | DOSYM | ENDSYM);
        pc2 = pc;
        // while语句和if类似需要回填技术
        // 当循环退出时需要回填
        gen(JPC, 0, 0);

        if (sym == DOSYM)
        {
            getsym();
        }
        else
        {
            error(18);
        }

        statement(fsys);

        // 执行完语句块后回到while条件判断处
        // 直接生成相应的条件跳转中间代码
        gen(JMP, 0, pc1);
        // 回填循环退出时应该跳转到的地址
        // 循环退出的地址回填为pc
        code[pc2].A = pc;
    }
    else if (sym == CALLSYM)
    {
        getsym();
        int cnt = 0;

        if (sym != IDT)
        {
            do_error(IDT, fsys, 14);
        }
        else
        {
            // i是调用过程的在符号表中的id
            i = position(pro, id);
            getsym();
            do_error(LPAREN, fsys | (factor_begin_sym | PLUS | MINUS) | RPAREN | COMMA | SEMICOLON | ENDSYM, 40);
            if (sym == LPAREN)
            {
                getsym();
            }
            do_error((factor_begin_sym | PLUS | MINUS) | RPAREN, fsys | COMMA | SEMICOLON | ENDSYM, 41);

            if (sym & (factor_begin_sym | PLUS | MINUS))
            {
                expression(fsys | RPAREN | PLUS | MINUS | COMMA | ENDSYM);
                cnt++;
            }
            while (sym == COMMA)
            {
                getsym();
                expression(fsys | RPAREN | PLUS | MINUS | COMMA | ENDSYM);
                cnt++;
            }

            if (sym == RPAREN)
            {
                getsym();
            }
            else
            {
                error(22);
            }

            if (i == -1)
            {
                error(11);
            }
            else
            {
                // call发生过程调用
                // 进行参数传递
                if (cnt != table[i].size)
                {
                    error(48);
                    if (cnt > table[i].size)
                    {
                        // 多则取前
                        pc = pc - (cnt - table[i].size);
                    }
                    else if (cnt < table[i].size)
                    {
                        // 少则填0
                        int l = table[i].size - cnt;
                        while (l--)
                        {
                            // 将0入栈进行参数传递
                            gen(LIT, 0, 0);
                        }
                    }
                }
                // 生成cal中间代码
                // i是相应过程的符号表
                // 对于过程符号
                // 符号表中的val值存放的是这个过程所在层次的地址
                gen(CAL, cur_lev - table[i].level, table[i].val);
            }

        }
    }
    else if (sym == BEGINSYM)
    {
        getsym();
        body(fsys);
    }
    else if (sym == READSYM)
    {
        getsym();

        do_error(LPAREN, fsys | IDT | COMMA | RPAREN | SEMICOLON | ENDSYM, 40);
        if (sym == LPAREN)
        {
            getsym();
        }
        do_error(IDT, fsys | COMMA | RPAREN | SEMICOLON | ENDSYM, 4);

        if (sym == IDT)
        {
            i = position(var, id);
            if (i == -1)
            {
                error(11);
            }
            else
            {
                // red指令从命令行读取数据到对应变量中
                // i是对应变量在符号表中的下标
                // 根据符号表中的层次信息计算层差和相对地址即可生成相应的red中间代码
                gen(RED, cur_lev - table[i].level, table[i].addr);
            }
            getsym();
        }

        while (sym == COMMA)
        {
            getsym();
            if (sym == IDT)
            {
                i = position(var, id);
                if (i == -1)
                {
                    error(11);
                }
                else
                {
                    // 同上
                    gen(RED, cur_lev - table[i].level, table[i].addr);
                }
                getsym();
            }
            else
            {
                error(36);
            }
        }
        if (sym == RPAREN)
        {
            getsym();
        }
        else
        {
            error(22);
        }
    }
    else if (sym == WRITESYM)
    {
        getsym();

        do_error(LPAREN, fsys | RPAREN | (factor_begin_sym | PLUS | MINUS) | COMMA | ENDSYM, 40);
        if (sym == LPAREN)
        {
            getsym();
        }
        expression(fsys | RPAREN | PLUS | MINUS | COMMA | ENDSYM);
        // 将栈顶值输出到命令行
        // 直接生成相应的中间代码即可
        gen(WRT, 0, 0);
        while (sym == COMMA)
        {
            getsym();
            expression(fsys | RPAREN | PLUS | MINUS | COMMA | ENDSYM);
            gen(WRT, 0, 0);
        }
        // 调用write后要打印回车
        // 增加一个opr指令
        gen(OPR, 0, 13);
        if (sym == RPAREN)
        {
            getsym();
        }
        else
        {
            error(22);
        }
    }
    else if (sym == ENDSYM)
    {
        col -= 3;
        error(35);
        col += 3;
        getsym();
    }
}

// 符号登记注册符号表
// 进行符号表管理最关键的一个函数
// 需要注意这里符号管理以及层次关系的全局维护
// 返回值使用bool用以确定该符号是否已经注册过 重定义问题
bool enter(enum symtype k, char* sym_name)
{
    bool is_exist = 0;
    if (table_top >= TABLELEN)
    {
        error(47);
        return false;
    }
    // 第三个same_pro参数传递主要是为了
    // 同层区域内不允许出现重复定义
    // 控制在同层区域内
    int i = position(k, sym_name, true);
    if (i == -1)
    {
        table_top++;
        i = table_top;
    }
    else
    {
        is_exist = 1;
    }
    // 如果同层内is_exist为真那就直接在符号表内进行覆盖了
    table[i].kind = k;
    strcpy(table[i].name, sym_name);
    if (is_exist)
    {
        if (k == con)
        {
            // 值覆盖即可
            table[i].val = num;
            return is_exist;
        }
        else if (k == var)
        {
            // 变量无初始化
            // 这种情况直接返回即可
            return is_exist;
        }
    }

    switch (k)
    {
    case con:
        // 常量类型
        table[i].val = num;
        table[i].level = cur_lev;
        table[i].pre = last;
        if (last == i && is_exist)
        {
            // 防止自己指向自己从而造成符号表遍历死循环
            // 当间隔出现重定义时会出错 解决办法见上的条件判断
            table[i].pre = llast;
        }
        display[dpl_top] = i;
        break;
    case var:
        // 变量类型
        table[i].level = cur_lev;
        table[i].addr = d_bias;
        d_bias++;
        table[i].pre = last;
        if (last == i && is_exist)
        {
            table[i].pre = llast;
        }
        display[dpl_top] = i;
        break;
    case pro:
        // 过程类型
        table[i].level = cur_lev;
        // pro
        table[i].pre = 0;
        table[i].size = 0;
        // dpl_top只会在类型为pro的符号注册时增加
        // 退表时层次的变化也需要减少
        dpl_top++;
        display[dpl_top] = i;
        break;
    default:
        break;
    }
    llast = last;
    last = i;

    return is_exist;
}

int position(enum symtype k, char* sym_name, bool is_same_pro)
{
    // 需要遍历符号表进行符号定位和查找
    // 找到为0 找不到为1
    bool f = 1;
    int i = display[dpl_top];
    int dpl_top_temp = dpl_top;
    if (is_same_pro)
    {
        // 同一个过程下
        if (k == cav)
        {
            while ((strcmp(sym_name, table[i].name) != 0 || table[i].kind == pro) && i != 0)
            {
                i = table[i].pre;
            }
        }
        else
        {
            while ((strcmp(sym_name, table[i].name) != 0 || table[i].kind != k) && i != 0)
            {
                i = table[i].pre;
            }
        }
        if (i != 0)
        {
            f = 0;
        }
    }
    else
    {
        // 如果不在同一个过程下
        // 那么就需要到不同层次下进行符号的查询了
        // 控制dpl_top_temp进行循环查找
        while (dpl_top_temp && f)
        {
            // 循环查表
            i = display[dpl_top_temp];
            if (k == cav)
            {
                while ((strcmp(sym_name, table[i].name) != 0 || table[i].kind == pro) && i != 0)
                {
                    i = table[i].pre;
                }
            }
            else
            {
                while ((strcmp(sym_name, table[i].name) != 0 || k != table[i].kind) && i != 0)
                {
                    i = table[i].pre;
                }
            }
            if (i != 0)
            {
                f = 0;
                break;
            }
            dpl_top_temp--;
        }
    }

    if (f)
    {
        return -1;
    }
    return i;
}

void exittable(int d)
{
    // 退表
    // 当过程退出后子符号表需要删除
    // 子过程作为变量存入上一层符号表
    // 过程其实就是模拟链表的删除
    // 后向式链表结构
    if (d > dpl_top)
    {
        return;
    }
    int i;
    while (d < dpl_top)
    {
        // 将层次从d+1-dpl_top的所有层次表全部删除
        // 就是将这些层pre为0即类型为proc的表项
        // 直接接入层次为d的符号表中
        i = display[dpl_top];
        while (table[i].pre != 0)
        {
            i = table[i].pre;
        }
        dpl_top--;
        table[i].pre = display[d];
        display[d] = i;
    }
    // 上述退表的循环过程就是不断修正待删除层次之间的符号表的pre指针关系
    // 退完表之后维护的层次符号表的顶层指针被底层指针覆盖
    table_top = table_sp;
}

void gen(enum obj_code x, int y, int z)
{
    code[pc].F = x;
    code[pc].L = y;
    code[pc].A = z;
    pc++;
}

// 输出一些代码信息
void listpcode(int p)
{
    for (int i = p;i <= pc;i++)
    {
        cout << codename[code[i].F] << ' ' << code[i].L << ' ' << code[i].A << endl;
    }
}

void printcode()
{
    for (int i = 0;i < pc;i++)
    {
        cout << i << ": " << codename[code[i].F] << ' ' << code[i].L << ' ' << code[i].A << endl;
    }
}

void printtable()
{
    cout << "Kind Name Val Level Addr Size Pre" << endl;
    for (int i = 1;i <= table_top;i++)
    {
        cout << table[i].kind << ' ' << table[i].name << ' ' << table[i].val << ' ' << table[i].level << ' '
            << table[i].addr << ' ' << table[i].size << ' ' << table[i].pre << endl;
    }
}

// 通过静态链求出数据区基地址的函数
// l为层差
int findbase(int sp, int l)
{
    int ret_sp;
    ret_sp = sp;
    while (l > 0)
    {
        // 静态链
        ret_sp = datastack[ret_sp + 2];
        l--;
    }
    return ret_sp;
}

// 解释执行过程
void interpret()
{
    int t;
    cout << "*********** PL/0 Program INTERPRET START ! ***********" << endl;

    pc = 0;
    sp = 0;
    top = -1;
    datastack[0] = datastack[1] = datastack[2] = 0;

    do
    {
        ins = code[pc];
        pc++;
        switch (ins.F)
        {
        case LIT:
            // 将常量入栈顶
            top++;
            datastack[top] = ins.A;
            break;
        case OPR:
            // 执行运算指令
            switch (ins.A)
            {
            case 0:
                // return
                top = sp - 1;
                pc = datastack[sp + 1];
                sp = datastack[sp]; // 动态链
                break;
            case 1:
                // 取反
                datastack[sp] = -datastack[sp];
                break;
            case 2:
                // 加法
                top = top - 1;
                datastack[top] = datastack[top] + datastack[top + 1];
                break;
            case 3:
                // 减法
                top = top - 1;
                datastack[top] = datastack[top] - datastack[top + 1];
                break;
            case 4:
                // 乘法
                top = top - 1;
                datastack[top] = datastack[top] * datastack[top + 1];
                break;
            case 5:
                // 除法
                top = top - 1;
                datastack[top] = datastack[top] / datastack[top + 1];
                break;
            case 6:
                // odd
                datastack[top] = datastack[top] % 2;
                break;
            case 7:
                // ==
                top = top - 1;
                datastack[top] = (datastack[top] == datastack[top + 1]);
                break;
            case 8:
                // !=
                top = top - 1;
                datastack[top] = (datastack[top] != datastack[top + 1]);
                break;
            case 9:
                // <
                top = top - 1;
                datastack[top] = (datastack[top] < datastack[top + 1]);
                break;
            case 10:
                // <=
                top = top - 1;
                datastack[top] = (datastack[top] <= datastack[top + 1]);
                break;
            case 11:
                // >
                top = top - 1;
                datastack[top] = (datastack[top] > datastack[top + 1]);
                break;
            case 12:
                // >=
                top = top - 1;
                datastack[top] = (datastack[top] >= datastack[top + 1]);
                break;
            case 13:
                // 打印回车
                cout << endl;
                break;
            }
            break;
        case LOD:
            // 将变量的值入栈
            top = top + 1;
            datastack[top] = datastack[findbase(sp, ins.L) + ins.A];
            break;
        case STO:
            // 将栈顶的值出栈送给变量
            top = top - 1;
            datastack[findbase(sp, ins.L) + ins.A] = datastack[top + 1];
            break;
        case CAL:
            // 过程调用
            datastack[top + 1] = sp;    // 动态链
            datastack[top + 2] = pc;    // 返回地址
            datastack[top + 3] = findbase(sp, ins.L);   // 静态链 L是调用过程的层差
            sp = top + 1;
            pc = ins.A;
            break;
        case INT:
            // 开辟空间
            top = top + ins.A;
            break;
        case JMP:
            // 无条件跳转
            pc = ins.A;
            break;
        case JPC:
            // 条件跳转
            top = top - 1;
            if (datastack[top + 1] == 0)
            {
                // 栈顶值为假时则跳转
                pc = ins.A;
            }
            break;
        case RED:
            // 从键盘读取输入
            cout << "Please Input the value of the variable:";
            cin >> t;
            datastack[findbase(sp, ins.L) + ins.A] = t;
            break;
        case WRT:
            // 输出
            cout << datastack[top] << ' ';
            break;
        default:
            break;
        }
    } while (pc != 0);
    cout << "*********** PL/0 Program INTERPRET END ! ***********" << endl;
}

// 主函数
// 启动 PL/0 编译器
int main()
{
    for (int i = 0;i < 300;i++)
    {
        ssym[i] = NIL;
    }
    ssym['+'] = PLUS;
    ssym['-'] = MINUS;
    ssym['*'] = MUL;
    ssym['/'] = DIV;
    ssym['('] = LPAREN;
    ssym[')'] = RPAREN;
    ssym['='] = EQL;
    ssym[','] = COMMA;
    ssym[';'] = SEMICOLON;

    decl_begin_sym = CONSTSYM | VARSYM | PROCSYM;
    stat_begin_sym = BEGINSYM | CALLSYM | IFSYM | WHILESYM | READSYM | WRITESYM | IDT;
    factor_begin_sym = IDT | NUM | LPAREN;
    lop_begin_sym = EQL | NEQ | LES | LEQ | GTR | GEQ;

    cout << "Please Input the PL/0 Program File Name: ";
    cin >> infilename;
    cout << endl;

    if ((infile = fopen(infilename, "r")) == NULL)
    {
        cout << "File Open Fail! " << endl;
        exit(-1);
    }

    table_top = 0;
    cur_lev = 0;
    dpl_top = 1;
    display[dpl_top] = 1;
    err_cnt = 0;

    cur_col = 0;
    pc = 0;
    cur_len = 0;
    ch = ' ';
    row = 0;
    col = 0;
    llast = last = 0;
    rowlast = collast = 0;

    program(decl_begin_sym | stat_begin_sym);

    int f;
    cout << "Please input 1 or 0 to choose whether show PCode: ";
    cin >> f;
    if (f)
    {
        printcode();
    }
    cout << endl;

    cout << "Please input 1 or 0 to choose whether show Symbol Table: ";
    cin >> f;
    if (f)
    {
        printtable();
    }
    cout << endl;

    if (err_cnt == 0)
    {
        // 没有发生错误时进行解释执行
        interpret();
    }
    else
    {
        cout << "Errors Occur ! " << endl;
        cout << "Interpret Interrupt ! " << endl;
    }

    fclose(infile);
    system("pause");
    return 0;
}