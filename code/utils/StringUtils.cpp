# include "StringUtils.h"

bool StringUtils::isEmpty(const string &str) {
    if(str.empty())
        return true;
    for(auto ch : str) {
        if(ch != ' ')
            return false;
    }
    return true;
}

string StringUtils::trim(const string & str) {
    string whiteSpaces(" \t\f\v\n\r");
    auto startIdx = str.find_first_not_of(whiteSpaces);
    auto endIdx = str.find_last_not_of(whiteSpaces);
    return str.substr(startIdx, endIdx - startIdx + 1);
}

