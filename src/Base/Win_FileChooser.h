#include <Core/Application.h>

namespace OS
{
    inline const char* FILE_OPEN_FILTER = "Picture\0*.png;*.doo\0\0";
    inline const char* FILE_SAVE_FILTER = "DooPic(*.doo)\0*.doo\0\0";
    inline const char* FILE_EXPORT_FILTER = "Picture(*.png)\0\0";

    inline std::string choose_file_open() {
        char fname[512];

        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);

        ofn.lpstrFile = fname;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(fname);

        ofn.lpstrFilter = FILE_OPEN_FILTER;
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = fname;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return fname;
        else
            return "";
    }

    inline std::string choose_file_save() {
        char fname[512];
        char* templt = "hello.doo";

        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);

        ofn.lpstrFile = fname;
        strcpy(fname, templt);
        ofn.nMaxFile = sizeof(fname) / sizeof(*fname);

        ofn.lpstrFilter = FILE_SAVE_FILTER;
        ofn.nFilterIndex = 1;

        ofn.lpstrDefExt = "doo";

        ofn.lpTemplateName = templt;

        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return fname;
        else
            return "";
    }

    inline std::string choose_file_export() {
        char fname[512];
        char* templt = "hello.png";

        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);

        ofn.lpstrFile = fname;
        strcpy(fname, templt);
        ofn.nMaxFile = sizeof(fname) / sizeof(*fname);

        ofn.lpstrFilter = FILE_EXPORT_FILTER;
        ofn.nFilterIndex = 1;

        ofn.lpstrDefExt = "png";

        ofn.lpTemplateName = templt;

        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return fname;
        else
            return "";
    }
}


