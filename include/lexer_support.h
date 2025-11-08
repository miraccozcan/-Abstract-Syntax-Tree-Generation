#pragma once
#include <string>
#include <unordered_set>

struct Token {
    std::string type;
    std::string lexeme;
    int line = 1;
    int col  = 1;
};

extern const std::unordered_set<std::string> KEYWORDS;

struct Lexer {
    std::string src;
    size_t i = 0;
    int line = 1, col = 1;

    bool atEnd() const;
    char peek(int k = 0) const;
    char advance();

    static bool isLetter(char c);
    static bool isDigit (char c);
    static bool isAlphanumUnderscore(char c);
    static bool isSpace(char c);

    void skipWhitespace();
    bool match(char a, char b);

    Token readInlineComment(int L, int C);
    Token readBlockComment(int L, int C);
    Token readIdentifier(int L, int C);
    Token readInvalidIdentifierStartingWithUnderscore(int L, int C);
    Token readNumberGreedyAndValidate(int L, int C);

    Token nextToken(); // <-- the one your parser needs
};
