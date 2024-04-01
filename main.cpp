#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <cctype>
#include <stack>
#include <algorithm>
#include <fstream>
#include <cstring>

using namespace std;

ifstream input_f;

void Error(string str1, string str2) {
    cout << str1 << " " << str2 << endl;
}

void Text(string str1, string str2) {
    cout << str1 << " -- " << str2 << endl;
}

/* enum */

enum LexType {
    LNULL,// 0
    BOOL, BREAK, GOTO, ELSE, FALSE, FOR, READ, IF, // 8
    
    NUMBER/*int*/, NULLPTR, OBJECT, RETURN, STRING, TRUE, // 14
    UNDEFINED, WHILE, WRITE, INT, // 18
    FIN, // 19
    SEMICOLON, COMMA, COLON, DOT, LPAREN, RPAREN, LQPAREN, RQPAREN, BEGIN, END, // 29
    EQ, DEQ, LSS, GTR, PLUS, PLUSEQ, DPLUS, MINUS, MINUS_EQ, DMINUS, // 39
    TIMES, TIMES_EQ, TIMES_SLASH, SLASH, SLASH_EQ, SLASH_TIMES, DSLASH, PERCENT, PERCENT_EQ, // 48
    LEQ, NOT, NEQ, GEQ, OR, DPIPE, AND, DAMP, // 56
    NUMB, // 57
    STR_CONST, // 58
    ID, // 59
    PLABEL, PADDRESS, PGO, PFGO, PLEFTINC, PRIGHTINC,  PLEFTDEC, PRIGHTDEC, PWRITE, PREAD// 69
};

enum state{ H, IDENT, NUM, COM, COM2, COM3, SLSH, EEQ, EEQ2, PLUSS, MINUSS, AMP, PIPE, QUOTE };

const char* words[] = { "", "boolean", "break", "goto", "else", "false", "for", "read", "if", "int", "null", "Object", "return", "string", "true", "undefined", "while", "write", NULL };

const char* tokens[] = { "@", ";", ",", ":", ".", "(", ")", "[", "]", "{", "}", "=", "==", "<", ">", "+", "+=", "++", "-", "-=", "--", "*", "*=", "*/", "/", "/=", "/*", "//", "%", "%=", "<=", "!", "!=", ">=", "|", "||", "&", "&&", NULL };

/* Ident */

class Ident {
    string name;
    LexType type;
    int value;

    bool assign;
    bool declare;
public:
    Ident(): assign(false), declare(false) {}
    Ident(const string n) : name(n), assign(false), declare(false) {}
    bool operator==(const string& s) const { return name == s; }

    string get_str() const { return name;}
    LexType get_type() const { return type;}
    int get_value() const { return value;}
    bool get_assign() const { return assign;}
    bool get_declare() const { return declare;}

    void set_type(LexType t) { type = t;}
    void set_value(int v) { value = v;}
    void set_assign() { assign = true;}
    void set_declare() { declare = true;}
    void set_string(string str) { name = str;}
};

vector<Ident> TID;

int place_in_tid(const string& buf){
    vector<Ident>::iterator k;
    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end()) return k - TID.begin();
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

/* Lex */

class Lex {
    LexType type;
    int val;
    string str;
public:
    Lex(LexType t = LNULL, int v = 0, string s = "") : type(t), val(v), str(s) {}
    LexType get_type() const { return type;}
    int get_value() const { return val;}
    string get_str() const { return str;}
    void Print();

    void set_type(LexType ntype) { type = ntype;}
    void set_value(int nval) {val = nval;}
    void set_string(string nstr) {str = nstr;}
};

int set_state(string& buf, state& st, char ch, int& digit) {
    if (cin.eof()) { return 1;}
    if (!isspace(ch)) {
        if(ch == '@') { return 1;}
        if (isalpha(ch)) {
            buf.push_back(ch);
            st = IDENT;
        } else if (isdigit(ch)) {
            digit = ch - '0';
            st = NUM;
        } else switch(ch) {
            case '/':
                buf.push_back(ch);
                st = SLSH;
                break;
            case '!': case '=':
                buf.push_back(ch);
                st = EEQ;
                break;
            case '*': case '<': case '>': case '%':
                buf.push_back(ch);
                st = EEQ2;
                break;
            case '+':
                buf.push_back(ch);
                st = PLUSS;
                break;
            case '-':
                buf.push_back(ch);
                st = MINUSS;
                break;
            case '&':
                buf.push_back(ch);
                st = AMP;
                break;
            case '|':
                buf.push_back(ch);
                st = PIPE;
                break;
            case '#':
                st = COM3;
                break;
            case '"':
                st = QUOTE;
                break;
            case '@':
                return 1;
            default:
                return 2;
        }
    }
    return 0;
} 

/* Scanner */

class Scanner {
    char ch;
    bool flag = true;
    void gc() {  cin.read(&ch, 1);}
    int find_string(string str, const char** list);
public:
    Scanner() = default;
    Scanner(const char* name){
        input_f.open(name);
        if (!input_f) {
            cout << "File not found\n";
            exit(1);
        }
        cin.rdbuf(input_f.rdbuf());
    }
    Lex get_lex();
};

int Scanner::find_string(string str, const char** list) {
    int i = 0;
    while (list[i]) {
        if (str == list[i]) return i;
        i++;
    }
    return 0;
}

