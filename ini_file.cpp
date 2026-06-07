
#include "ini_file.h"

namespace wix_installer {
        ini_plain::ini_plain(string ini_path) {
            ext_path_ = (gcnew FileInfo(ini_path))->FullName;
        }

        string ini_plain::read_string(string section, string key) {
            auto ret_val = gcnew StringBuilder(255);
            GetPrivateProfileString(section, key, "", ret_val, 255, ext_path_);
            return ret_val->ToString();
        }

        void ini_plain::write_string(string section, string key, string value) {
            WritePrivateProfileString(section, key, value, ext_path_);
        }

        void ini_plain::delete_key(string section, string key) {
            WritePrivateProfileString(section, key, nullptr, ext_path_);
        }

        void ini_plain::delete_section(string section) {
            WritePrivateProfileString(section, nullptr, nullptr, ext_path_);
        }

        bool ini_plain::key_exists(string section, string key) {
            return read_string(section, key)->Length > 0;
        }
    }