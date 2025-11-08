///////////////////////////////////////////////////////////////////////////
// File Name: lexer_support.cpp
// Group Number: 2
// Group Members Names: Mirac Ozcan, Sidhant Sharma, Arvin, Paschal
// Group Members Seneca Email: mozkan1@myseneca.ca, ssharma471@myseneca.ca, aarmand1@myseneca.ca, Pibeh@myseneca.ca
// Date: November 8, 2025
// Authenticity Declaration:
// I declare this submission is the result of our group work and has not
// been shared with any other groups/students or 3rd party content provider.
///////////////////////////////////////////////////////////////////////////

#include "lexer_support.h"
#include <sstream>
#include <cctype>

using namespace std;

const unordered_set<string> KEYWORDS = {
    "integer","float","void","class","return","while","if","then","else",
    "function","constructor","attribute","public","private","self","localvar",
    "read","write","and","or","not","isa"
};

bool Lexer::atEnd() const { return i >= src.size(); }
char Lexer::peek(int k) const { return (i + k < src.size()) ? src[i + k] : '\0'; }

char Lexer::advance() {
    char c = peek();
    if (c == '\n') { line++; col = 1; }
    else col++;
    i++;
    return c;
}

bool Lexer::isLetter(char c){ return (c>='A'&&c<='Z')||(c>='a'&&c<='z'); }
bool Lexer::isDigit (char c){ return (c>='0'&&c<='9'); }
bool Lexer::isAlphanumUnderscore(char c){ return isLetter(c)||isDigit(c)||c=='_'; }
bool Lexer::isSpace(char c){ return c==' '||c=='\t'||c=='\r'||c=='\n'; }

void Lexer::skipWhitespace() { while (isSpace(peek())) advance(); }
bool Lexer::match(char a, char b){ return peek()==a && peek(1)==b; }

// ---- same tokenization routines as in your A1:
Token Lexer::readInlineComment(int L, int C) {
    string lex = "//";
    advance(); advance();
    while (!atEnd() && peek()!='\n') lex.push_back(advance());
    return Token{"inlinecmt", lex, L, C};
}
Token Lexer::readBlockComment(int L, int C) {
    string lex; lex.push_back(advance()); // '/'
    lex.push_back(advance());             // '*'
    int depth = 1;
    while (!atEnd()) {
        if (peek()=='/' && peek(1)=='*') { lex.push_back(advance()); lex.push_back(advance()); depth++; }
        else if (peek()=='*' && peek(1)=='/') { lex.push_back(advance()); lex.push_back(advance()); depth--; if (depth==0) return Token{"blockcmt", lex, L, C}; }
        else { lex.push_back(advance()); }
    }
    return Token{"invalidcomment", lex, L, C};
}

Token Lexer::readIdentifier(int L, int C) {
    string lex; lex.push_back(advance());
    while (isAlphanumUnderscore(peek())) lex.push_back(advance());
    if (KEYWORDS.count(lex)) return Token{lex, lex, L, C};
    return Token{"id", lex, L, C};
}
Token Lexer::readInvalidIdentifierStartingWithUnderscore(int L, int C) {
    string bad; bad.push_back(advance());
    while (isLetter(peek()) || isDigit(peek()) || peek()=='_') bad.push_back(advance());
    return Token{"invalidid", bad, L, C};
}

