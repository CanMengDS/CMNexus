#include<iostream>
#include<filesystem>
#include <fstream>

namespace fs = std::filesystem;

class MFileSolve {
public:
	MFileSolve() = default;
	void CreateDirectories(std::string path);
	void CreateAndWriteFile(std::string path, const char* data, const int le);
	size_t ReadFile(std::string path, char* buffer, const int maxlen);
	size_t WriteFile(std::string path, char* buffer, int len);
	size_t GetTotalSize(std::string path);
	~MFileSolve();
private:
	std::fstream* fst = nullptr;
	std::size_t operateIndex = 0;
	std::string lastPath;
	bool just_one = true;
};