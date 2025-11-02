#include <iostream>

#include "interpreter/processor.h"

int main(/*int argc, char** argv*/)
{
	LinkType linkType = LinkType();
	TypeVariant linkVariant(linkType);

	
	setValidationLevel(ValidationLevel::basic);
	std::cout << "check" << std::endl;
	Processor proc(1 << 20);
	std::cout << "check" << std::endl;
	std::vector<Instruction> program
	{
		Instruction(OpCode::init_, std::vector<Argument>{Argument(
			FunctionType(std::vector<TypeVariant>{}, TypeVariant(&proc.baseTypes()[BaseTypeId::void_])) 
		)}),
		Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(0, true))}),
		Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value(Function( 
			FunctionType(std::vector<TypeVariant>{}, TypeVariant(&proc.baseTypes()[BaseTypeId::void_])), 
			std::vector<Instruction>
			{
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('H'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('e'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('o'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value(' '))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('W'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('o'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('r'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('d'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('!'))}),
				Instruction(OpCode::printCh_, std::vector<Argument>{})
			}
		)))}),


		Instruction(OpCode::set_, std::vector<Argument>{}),
		Instruction(OpCode::runInstsVec_, std::vector<Argument>{Argument(std::vector<Instruction>
		{
			Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(0, true))}),
			Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
			Instruction(OpCode::call_, std::vector<Argument>{})
		})} )
	};
	std::cout << "start" << std::endl;
	proc.setProgram(program);
	proc.run();
	return 0;
}