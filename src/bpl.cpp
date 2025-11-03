#include <iostream>

#include "interpreter/processor.h"

int main(/*int argc, char** argv*/)
{
	setValidationLevel(ValidationLevel::basic);
	Processor proc(1 << 20);
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

	std::vector<Instruction> prog1
	{
		Instruction(OpCode::readNum_, std::vector<Argument>{}),
		Instruction(OpCode::readCh_, std::vector<Argument>{}),
		Instruction(OpCode::readNum_, std::vector<Argument>{}),
		Instruction(OpCode::if_, std::vector<Argument>{Argument(std::vector<Instruction>
			{
				Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(1, true))}),
				Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('+'))}),
				Instruction(OpCode::equ_, std::vector<Argument>{})
			}),
			Argument(std::vector<Instruction>
				{
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(0, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(2, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::add_, std::vector<Argument>{}),
					Instruction(OpCode::printNum_, std::vector<Argument>{})
				}),
			Argument(std::vector<Instruction>{
					Instruction(OpCode::if_, std::vector<Argument>{Argument(std::vector<Instruction>
			{
				Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(1, true))}),
				Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('-'))}),
				Instruction(OpCode::equ_, std::vector<Argument>{})
			}),
			Argument(std::vector<Instruction>
				{
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(0, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(2, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::sub_, std::vector<Argument>{}),
					Instruction(OpCode::printNum_, std::vector<Argument>{})
				}),
			Argument(std::vector<Instruction>{
					Instruction(OpCode::if_, std::vector<Argument>{Argument(std::vector<Instruction>
			{
				Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(1, true))}),
				Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('*'))}),
				Instruction(OpCode::equ_, std::vector<Argument>{})
			}),
			Argument(std::vector<Instruction>
				{
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(0, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(2, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::mul_, std::vector<Argument>{}),
					Instruction(OpCode::printNum_, std::vector<Argument>{})
				}),
			Argument(std::vector<Instruction>{
					Instruction(OpCode::if_, std::vector<Argument>{Argument(std::vector<Instruction>
			{
				Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(1, true))}),
				Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
				Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('/'))}),
				Instruction(OpCode::equ_, std::vector<Argument>{})
			}),
			Argument(std::vector<Instruction>
				{
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(0, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::get_, std::vector<Argument>{Argument(PreStackIndex(2, true))}),
					Instruction(OpCode::valfromstlink_, std::vector<Argument>{}),
					Instruction(OpCode::div_, std::vector<Argument>{}),
					Instruction(OpCode::printNum_, std::vector<Argument>{})
				}),
			Argument(std::vector<Instruction>{
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('I'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('n'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('v'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('a'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('l'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('i'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('d'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value(' '))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('o'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('p'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('e'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('r'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('a'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('t'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('i'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('o'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{}),
					Instruction(OpCode::valfromarg_, std::vector<Argument>{Argument(Value('n'))}),
					Instruction(OpCode::printCh_, std::vector<Argument>{})
					
			})
		})
			})
		})
			})
		})
			})
		})
	};

	proc.setProgram(prog1);
	proc.run();
	return 0;
}