#pragma once

#include "stdafx.h"

[DllImport("kernel32")]
extern int GetPrivateProfileString(string Section, string Key, string Default, StringBuilder^ RetVal, int Size, string FilePath);

[DllImport("kernel32")]
extern long WritePrivateProfileString(string Section, string Key, string Value, string FilePath);


namespace wix_installer {
    public ref class ini_plain {
        string ext_path_;       
    public:        
        ini_plain(string ini_path);
        string read_string(string section, string key);
        void write_string(string section, string key, string value);
        void delete_key(string section, string key);
        void delete_section(string section);
        bool key_exists(string section, string key);
    };
}