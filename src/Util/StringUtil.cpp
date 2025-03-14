#include <Util/StringUtil.hpp>

std::vector<std::string> libdbusmenu::Util::String::split(std::string str, const char* delim) {
    std::vector<std::string> out;

    const char *strPtr = str.c_str(), *tmp;
    size_t delimLen    = strlen(delim);
    while((tmp = strstr(strPtr, delim)) != NULL) {
        out.push_back(std::string(strPtr, tmp));

        tmp += delimLen;
        strPtr = tmp;
    }

    out.push_back(std::string(strPtr));

    return out;
}

std::string libdbusmenu::Util::String::replace(std::string str, std::string find, std::string replace) {
    if(find.empty()) return str;

    size_t i = 0;
    while((i = str.find(find, i)) != std::string::npos) {
        str.replace(i, find.length(), replace);
        i += replace.length();
    }

    return str;
}
