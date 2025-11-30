#include <iostream>
#include <string>
#include <fstream>

#include "interpreter/processor.h"
#include "interpreter/parser.h"

std::vector<std::string> readFile(std::string path)
{
	std::ifstream file(path);
	if(!file.is_open())
	{
		std::cerr << "Error: Could not open file " << path << std::endl;
		throw std::runtime_error("File open error");
	}
	std::string line;
	std::vector<std::string> lines;
	while(std::getline(file, line))
	{
		lines.push_back(line);
	}
	return lines;
}

int main()//int argc, char** argv)
{
	setValidationLevel(ValidationLevel::basic);
	Processor proc(1 << 20);

	/*if(argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <source-file>" << std::endl;
		return 1;
	}*/
	//std::string path = argv[1];
	std::vector<std::string> code = readFile("../func.bpl");//path);
	Parser parser(&proc);
	std::vector<Instruction> prog = parser.parse(code);
	proc.setProgram(prog);
	proc.run();
	return 0;
}