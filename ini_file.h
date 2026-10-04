#pragma once

#include "stdafx.h"

/* Unicode-версии функций: ANSI-вариант портит пути с символами вне текущей кодовой страницы */
[DllImport("kernel32", CharSet = CharSet::Unicode, SetLastError = true)]
extern unsigned int GetPrivateProfileString(string Section, string Key, string Default, StringBuilder^ RetVal, unsigned int Size, string FilePath);

[DllImport("kernel32", CharSet = CharSet::Unicode, SetLastError = true)]
extern int WritePrivateProfileString(string Section, string Key, string Value, string FilePath);


namespace wix_installer {
    public ref class ini_plain {
        string ext_path_;
    public:
        ini_plain(string ini_path);
        string read_string(string section, string key);
        string read_string(string section, string key, string default_value);
        void write_string(string section, string key, string value);
        void delete_key(string section, string key);
        void delete_section(string section);
        bool key_exists(string section, string key);
    };
}
