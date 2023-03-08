#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#define DEBUG_PRINT if(DEBUG)printf
bool DEBUG=1;

void fillRegName(char regChars[], char regBits, bool W);
void fillRmName(char rmChars[], char rmBits, char modBits, bool W);

int main(int argc, char **argv)
{
	//Get file(s)
	FILE *fInP = fopen(argv[1], "rb");
	FILE *fOutP = fopen(argv[2], "w");

	//Get file size
	fseek(fInP, 0L, SEEK_END); //Set file position to end
	int fInSz = ftell(fInP); //Store position (size)
	fseek(fInP, 0L, SEEK_SET);//Return to start of file
	DEBUG_PRINT("File IN size is %i.\n", fInSz);

	//Get file contents
	unsigned char inBytes[fInSz];
	for(int i=0; i<fInSz; i++)
	{
		inBytes[i] = fgetc(fInP);
	}
	//instructP points to the bottom byte of our working instruction
	unsigned char *instructP = inBytes;

	//WRITE bit width directive to output file
	//(We just do this literally with no logic needed, right?)
	fprintf(fOutP, "%s", "bits 16\n");

	//Disassemble all instructions and write to file
	int instructionsProcessed=0;
	int bytesProcessed=0;
	int currentInstructionLength;
	while(bytesProcessed < fInSz)
	{
		//WRITE newline to file for every instruction after the first
		if(bytesProcessed)fprintf(fOutP, "%c", '\n');

		DEBUG_PRINT("Beginning work on instruction %i.\n", instructionsProcessed);
		
		//Select mnemonic type
		if(instructP[0]>>4 == 0b1011)
		{///MOV (Immediate to register) MnemWReg Datadata
			DEBUG_PRINT("  Mnemonic was recognized as 1011! (MOV) [Writing...]\n");
			fprintf(fOutP, "%s", "mov "); //write mnemonic name to file
			
			//Prep variables
			bool W = ( (instructP[0]&0b00001000) >>3);
			char regChars[3]; //Human-readable REG operand chars
			
			//REG
			fillRegName(regChars, (instructP[1]&0b00111000)>>3, W);
			DEBUG_PRINT("  REG is: %c%c\n", regChars[0], regChars[1]);
		}
		else if(*instructP>>2 == 0b100010) 
		{///MOV (Register/memory to/from register) MnemonDW MdRegR/m
			DEBUG_PRINT("  Mnemonic was recognized as 100010! (MOV, REG<>REG/MEM) [Writing...]\n");
			fprintf(fOutP, "%s", "mov "); //WRITE mnemonic name to file
			
			//Prep for operand parsing
			bool D = (instructP[0] & 0b00000010);
			bool W = (instructP[0] & 0b00000001);
			DEBUG_PRINT("  %s\n", D?"(D) MOV REG, R/M":"(!D) MOV R/M, REG");
			DEBUG_PRINT("  %s\n", W?"(W) MOV 2 bytes":"(!W) MOV 1 byte");
			char regChars[3]; //Human-readable REG operand chars
			char rmChars[69]; //Human-readable R/M operand chars
			
			//MOD detection
			switch( instructP[1] >> 6 )
			{
				case 0b00:
				{
					DEBUG_PRINT("  MOD 00.\n");
				}break;
				case 0b01:
				{
					DEBUG_PRINT("  MOD 01.\n");
				}break;
				case 0b10:
				{
					DEBUG_PRINT("  MOD 10.\n");
				}break;
				case 0b11:
				{
					DEBUG_PRINT("  MOD 11.\n");
				}break;
				default:
				{
					printf("ERROR: Unrecognized MOD. Terminating...\n");
					return 1;
				}
			}
			
			//REG name generation
			fillRegName(regChars, (instructP[1]&0b00111000)>>3, W);
			DEBUG_PRINT("  REG is: %c%c\n", regChars[0], regChars[1]);
			
			//R/M name generation
			fillRmName(rmChars, (instructP[1]&0b00000111), (instructP[1]>>6), W);
			DEBUG_PRINT("  R/M is: %c%c\n", rmChars[0], rmChars[1]);
			
			//WRITE OPERANDS to .ams instruction line
			DEBUG_PRINT("  REG and R/M disassembled successfully. [Writing...]\n");
			if(D)
			{
				fprintf(fOutP, "%s, %s", regChars, rmChars);
			}
			else
			{
				fprintf(fOutP, "%s, %s", rmChars, regChars);
			}
		}
		else if(instructP[0]>>1 == 0b1100011)
		{///MOV (Immediate to register/memory) 1100011W Mod000R/m (DISP-LO) (DISP-HI) Datadata Dataifw1
			DEBUG_PRINT("  Mnemonic was recognized as 1100011! (MOV) [Writing...]\n");
			fprintf(fOutP, "%s", "mov "); //write mnemonic name to file
			
			//Prep variables
			bool W = (instructP[0] & 0b00000001);
			char rmChars[69]; //Human-readable R/M operand chars
		}
		else
		{///ERROR: All cases failed. Unrecognized mnemonic.
			printf("ERROR: Instruction unrecognized mnemonic. Terminating.\n");
			return 1;
		}
		
		//Upkeep
		instructionsProcessed++;
		instructP+=2;///TODO: Increment this in the loop
		bytesProcessed = instructP-inBytes;
		DEBUG_PRINT("  Bytes processed: %i/%i\n", instructP-inBytes, fInSz);
	}//End of instruction disassembly

	//Get output file size
	fseek(fOutP, 0L, SEEK_END); //Set file position to end
	int fOutSz = ftell(fOutP); //Store position (size)
	fseek(fOutP, 0L, SEEK_SET); //Return to start of file
	DEBUG_PRINT("File OUT size is %i.\n", fOutSz);

	//Print output file contents to stdout
	printf("::: CONTENTS OF OUTPUT FILE :::\n");
	printf("%s", *fOutP);
	
	///TODO: Compare bins
	//Done.
	fclose(fInP);
	fclose(fOutP);
	//system("pause");
	return 0;
}//END MAIN

