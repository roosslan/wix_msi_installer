
#include "ini_file.h"

namespace wix_installer {
        ini_plain::ini_plain(string ini_path) {
            ext_path_ = (gcnew FileInfo(ini_path))->FullName;
        }

        string ini_plain::read_string(string section, string key) {
            return read_string(section, key, "");
        }

        string ini_plain::read_string(string section, string key, string default_value) {
            /* Буфер растёт, пока значение не поместится целиком */
            for (unsigned int size = 256; size <= 65536; size *= 2) {
                auto ret_val = gcnew StringBuilder((int)size);
                unsigned int n = GetPrivateProfileString(section, key, default_value, ret_val, size, ext_path_);
                if (n < size - 1)
                    return ret_val->ToString();
            }
            throw gcnew InvalidDataException("Значение [" + section + "] " + key + " в " + ext_path_ + " слишком длинное");
        }

        void ini_plain::write_string(string section, string key, string value) {
            if (!WritePrivateProfileString(section, key, value, ext_path_))
                throw gcnew System::ComponentModel::Win32Exception(Marshal::GetLastWin32Error());
        }

        void ini_plain::delete_key(string section, string key) {
            write_string(section, key, nullptr);
        }

        void ini_plain::delete_section(string section) {
            write_string(section, nullptr, nullptr);
        }

        bool ini_plain::key_exists(string section, string key) {
            /* Маркер, которого не может быть в реальном файле: отличает отсутствующий ключ от пустого значения */
            string marker = "\x01<missing>";
            return !String::Equals(read_string(section, key, marker), marker);
        }
    }
