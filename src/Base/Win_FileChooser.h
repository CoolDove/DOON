#include <Core/Application.h>

namespace OS
{
    inline const char* FILE_OPEN_FILTER = "Picture\0*.png;*.doo\0\0";

    inline std::string choose_file_open() {
        Application* app = Application::instance_;

        char fname[512];

        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);

        ofn.lpstrFile = fname;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(fname);

        ofn.lpstrFilter = FILE_OPEN_FILTER;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = nullptr;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrFile = fname;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameA(&ofn)) return fname;
        else return "";
    }

    inline std::string choose_file_save() {
        
        
    }
}