Token Lexer::readNumberGreedyAndValidate(int L, int C) {
    string lex;
    if (!isDigit(peek())) { char ch = advance(); return Token{"invalidnum", string(1,ch), L, C}; }
    while (isDigit(peek())) lex.push_back(advance());

    bool sawDot = false;
    if (peek()=='.') {
        sawDot = true;
        lex.push_back(advance());
        while (isDigit(peek())) lex.push_back(advance());
        if (peek()=='e' || peek()=='E') {
            lex.push_back(advance());
            if (peek()=='+' || peek()=='-') lex.push_back(advance());
            if (isDigit(peek())) while (isDigit(peek())) lex.push_back(advance());
        }
    }
    while (isLetter(peek()) || peek()=='_') lex.push_back(advance());

    auto allDigits = [](const string& s)->bool{
        if (s.empty()) return false;
        for (char c: s) if (!isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    };
    auto validInteger = [&](const string& s)->bool{
        if (!allDigits(s)) return false;
        if (s=="0") return true;
        return s[0] != '0';
    };

    bool hasE = (lex.find('e') != string::npos) || (lex.find('E') != string::npos);
    bool hasAlphaOrU = false;
    for (char ch: lex) if (isalpha(static_cast<unsigned char>(ch)) || ch=='_') { hasAlphaOrU = true; break; }

    if (!sawDot && !hasE && !hasAlphaOrU) {
        if (validInteger(lex)) return Token{"intnum", lex, L, C};
        return Token{"invalidnum", lex, L, C};
    }

    size_t pos = 0;
    while (pos < lex.size() && isdigit(static_cast<unsigned char>(lex[pos]))) pos++;
    string intPart = lex.substr(0, pos);
    if (pos >= lex.size() || lex[pos] != '.') return Token{"invalidnum", lex, L, C};
    pos++;

    string fracDigits;
    while (pos < lex.size() && isdigit(static_cast<unsigned char>(lex[pos]))) { fracDigits.push_back(lex[pos]); pos++; }

    bool hasExp = false;
    string expDigits;
    if (pos < lex.size() && (lex[pos]=='e' || lex[pos]=='E')) {
        hasExp = true; pos++;
        if (pos < lex.size() && (lex[pos]=='+'||lex[pos]=='-')) pos++;
        while (pos < lex.size() && isdigit(static_cast<unsigned char>(lex[pos]))) { expDigits.push_back(lex[pos]); pos++; }
    }

    if (pos != lex.size()) return Token{"invalidnum", lex, L, C};
    auto allDigits2 = [&](const string& s){ if(s.empty()) return false; for(char c:s) if(!isdigit((unsigned char)c)) return false; return true; };
    if (!(intPart=="0" || (allDigits2(intPart) && intPart[0]!='0'))) return Token{"invalidnum", lex, L, C};

    if (fracDigits == "0") {
        // ok
    } else {
        if (fracDigits.empty()) return Token{"invalidnum", lex, L, C};
        if (fracDigits.back()=='0') return Token{"invalidnum", lex, L, C};
    }

    if (hasExp) {
        if (!allDigits2(expDigits)) return Token{"invalidnum", lex, L, C};
        if (!(expDigits=="0" || (expDigits[0]!='0'))) return Token{"invalidnum", lex, L, C};
    }

    return Token{"floatnum", lex, L, C};
}

Token Lexer::nextToken() {
    skipWhitespace();
    if (atEnd()) return Token{"eof","", line, col};

    int L=line, C=col;
    char c = peek();

    if (c=='/' && peek(1)=='/') return readInlineComment(L, C);
    if (c=='/' && peek(1)=='*') return readBlockComment(L, C);

    if (isLetter(c)) return readIdentifier(L, C);
    if (c=='_') return readInvalidIdentifierStartingWithUnderscore(L, C);

    if (isDigit(c)) return readNumberGreedyAndValidate(L, C);

    if (match('=','=')) { advance(); advance(); return Token{"eq","==",L,C}; }
    if (match('<','>')) { advance(); advance(); return Token{"noteq","<>",L,C}; }
    if (match('<','=')) { advance(); advance(); return Token{"leq","<=",L,C}; }
    if (match('>','=')) { advance(); advance(); return Token{"geq",">=",L,C}; }
    if (match('=','>')) { advance(); advance(); return Token{"returntype","=>",L,C}; }
    if (match(':',':')) { advance(); advance(); return Token{"scopeop","::",L,C}; }

    switch(c){
        case '+': advance(); return Token{"plus", "+", L, C};
        case '-': advance(); return Token{"minus","-", L, C};
        case '*': advance(); return Token{"mult","*", L, C};
        case '/': advance(); return Token{"div","/", L, C};
        case '(': advance(); return Token{"openpar","(", L, C};
        case ')': advance(); return Token{"closepar",")", L, C};
        case '[': advance(); return Token{"opensqbr","[", L, C};
        case ']': advance(); return Token{"closesqbr","]", L, C};
        case '{': advance(); return Token{"opencubr","{", L, C};
        case '}': advance(); return Token{"closecubr","}", L, C};
        case ';': advance(); return Token{"semi",";", L, C};
        case ',': advance(); return Token{"comma",",", L, C};
        case ':': advance(); return Token{"colon",":", L, C};
        case '.': advance(); return Token{"dot",".", L, C};
        case '<': advance(); return Token{"lt","<", L, C};
        case '>': advance(); return Token{"gt",">", L, C};
        case '=': advance(); return Token{"assign","=", L, C};
    }

    char bad = advance();
    return Token{"invalidchar", string(1,bad), L, C};
}
