#include"CMFileSolve.h"

void CMFileSolve::CreateAndWriteFile(std::string path, const char* data, const int len)
{
    std::ofstream ofs(path, std::ios::out | std::ios::binary);
    ofs.write(data, len);
    ofs.close();
}

size_t CMFileSolve::ReadFile(std::string path, char* buffer, const int maxlen)
{
    //if (fst != nullptr && just_one) return 0;
    if (just_one) {
        fst = new std::fstream(path, std::ios::in | std::ios::binary);
        lastPath = path;
        just_one = false;
    }
    if (!fs::equivalent(lastPath, path)) return 0;
    fst->seekg(operateIndex);
    fst->read(buffer, maxlen);
    operateIndex += fst->gcount();
    return fst->gcount();
}

size_t CMFileSolve::GetTotalSize(std::string path)
{
    return fs::file_size(path);
}

size_t CMFileSolve::WriteFile(std::string path, char* buffer, int len)
{
    if (just_one) {
        fst = new std::fstream(path, std::ios::out | std::ios::binary | std::ios::app);
        lastPath = path;
        just_one = false;
    }
    if (!fs::equivalent(lastPath, path)) return 0;
    fst->write(buffer, len);
    fst->flush();
    size_t bytes = fst->gcount();
    if (fst->gcount() < len) {
        operateIndex = 0;
        lastPath.clear();
        just_one = true;
        delete fst;
        fst = nullptr;
    }
    return bytes;
}

CMFileSolve::~CMFileSolve()
{
    if (fst != nullptr) delete fst;
}