Lex Scanner::get_lex() {
    string buf, res;
    int digit, t, k;
    state CurState = H;
    while (1) {
        if (flag) gc(); else flag = true;
        switch (CurState) {
            case H:
                k = set_state(buf, CurState, ch, digit);
                if (k == 2){
                    buf.push_back(ch);
                    if ((t = find_string(buf, tokens))){
                        return Lex((LexType)(FIN + t), t);
                    } else throw ch;
                }
                if (k == 1) return Lex(FIN);
                break;
            case IDENT:
                if (isdigit(ch) || isalpha(ch)) { 
                    buf.push_back(ch);
                } else {
                    flag = false;
                    if ((t = find_string(buf, words)) != 0) {
                        return Lex((LexType)t, t);
                    } else {
                        t = place_in_tid(buf);
                        return Lex(ID, t);
                    }
                }
                break;
            case NUM:
                if (isdigit(ch)) { digit = digit * 10 + (ch - '0');}
                else if (isalpha(ch)) { throw ch;}
                else {
                    flag = false;
                    return Lex(NUMB, digit);
                }
                break;
            case SLSH:
                if (ch == '*'){
                    buf.pop_back();
                    CurState = COM;
                }
                else if (ch == '='){
                    buf.push_back(ch);
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                else if (ch == '/'){
                    buf.pop_back();
                    CurState = COM3;
                }
                else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case COM:
                if (ch == '*'){
                    CurState = COM2;
                } else if (ch == '@' || std::cin.eof()) CurState = H;
                    
                break;
            case COM2:
                if (ch == '/') {
                    CurState = H;
                } else if (ch == '@' || std::cin.eof()) {
                    CurState = H;
                } else
                    CurState = COM;
                break;
            case COM3:
                if (ch == '\n')
                    CurState = H;
                else if (ch == '@' || std::cin.eof()) CurState = H;
                break;
            case EEQ:
                if (ch == '='){
                    buf.push_back(ch);
                    CurState = EEQ2;
                } else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case EEQ2:
                if (ch == '=') {
                    buf.push_back(ch);
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                } else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case PLUSS:
                if (ch == '=' || ch == '+') {
                    buf.push_back(ch);
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                } else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case MINUSS:
                if (ch == '=' || ch == '-') {
                    buf.push_back(ch);
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                } else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case AMP:
                if (ch == '&') {
                    buf.push_back(ch);
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                } else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case PIPE:
                if (ch == '|') {
                    buf.push_back(ch);
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                } else {
                    flag = false;
                    t = find_string(buf, tokens);
                    return Lex((LexType)(FIN + t), t);
                }
                break;
            case QUOTE:
                if (ch == '"') {
                    res = "";
                    for (int i = 0; i < buf.size(); i++) res += buf[i];
                    return Lex(STR_CONST, 0, res);
                } else if (cin.eof()) throw ch;
                buf.push_back(ch);
                break;
        }
    }
}

void Lex::Print() { 
        string t;
        bool fl = true;
        if (type <= WRITE)
            t = words[type];
        else if (type >= FIN && type <= DAMP)
            t = tokens[type - FIN];
        else switch(type) {
            case NUMB:
                t = "int";
                break;
            case STR_CONST:
                t = "string const";
                fl = false;
                break;
            case ID:
                t = TID[val].get_str();
                break;
            case PLABEL:
                t = "Label";
                break;
            case PADDRESS:
                t = "Addr";
                break;
            case PGO:
                t = "!";
                break;
            case PFGO:
                t = "!F";
                break;
            case PLEFTINC:
                t = "+#";
                break;
            case PRIGHTINC:
                t = "#+";
                break;
            case PLEFTDEC:
                t = "-#";
                break;
            case PRIGHTDEC:
                t = "#-";
                break;
            case PWRITE:
                t = "write";
                break;
            case PREAD:
                t = "read";
                break;
            default:
                throw type;
                break;
        }
        if (fl) Text(t, to_string(val)); else Text(t, str);
    }

template <class A, class B>
void Pop(A& st, B& a){
    a = st.top();
    st.pop();
}

/* Poliz *///===========================================================================================================================================//

class Poliz {
    vector<Lex> p;
    int index;
public:
    Poliz(int max = 1000) : index(0) { p.resize(max);}
    void PutLex(Lex l) { 
        p[index] = l; 
        ++index;
    }
    void PutLex(Lex l, int pl) { p[pl] = l;}
    void blank() { index++;}
    void PopBack() { p.pop_back();}
    int GetIndex() { return index;}
    Lex& operator[] (int idx);
    void Print() { 
        for (int i = 0; i < index; ++i) {
            cout << i << " ";
            p[i].Print(); 
        }
    }
};

Lex& Poliz::operator[](int idx){
    if (idx > p.size()) throw "Poliz: out of range";
    else if (idx > index) throw "Poliz: indefinite";
    else return p[idx];
}

/* Parser *///==========================================================================================================================================//

class Parser {
    Scanner s;

    Lex curLex;
    LexType curType;
    LexType prevType;
    int curVal;
    int prevVal;
    string curStr;
    string prevStr;

    stack<int> StInt;
    stack<LexType> StLex;
    stack <int> StLabelCon;
	stack <int> StLabelBr;

    void gl();

    void B();
    void S();
    void B1();
    void S1();
    void S_IF();
    void S_WHILE();
    void S_DO();
    void S_FOR();
    void S_FOR_P();
    void S_ID();
    void S_GOTO();
    void S_BREAK();
    void S_WRITE();
    void S_READ();
    void S_CHECKBREAK();
    void S_1();
    void S_2(int tmp);
    void E();
    void E1();
    void ED(LexType l);
    void ED2(LexType l);
    void T();
    void F();
    bool TF(bool b);
    void D();
    void D_EQ();
    void G(LexType l);
    void J(bool b);
    void S_LABEL();
    void dec(LexType type, int i);
    void declabel(LexType type, int i);

    void CheckId(LexType l);
    void CheckOp();
    void CheckNot();
    void EqType(LexType& type);
    void EqBool();
public:
    Poliz prog;
    Parser(): s() {}
    Parser(const char* name) : s(name), prog() {}
    void Analyze();
};

void Parser::gl() {
    curLex = s.get_lex();
    prevType = curType;
    prevVal = curVal;
    prevStr = curStr;
    curType = curLex.get_type();
    curVal = curLex.get_value();
    curStr = curLex.get_str();
    curLex.Print();
}

void Parser::CheckId(LexType l) {
    if (TID[curVal].get_declare()) StLex.push (TID[curVal].get_type());
    else throw "Parser: not declared";
    prog.PutLex(Lex(l, curVal));
}

void Parser::CheckNot() {
    if (StLex.top() == BOOL) prog.PutLex(Lex(NOT));
    else throw "Parser: wrong type in not";
}

void Parser::CheckOp() {
    LexType t1, t2, op;
    LexType it = INT, bl = BOOL;
    Pop(StLex, t2);
    Pop(StLex, op);
    Pop(StLex, t1);
    if (op == PLUS || op == MINUS || op == TIMES || op == SLASH || op == PERCENT) bl = INT;
    if (op == OR || op == AND) it = BOOL;
    if (t1 == t2  &&  t1 == it) StLex.push(bl);
    else throw "Parser: wrong types";
    prog.PutLex(Lex(op));
}

void Parser::EqBool() {
    if (StLex.top() != BOOL) throw "Parser: expression is not boolean";
    StLex.pop();
}

void Parser::EqType(LexType & type) {
    Pop(StLex, type);
    if (type == UNDEFINED) throw "Parser: wrong types in =";
}

void Parser::Analyze() {
    gl();
    B();
    if (curType != FIN) throw curLex;
    cout << "------------\n";
    cout << "Poliz:\n";
    prog.Print();
}

void Parser::B() {
    if(curType == FIN || curType == END){
        return;
    }
    if (curType == BEGIN){
        gl();
        S();
        if (curType == END) gl();
        else throw curType;
    } else S();
    B();
}

void Parser::B1() {
    if (curType == BEGIN){
        gl();
        S1();
        if (curType == END) gl();
        else throw curType;
    } else S();
}

void Parser::S1() {
    switch (curType){
    case IF:
        S_IF();
        break;
    case WHILE:
        S_WHILE();
        break;
    case FOR:
        S_FOR();
        break;
    case ID:
        S_ID();
        break;
    case GOTO:
        S_GOTO();
        break;
    case BREAK:
        S_BREAK();
        break;
    case BOOL: case STRING: case NUMBER:
        D();
        break;
    case NUMB:
        StLex.push(NUMB);
        prog.PutLex(Lex(curType));
        E();
        gl();
        break;
    case WRITE:
        S_WRITE();
        gl();
        break;
    case READ:
        S_READ();
        break;
    case DPLUS: case DMINUS:
        E();
        gl();
        break;
    case SEMICOLON:
        break;
    case FIN: case END:
        return;
    default:
        throw "not implemented";
    }
    B();
}

void Parser::S() {
    switch (curType){
    case IF:
        S_IF();
        break;
    case WHILE:
        S_WHILE();
        break;
    case FOR:
        S_FOR();
        break;
    case ID:
        S_ID();
        break;
    case GOTO:
        S_GOTO();
        break;
    case BREAK:
        S_BREAK();
        break;
    case BOOL: case STRING: case NUMBER:
        D();
        break;
    case NUMB:
        StLex.push(NUMB);
        prog.PutLex(Lex(curType));
        E();
        gl();
        break;
    case WRITE:
        S_WRITE();
        gl();
        break;
    case READ:
        S_READ();
        break;
    case DPLUS: case DMINUS:
        E();
        gl();
        break;
    case SEMICOLON:
        break;
    case FIN: case END:
        return;
    default:
        throw "not implemented";
    }
}

void Parser::S_IF() {
    int pl0, pl1;
    gl();
    if (curType != LPAREN) throw curLex;
    gl();
    E();
    pl0 = prog.GetIndex();
    G(PFGO);
    if (curType == RPAREN) {
        gl();
		B1();
        pl1 = prog.GetIndex();
        G(PGO);
        if (curType == ELSE){
            prog[pl0] = Lex(PLABEL, prog.GetIndex());
            gl();
            B1();
            prog[pl1] = Lex(PLABEL, prog.GetIndex());
        } else {
            //prog.PopBack();
			//prog.PopBack();
			prog[pl0] = Lex(PLABEL, prog.GetIndex());
            prog[pl1] = Lex(PLABEL, prog.GetIndex());
        }
    } else throw curLex;
}

void Parser::S_WHILE() {
    int pl0, pl1;
    pl0 = prog.GetIndex();
    StLabelCon.push(pl0);
    gl();
    if (curType != LPAREN) throw curLex;
    gl();
    E();
    pl1 = prog.GetIndex();
    G(PFGO);
    if (curType == RPAREN) {
        gl();
        B1();
        prog.PutLex(Lex(PLABEL, pl0));
        prog.PutLex(Lex(PGO));
        prog[pl1] = Lex(PLABEL, prog.GetIndex());
    } else throw curLex;
    S_CHECKBREAK();
}

void Parser::S_FOR() {
    int pl0, pl1;
    gl();
    if (curType == LPAREN) {
        S_FOR_P();
        pl0 = prog.GetIndex();
        if (curType == SEMICOLON) {
            gl();
            E();
            G(PFGO);
            G(PGO);
            pl1 = prog.GetIndex();
            StLabelCon.push(prog.GetIndex());
            if (curType == SEMICOLON) {
                S_FOR_P();
                prog.PutLex(Lex(PLABEL, pl0));
                prog.PutLex(Lex(PGO));
                prog[pl1 - 2] = Lex(PLABEL, prog.GetIndex());
                if (curType == RPAREN) {
                    gl();
                    B1();
                    prog.PutLex(Lex(PLABEL, pl1));
                    prog.PutLex(Lex(PGO));
                    prog[pl1 - 4] = Lex(PLABEL, prog.GetIndex());
                } else throw curLex;
            } else throw curLex;
        }
        S_CHECKBREAK();
    } else throw curLex;
}

void Parser::S_FOR_P() {
    gl();
    int tmp = curLex.get_value();
	CheckId(PADDRESS);
    gl();
	if (curType == EQ || curType == MINUS_EQ || curType == PLUSEQ || curType == TIMES_EQ || curType == PERCENT_EQ || curType == SLASH_EQ) {
        S_2(tmp);
	} else if (curType == DPLUS) {
        prog.PutLex(Lex(PRIGHTINC));
		gl();
		if (curType != SEMICOLON && curType != RPAREN) throw curLex;
		if (curType != RPAREN) gl();
	} else if (curType == DMINUS) {
        prog.PutLex(Lex(PRIGHTDEC));
		gl();
		if (curType != SEMICOLON && curType != RPAREN) throw curLex;
		if (curType != RPAREN) gl();
	} else throw curLex;
}

void Parser::S_CHECKBREAK() {
    if (!StLabelBr.empty()){
        prog[StLabelBr.top()] = Lex(PLABEL, prog.GetIndex());
        StLabelBr.pop();
    }
}

void Parser::S_ID() {
    int tmp = curLex.get_value();
    gl();
    if(curType != COLON) {
        int tmp = curVal;
        curVal = prevVal;
        CheckId(PADDRESS);
        curVal = tmp;
    }
    //gl();
    if (curType == EQ || curType == MINUS_EQ || curType == PLUSEQ || curType == TIMES_EQ || curType == PERCENT_EQ || curType == SLASH_EQ) {
        S_2(tmp);
        if (curType == SEMICOLON) gl();
        else if (curType == FIN) return;
        else throw curLex;
    } else if (curType == DPLUS) {
        prog.PutLex(Lex(PRIGHTINC));
        S_1();
    } else if (curType == DMINUS) {
        prog.PutLex(Lex(PRIGHTDEC));
        S_1();
    }else if(curType == COLON){
        //prog.PutLex(Lex(PADDRESS, prog.GetIndex(), ""));
        dec(PADDRESS, tmp);
        TID[tmp].set_value(prog.GetIndex());
        TID[tmp].set_assign();
        cout << TID[tmp].get_value() << " " << TID[tmp].get_type();
        gl();
        S();
    }
    else throw curLex;
}

void Parser::S_GOTO() {
    gl();
    prog.PutLex(Lex(PADDRESS, curVal));
    prog.PutLex(GOTO);
    S_1();
}

void Parser::S_BREAK() {
    StLabelBr.push(prog.GetIndex());
    G(PGO);
    S_1();
}

void Parser::S_1() {
    gl();
    if (curType != SEMICOLON && curType != FIN) throw curLex;
    gl();
}

void Parser::S_2(int tmp) {
    LexType newVal;
    gl();
	E();
	EqType(newVal);
    TID[tmp].set_type(newVal);
    prog.PutLex(Lex(EQ));
}

void Parser::S_WRITE() {
    gl();
    if (curType != LPAREN) throw curLex;
    gl();
    E();
    if (curType != RPAREN) throw curLex;
    gl();
    if (curType != SEMICOLON) throw curLex;
    prog.PutLex(Lex(PWRITE));
}

void Parser::S_READ() {
    gl();
    if (curType != LPAREN) throw curLex;
    gl();
    if (curType == ID) {
        prog.PutLex(Lex(PADDRESS, curVal, curStr));
        gl();
    } else throw curLex;

    if (curType == RPAREN) {
        gl();
        prog.PutLex(Lex(PREAD));
    } else throw curLex;
    gl();
}

void Parser::E() {
    if (curType == DPLUS){
        ED(PLEFTINC);
	} else if (curType == DMINUS) {
        ED(PLEFTDEC);
    } else {
        E1();
        while (curType == EQ || curType == LSS || curType == GTR || curType == LEQ || curType == GEQ || curType  == NEQ || curType == DMINUS || curType == DPLUS || curType == DEQ){
			LexType tmp = curType;
            StLex.push(curType);
            if (!(curType == DMINUS || curType == DPLUS)) {
                gl();
                E1();
                prog.PutLex(Lex(tmp));
            } else {
                if (curType == DMINUS) ED2(PRIGHTDEC);
                if (curType == DPLUS)  ED2(PRIGHTINC);
            }
		}
    }
}

void Parser::E1() { F(); J(0); J(1);}

void Parser::ED(LexType l) {
    gl();
    int lvIndex = curVal;
    if (curType == ID) CheckId(PADDRESS);
    else throw curLex;
    prog.PutLex(Lex(l));
    gl();
}

void Parser::ED2(LexType l) {
    prog[prog.GetIndex() - 1].set_type(PADDRESS);
    gl();
    prog.PutLex(Lex(l));
}

void Parser::F() {
    switch (curType){
    case ID:
        CheckId(ID);
        gl();
        break;
    case NUMB:
        StLex.push(NUMB);
        prog.PutLex(curLex);
        gl();
        break;
    case TRUE:
        StLex.push(BOOL);
        prog.PutLex(Lex(TRUE, 1));
        gl();
        break;
    case FALSE:
        StLex.push(BOOL);
        prog.PutLex(Lex(FALSE, 0));
        gl();
        break;
    case STR_CONST:
        StLex.push(STR_CONST);
        prog.PutLex(curLex);
        gl();
        break;
    case NOT:
        gl();
        F();
        break;
    case LPAREN:
        gl();
        E();
        if (curType == RPAREN) gl();
        else throw curLex;
    default:
        throw curLex;
    }   
}

void Parser::D() {
    do D_EQ(); while (curType == COMMA);
    if (curType != SEMICOLON && curType != FIN) throw curLex;
    if (curType == SEMICOLON) gl(); else throw curLex;
}

void Parser::D_EQ() {
    LexType type = curType;
    gl();
    if (curType != ID) throw curLex;   
    StInt.push(curVal);
    int lvIndex = curVal;
    gl();
    if (curType == EQ) {
        prog.PutLex(Lex(PADDRESS, lvIndex));
        LexType i;
        LexType tmp = curType;
        gl();
        E();
        Pop(StLex, i);
        dec(i, lvIndex);
        prog.PutLex(Lex(tmp));
    } else dec(type, lvIndex);
}

void Parser::G(LexType l) {
    prog.blank();
    prog.PutLex(Lex(l));
}

void Parser::J(bool b) {
    while (TF(b)) {
        LexType tmp = curType;
        StLex.push(curType);
        gl();
        F();
        if(b) J(0);
        prog.PutLex(Lex(tmp));
    }
}

bool Parser::TF(bool b) {
    if (b) return (curType == PLUS || curType == MINUS || curType == DPIPE || curType == COMMA);
    else return (curType == TIMES || curType == SLASH || curType == DAMP || curType == PERCENT);
}

void Parser::dec(LexType type, int i) {
    if (TID[i].get_declare()) throw "Parser: declared twice";
    TID[i].set_declare();
    TID[i].set_type(type);
}

void Parser::declabel(LexType type, int i){
    if(TID[i].get_declare());
    else{
        //TID[i].set_declare();
        TID[i].set_type(type);
    }
}

void Print(vector<string> vect) {
    for (int i = 0; i < vect.size(); i++) cout << vect[i];
}

/* Executer *///================================================================================================================================================//

class Executer {
    Lex elem;

    stack<int> IncSt;
    stack<int> DecSt;

    bool SetBools(Lex& lex1, Lex& lex2);
    bool SetBoolsI(int tmp, Lex& lex2);
    void DecIncSt();

    vector<string> ToPrint;
public:
    void Execute(Poliz& prog);
};

void Executer::Execute(Poliz& prog) {
    stack<Lex> args;
    int i, j, k, index = 0, size = prog.GetIndex(), tmp;
    string str;
    Lex lex1, lex2;
    LexType tmp1, tmp2;
    int IncDec;
    while(index < size) {
        elem = prog[index];
        cout << index << " executing" << endl;
        switch (elem.get_type()){
        case TRUE: case FALSE: case NUMB: case PADDRESS: case PLABEL: ;
        case STR_CONST:
            args.push(elem);
            break;
        case ID:
            i = elem.get_value();
            if (!TID[i].get_assign()) throw "Executer: indefinite identifier";
            args.push(Lex(TID[i].get_type(), TID[i].get_value(), TID[i].get_str()));
            break;
        case GOTO:
            Pop(args, lex1);
            if(!TID[lex1.get_value()].get_declare()) throw "Lable not declared";
            if(!TID[lex1.get_value()].get_assign()) throw "Lable not assigned";
            index = TID[lex1.get_value()].get_value() - 1;
            break;
        case PGO: 
            Pop(args, lex1);
            index = lex1.get_value() - 1;
            break;
        case PFGO: 
            Pop(args, lex1);
            Pop(args, lex2);
            if (lex2.get_type() != TRUE && lex2.get_type() != FALSE) throw "Executer: FGO error";
            if (lex2.get_type() == FALSE) index = lex1.get_value() - 1;
            break;
        case PLUS: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            tmp1 = lex1.get_type();
            tmp2 = lex2.get_type();
            if (tmp1 == STR_CONST && tmp2 == STR_CONST) {
                str = lex1.get_str() + lex2.get_str();
                lex1.set_string(str);
            } else if (tmp1 == NUMB && tmp2 == NUMB) {
                tmp = lex1.get_value() + lex2.get_value();
                lex1.set_value(tmp);
            } else if (tmp1 == NUMB && tmp2 == STR_CONST) {
                str = to_string(lex1.get_value()) + lex2.get_str();
                lex1.set_type(STR_CONST);
                lex1.set_string(str);
            } else if (tmp1 == STR_CONST && tmp2 == NUMB) {
                str = lex1.get_str() + to_string(lex2.get_value());
                lex1.set_string(str);
            } else throw "Executer: PLUS error";
            args.push(lex1);
            break;
        case PLUSEQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                cout << lex1.get_type() << " " << lex2.get_type() << endl;
                if(!SetBoolsI(tmp, lex2))
                if (TID[tmp].get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                    TID[tmp].set_string(TID[tmp].get_str() + lex2.get_str());
                } else if (lex1.get_type() == PADDRESS && lex2.get_type() == NUMB) {
                    TID[tmp].set_value(TID[tmp].get_value() + lex2.get_value());
                } else throw "Executer: PLUS EQ error 1";
            } else throw "Executer: PLUS EQ error 2";
            DecIncSt();
            break;
        case TIMES: 
            Pop(args, lex1);
            Pop(args, lex2);
            SetBools(lex1, lex2);
            tmp1 = lex1.get_type();
            tmp2 = lex2.get_type();
            if (tmp1 == NUMB && tmp2 == NUMB) {
                tmp = lex1.get_value() * lex2.get_value();
                lex1.set_value(tmp);
            } else throw "Executer: TIMES error";
            args.push(lex1);
            break;
        case TIMES_EQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if(!SetBoolsI(tmp, lex2))
                if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                    TID[tmp].set_value(TID[tmp].get_value() * lex2.get_value());
                } else throw "Executer: TIMES EQ error";
            } else throw "Executer: TIMES EQ error";
            DecIncSt();
            break;
        case MINUS: 
            Pop(args, lex1);
            Pop(args, lex2);
            SetBools(lex1, lex2);
            if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                tmp = lex2.get_value() - lex1.get_value();
                lex1.set_value(tmp);
            } else throw "Executer: MINUS error";
            args.push(lex1);
            break;
        case MINUS_EQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if(!SetBoolsI(tmp, lex2))
                if (lex1.get_type() == PADDRESS && lex2.get_type() == NUMB) {
                    TID[tmp].set_value(TID[tmp].get_value() - lex2.get_value());
                } else throw "Executer: MINUS EQ error";
            } else throw "Executer: MINUS EQ error";
            DecIncSt();
            break;
        case SLASH: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if(lex2.get_value() == 0 ) throw "Executer: Divide by zero";
                if (lex2.get_type()) {
                    tmp = lex1.get_value() / lex2.get_value();
                    lex1.set_value(tmp);
                }
            } else throw "Executer: SLASH error";
            args.push(lex1);
            break;
        case SLASH_EQ:
            Pop(args, lex2);
            Pop(args, lex1);
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if(!SetBoolsI(tmp, lex2))
                if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                    if(lex2.get_value() == 0 ) throw "Executer: Divide by zero";
                    if (!lex1.get_value()) {
                        TID[tmp].set_value(TID[tmp].get_value() / lex2.get_value());
                    }
                } else throw "Executer: SLASH EQ error";
            } else throw "Executer: SLASH EQ error";
            DecIncSt();
            break;
        case PERCENT:
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if(lex2.get_value() == 0 ) throw "Executer: Divide by zero";
                if (!lex2.get_type()) {
                    tmp = lex1.get_value() % lex2.get_value();
                    lex1.set_value(tmp);
                } 
            } else throw "Executer: PERCENT error";
            args.push(lex1);
            break;
        case PERCENT_EQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if(!SetBoolsI(tmp, lex2))
                if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                    if(lex2.get_value() == 0 ) throw "Executer: Divide by zero";
                    if (!lex1.get_value()) {
                        TID[tmp].set_value(TID[tmp].get_value() % lex2.get_value());
                    } else throw "Executer: Divide by zero";
                } else throw "Executer: Percent EQ error";
            } else throw "Executer: Percent EQ error";
            DecIncSt();
            break;
        case EQ:
            Pop(args, lex2);
            Pop(args, lex1);
            if(TID[lex1.get_value()].get_type() == NUMB){
                if(lex2.get_type() == NUMB){
                    TID[lex1.get_value()].set_value(lex2.get_value());
                }
                else throw "wrong types in =";
            }else if(TID[lex1.get_value()].get_type() == STR_CONST){
                if(lex2.get_type() == STR_CONST){
                    TID[lex1.get_value()].set_string(lex2.get_str());
                }
                else throw "wrong types in =";
            }else if(TID[lex1.get_value()].get_type() == TRUE || TID[lex1.get_value()].get_type() == FALSE){
                if(lex2.get_type() == TRUE || lex2.get_type() == FALSE){
                    TID[lex1.get_value()].set_type(lex2.get_type());
                }
                else throw "wrong types in =";
            }else throw "wrong eq attempted, intersected";
            TID[lex1.get_value()].set_assign();
            DecIncSt();
            break;
        case PLEFTINC: 
            Pop(args, lex1);
            if (lex1.get_type() != PADDRESS) throw "Executer: LEFT INC error";
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if (TID[tmp].get_type() == NUMB) {
                    k = TID[tmp].get_value();
                    TID[tmp].set_value(k + 1);
                    args.push(Lex(NUMB, k + 1, elem.get_str()));
                    IncSt.push(tmp);
                }
            } else throw "Executer: LEFT INC error";
            break;
        case PRIGHTINC: 
            Pop(args, lex1);
            if (lex1.get_type() != PADDRESS) throw "Executer: RIGHT INC error";
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if (TID[tmp].get_type() == NUMB) {
                    k = TID[tmp].get_value();
                    args.push(Lex(NUMB, k, elem.get_str()));
                    TID[tmp].set_value(k + 1);
                }
            } else throw "Executer: RIGHT INC error";
            break;
        case PLEFTDEC:
            Pop(args, lex1);
            if (lex1.get_type() != PADDRESS) throw "Executer: LEFT DEC error";
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if (TID[tmp].get_type() == NUMB) {
                    k = TID[tmp].get_value();
                    TID[tmp].set_value(k - 1);
                    DecSt.push(tmp);
                    args.push(Lex(NUMB, k - 1, elem.get_str()));
                }
            } else throw "Executer: LEFT DEC error";
            break;
        case PRIGHTDEC:
            Pop(args, lex1);
            if (lex1.get_type() != PADDRESS) throw "Executer: RIGHT DEC error";
            tmp = lex1.get_value();
            if (TID[tmp].get_assign()) {
                if (TID[tmp].get_type() == NUMB) {
                    args.push(Lex(NUMB, k = TID[tmp].get_value(), elem.get_str()));
                    TID[tmp].set_value(k - 1);
                }
            } else throw "Executer: RIGHT DEC error";
            break;
        case NOT:
            Pop(args, lex1);
            if (lex1.get_type() == STR_CONST) throw "Executer: NOT Error";
            if (lex1.get_type() == NUMB) {
                tmp = lex1.get_value();
                if (tmp == 1) lex1.set_type(FALSE);
                else if (tmp == 0) lex1.set_type(TRUE);
            } else if (lex1.get_type() == PADDRESS || lex1.get_type() == PLABEL) throw "Executer: NOT Error";
            if (lex1.get_type() == TRUE) lex1.set_type(FALSE);
            if (lex1.get_type() == FALSE) lex1.set_type(TRUE);
            args.push(lex1);
            break;
        case DPIPE:
            Pop(args, lex1);
            Pop(args, lex2);
            if (lex1.get_type() == NUMB) {
                if (lex1.get_value() > 0) tmp = 1; else tmp = 0;
            } else if (lex1.get_type() == TRUE) tmp = 1;
            else if (lex1.get_type() == FALSE) tmp = 0;
            else throw "OR Error";
            if (lex2.get_type() == NUMB) {
                if (lex2.get_value() > 0) tmp++;
            } else if (lex2.get_type() == TRUE) tmp++;
            else if (lex2.get_type() != FALSE) throw "Executer: OR Error";
            if (j == 0) lex1.set_type(FALSE);
            else lex1.set_type(TRUE);
            args.push(lex1);
            break;
        case DAMP: 
            Pop(args, lex1);
            Pop(args, lex2);
            if (lex1.get_type() == NUMB) {
                if (lex1.get_value() > 0) tmp = 1; else tmp = 0;
            } else if (lex1.get_type() == TRUE) tmp = 1;
            else if (lex1.get_type() == FALSE) tmp = 0;
            else throw "Executer: AND Error";
            if (lex2.get_type() == NUMB) {
                if (lex2.get_value() == 0) tmp = 0;
            } else if (lex2.get_type() == TRUE) tmp = 1;
              else if (lex2.get_type() == FALSE) tmp = 0;
              else throw "Executer: AND Error";
            if (tmp == 0) lex1.set_type(FALSE);
            else lex1.set_type(TRUE);
            args.push(lex1);
            break;
        case DEQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            if(!SetBools(lex1, lex2)) {
                if (lex1.get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                    if (lex1.get_str() == lex2.get_str()) lex1.set_type(TRUE);
                    else lex1.set_type(FALSE);
                } else if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                    if (lex1.get_value() == lex2.get_value()) lex1.set_type(TRUE);
                    else lex1.set_type(FALSE);
                } else throw "Executer: EQ error";
            }
            args.push(lex1);
            break;
        case LSS: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                if (lex1.get_str() < lex2.get_str()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if (lex1.get_value() < lex2.get_value()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else throw "Executer: LESS error";
            args.push(lex1);
            break;
        case GTR: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                if (lex1.get_str() > lex2.get_str()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if (lex1.get_value() > lex2.get_value()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else throw "Executer: GREATER error";
            args.push(lex1);
            break;
        case LEQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                if (lex1.get_str() <= lex2.get_str()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if (lex1.get_value() <= lex2.get_value()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else throw "Executer: LESS EQ error";
            args.push(lex1);
            break;
        case GEQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                if (lex1.get_str() >= lex2.get_str()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if (lex1.get_value() >= lex2.get_value()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else throw "Executer: GREATER EQ error";
            args.push(lex1);
            break;
        case NEQ: 
            Pop(args, lex2);
            Pop(args, lex1);
            SetBools(lex1, lex2);
            if (lex1.get_type() == STR_CONST && lex2.get_type() == STR_CONST) {
                if (lex1.get_str() != lex2.get_str()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else if (lex1.get_type() == NUMB && lex2.get_type() == NUMB) {
                if (lex1.get_value() != lex2.get_value()) lex1.set_type(TRUE);
                else lex1.set_type(FALSE);
            } else throw "Executer: NOT EQ error";
            args.push(lex1);
            break;
        case PWRITE: 
            Pop(args, lex1);
            if (lex1.get_type() == PADDRESS) {
                if (TID[lex1.get_value()].get_type() == STRING)
                    ToPrint.push_back(TID[lex1.get_value()].get_str());
                else if(TID[lex1.get_value()].get_type() == NUMBER) 
                    ToPrint.push_back(to_string(TID[lex1.get_value()].get_value()));
                else if(TID[lex1.get_value()].get_type() == TRUE)
                    ToPrint.push_back("true");
                else ToPrint.push_back("false");
            } else if (lex1.get_type() == STR_CONST) ToPrint.push_back(lex1.get_str());
            else if (lex1.get_type() == TRUE) ToPrint.push_back("true");
            else if (lex1.get_type() == FALSE) ToPrint.push_back("false");
            else if (lex1.get_type() == STRING) ToPrint.push_back(lex1.get_str());
            else ToPrint.push_back(to_string(lex1.get_value()));
            break;
        case PREAD: 
            Pop(args, lex1);
            if(lex1.get_type() == PADDRESS){
                LexType type = TID[lex1.get_value()].get_type();
                switch(type){
                    case NUMBER:
                        int a;
                        cin >> a;
                        TID[lex1.get_value()].set_value(a);
                        break;
                    case STRING:
                        cin >> str;
                        TID[lex1.get_value()].set_string(str);
                        break;
                    case TRUE: case FALSE: case BOOL:
                        cin >> str;
                        if(str == "true") TID[lex1.get_value()].set_type(TRUE);
                        else if(str == "false") TID[lex1.get_value()].set_type(FALSE);
                        else throw "wrong input";
                        break;
                    default:
                        throw "this wasn't supposed to happen";
                }
            }else throw "wrong type in read";
            TID[lex1.get_value()].set_assign();
            break;
        default:
            throw "Executer: unexpected element in POLIZ";
        }
        index++;
    }
    cout << "------------" << endl;
    cout << "Printing:"<< endl;
    Print(ToPrint);
}

bool Executer::SetBools(Lex& lex1, Lex& lex2) {
    LexType tmp1, tmp2;
    tmp1 = lex1.get_type();
    tmp2 = lex2.get_type();
    if (tmp1 == TRUE) {
        lex1.set_value(1);
        lex1.set_type(NUMB);
    } else if (tmp1 == FALSE) {
        lex1.set_value(0);
        lex1.set_type(NUMB);
    } else if (tmp2 == TRUE) {
        lex2.set_value(1);
        lex2.set_type(NUMB);
    } else if (tmp2 == FALSE) {
        lex2.set_value(0);
        lex2.set_type(NUMB);
    } else return 0;
    return 1;
}

bool Executer::SetBoolsI(int tmp, Lex& lex2) {
    LexType tmp1, tmp2;
    tmp1 = TID[tmp].get_type();
    tmp2 = lex2.get_type();
    if (tmp1 == TRUE) {
        TID[tmp].set_value(1);
        TID[tmp].set_type(NUMB);
    } else if (tmp1 == FALSE) {
        TID[tmp].set_value(0);
        TID[tmp].set_type(NUMB);
    } else if (tmp2 == TRUE) {
        lex2.set_value(1);
        lex2.set_type(NUMB);
    } else if (tmp2 == FALSE) {
        lex2.set_value(0);
        lex2.set_type(NUMB);
    } else return 0;
    return 1;
}

void Executer::DecIncSt() {
    int IncDec;
    while (!IncSt.empty()) {
        IncDec = IncSt.top();
        IncSt.pop();
        TID[IncDec].set_value(TID[IncDec].get_value() + 1);
    }
    while (!DecSt.empty()) {
        IncDec = DecSt.top();
        DecSt.pop();
        TID[IncDec].set_value(TID[IncDec].get_value() - 1);
    }
}

/* Translator *///-----------------------------------------------------------------------------------------------------------------------------//

class Translator {
    Parser p;
    Executer E;
public:
    Translator() : p() {}
    Translator(const char* name) : p(name) {};
    void Translate ();
};

void Translator::Translate() {
    p.Analyze();
    cout << "------------\n";
    cout << "Executing: " << endl;
    E.Execute(p.prog);
    cout << "\n------------" << endl;
    cout << "Finished" << endl;
}

int main(int argc, char** argv) {
    cout << "\n------------\n";
    cout << "Lexemes:\n";
    Translator t;
    if (argc == 2) t = Translator(argv[1]);
    else if (argc == 1) t = Translator();

    try { t.Translate();}
    catch (char ch){
        string s;
        s.push_back(ch);
        Error("Wrong lexem", s);
        return 1;
    }
    catch (LexType type) {
        Error("Error type", to_string(type));
        return 2;
    }
    catch (Lex l) {
        Error("Wrong lexem", "");
        l.Print();
        return 3;
    }
    catch (const char* str) {
        Error("Error: ", str);
        return 4;
    }
    cout << "------------\n";
    return 0;
}