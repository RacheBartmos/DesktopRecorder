#ifndef UTILS_STR_H
#define UTILS_STR_H
#include <string>
class utils_string
{
public:
    static std::wstring ascii_unicode(const std::string & str);
    static std::string  unicode_ascii(const std::wstring &wstr);
    static std::string  ascii_utf8(const std::string & str);
    static std::string  utf8_ascii(const std::string &utf8);
    static std::string  unicode_utf8(const std::wstring& wstr);
    static std::wstring utf8_unicode(const std::string &utf8);
};
#endif
