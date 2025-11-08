///////////////////////////////////////////////////////////////////////////
// File Name: lexer.hpp
// Group Number: 2
// Group Members Names: Mirac Ozcan, Sidhant Sharma, Arvin, Paschal
// Group Members Seneca Email: mozkan1@myseneca.ca, ssharma471@myseneca.ca, aarmand1@myseneca.ca, Pibeh@myseneca.ca
// Date: November 8, 2025
// Authenticity Declaration:
// I declare this submission is the result of our group work and has not
// been shared with any other groups/students or 3rd party content provider.
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <unordered_set>

struct Token {
    std::string type;
    std::string lexeme;
    int line = 1;
    int col  = 1;
};

// Paste your KEYWORDS set here (exact same as A1):
extern const std::unordered_set<std::string> KEYWORDS;

// Paste ONLY the Lexer class here (verbatim) with nextToken() etc.
// Do NOT paste your A1 main(), file I/O, or any driver-only code.
struct Lexer {
    std::string src;
    size_t i = 0;
    int line = 1, col = 1;

    bool atEnd() const;
    char peek(int k = 0) const;
    char advance();
    static bool isLetter(char c);
    static bool isDigit(char c);
    static bool isAlphanumUnderscore(char c);
    static bool isSpace(char c);
    void skipWhitespace();
    bool match(char a, char b);

    // … your comment/identifier/number helpers …

    Token nextToken();
};