void fillRegName(char regChars[], char regBits, bool W)
{
	///Fills in chars for MOV operand name. Works for REG
	///Works for R/M as well, as long as MOD bits in the instruction are 11
	///Char values come from tables on page 4-20 of 8086 manual (page 162 of the pdf)
	switch(regBits)
	{
		case 0b000:
		{
			regChars[0] = 'a';			//first char of operand
			regChars[1] = !W?'l':'x';	//secnd char of operand
			regChars[2] = '\0';
		}break;
		
		case 0b001:
		{
			regChars[0] = 'c';
			regChars[1] = !W?'l':'x';
			regChars[2] = '\0';
		}break;
		
		case 0b010:
		{
			regChars[0] = 'd';
			regChars[1] = !W?'l':'x';
			regChars[2] = '\0';
		}break;
		
		case 0b011:
		{
			regChars[0] = 'b';
			regChars[1] = !W?'l':'x';
			regChars[2] = '\0';
		}break;
		
		case 0b100:
		{
			regChars[0] = !W?'a':'s';
			regChars[1] = !W?'h':'p';
			regChars[2] = '\0';
		}break;
		
		case 0b101:
		{
			regChars[0] = !W?'c':'b';
			regChars[1] = !W?'h':'p';
			regChars[2] = '\0';
		}break;
		
		case 0b110:
		{
			regChars[0] = !W?'d':'s';
			regChars[1] = !W?'h':'i';
			regChars[2] = '\0';
		}break;
		
		case 0b111:
		{
			regChars[0] = !W?'b':'d';
			regChars[1] = !W?'h':'i';
			regChars[2] = '\0';
		}break;
		
		default:
		{
			printf("~~ERROR parsing REG (OR R/M). Terminating.\n");
		}
	}
}

void fillRmName(char rmChars[], char rmBits, char modBits, bool W)
{
	///TODO: This isn't fully implemented yet. It can't handle all cases of MOD.
	if(modBits == 0b11)
	{
		DEBUG_PRINT("  This R/M tastes a lot like a REG.\n");
		fillRegName(rmChars, rmBits, W);
	}
	else if(modBits==0b00 && rmBits==0b110)
	{
		//The special case
		rmChars = "DIRECT ADDRESS";
	}
	else
	{
		//We know we aren't MOD=11, and we aren't the special DIRECT ADDRESS case.
		//See table 4-10 on page 4-20. Now the plot thickens!
		switch(rmBits)
		{
			case 0b000:
			{
				rmChars = "[bx + si";
			}break;
			
			case 0b001:
			{
				rmChars = "[bx + di";
			}break;
			
			case 0b010:
			{
				rmChars = "[bp + si";
			}break;
			
			case 0b011:
			{
				rmChars = "[bp + di";
			}break;
			
			case 0b100:
			{
				rmChars = "[si";
			}break;
			
			case 0b101:
			{
				rmChars = "[di";
			}break;
			
			case 0b110:
			{
				rmChars = "[bp";
			}break;
			
			case 0b111:
			{
				rmChars = "[bx";
			}break;
			
			default:
			{
				printf("ERROR DISASSEMBLING R/M");
			}
		}//End switch
		//But we need to CONCATENATE a little more!
		//Just the bracket, plus MOD=01 and MOD=10 stuff
		switch(modBits)
		{
			case 0b00:
			{
				strncat(rmChars, "]", 1);
			}break;
			
			case 0b01:
			{
				strncat(rmChars, " + d8]", 1);
			}break;
			
			case 0b10:
			{
				strncat(rmChars, " + d16]", 1);
			}break;
			
			default:
			{
				DEBUG_PRINT("ERROR during final concatenation for fillRmName");
			}
		}
	}
}