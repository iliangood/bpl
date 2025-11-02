#include <iostream>

#include "interpreter/processor.h"

int main(/*int argc, char** argv*/)
{
	std::vector<Instruction> programm;
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('H'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('e'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('o'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value(' '))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('W'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('o'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('r'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('d'))} );
	programm.emplace_back(OpCode::printCh_);
	programm.emplace_back(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('!'))} );
	programm.emplace_back(OpCode::printCh_);
	Processor proc(programm);
	proc.run();
	return 0;
}