#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#define DEBUG_PRINT if(DEBUG)printf
#define USE_BYTE false
#define USE_WORD true
bool DEBUG=1;

enum ///Formats (Compatible mnemonics are listed below each format.)
{
//MAX size 1

	S1oooooReg, //to:with REG
	///XCHG, INC, DEC
	
	S1oooooooW, //Variable port?
	///IN, OUT

//MAX size 2
	
	S2oooooooW_Datadata, //Fixed port?
	///IN, OUT
	
//MAX size 3

	S3ooooWReg_Datadata_Datadatw, //IMM to REG
	///MOV
	
	S3oooooooW_AdDa_AdDa, //IMM or MEM tofrom ACC (AdDa is address OR data)
	///MOV, ADD, SUB, CMP(!W)
	
//MAX size 4
	
	S4ooooooDW_MdRegRgm_Di_Di, //RM to:from REG
	///MOV, XCHG(D), ADD, SUB, CMP, OR, XOR
	
	S4oooooooo_MdUSrRgm_DisplcLo_DisplcHi, //U=subOp RM tofrom SEGREG
	///MOV
	
//MAX size 6
	
	S6oooooooW_DATADATA_Di_Di_Da_Da,
	///XOR

	S6oooooooW_MdSubRgm_Di_Di_Da_Da, //IMM to RM
	///MOV, add, or, adc, sbb, and, sub, adc, cmp

//Buffer
	ENUM_COMMA_BUFFER
};

bool theseBitsMatch(unsigned char* instrP, char opString[]);
short valFromUCharP(unsigned char *charP, bool W);
void fillRegName(char regName[], char regBitsVal, bool W);
void fillRmName(char rmName[], char rmBitsVal, char modBitsVal, bool W, short disp);
void DEBUG_printBytesIn01s(unsigned char *startP, int size, int columns);

bool D;
bool W;
bool S;
short dispVal; //For displacement (including DIRECT ACCESS)
short dataVal;
unsigned char opForm;
unsigned char instrSz;
unsigned char subOpVal; //Bits that distinguish instructions with identical first-byte opcodes
unsigned char dispSz;
unsigned char dataSz;
unsigned char modBitsVal;
unsigned char regBitsVal;
unsigned char rmBitsVal;
char mnemName[8]; //Mnemonics like mov, add, sub, pushad, etc
char regName[3]; //Human-readable REG operand
char rmName[32]; //Human-readable R/M operand
char instrString[32];


int main(int argc, char *argv[])
{
	//Get file(s)
	FILE *fInP = fopen(argv[1], "rb");
	FILE *fOutP = fopen(argv[2], "w");

	//Get file in size
	fseek(fInP, 0L, SEEK_END); //Set file position to end
	int fInSz = ftell(fInP); //Store position (file size)
	fseek(fInP, 0L, SEEK_SET);//Return to start of file

	//Get file contents
	unsigned char inBytes[fInSz];
	for(int i=0 ; i<fInSz ; i++)
	{
		inBytes[i] = fgetc(fInP);
	}
	
	//For iterating through instructions
	unsigned char *instrP = inBytes;

	//In debug mode, print all bits of bin, in 4 columns.
	DEBUG_PRINT("\n\"%s\" bin size is %i. Contents:\n\n", argv[1], fInSz);
	DEBUG_printBytesIn01s(inBytes, fInSz, 4);
	DEBUG_PRINT("\n\n");
	
	//Startup vars and constants
	unsigned long long int instrsProcessed=0;
	unsigned long long int bytesProcessed=0;
	const char mnems100000[8][8]=
	{
		//Mnemonics for all operations that collide on instrP byte [0] opcode 100000xx,
		//sorted by binary value of there subOp bits from byte [1].
		"add\0\0\0\0"/*000*/, "or\0\0\0\0\0"/*001*/, "adc\0\0\0\0"/*010*/, "sbb\0\0\0\0"/*011*/,
		"and\0\0\0\0"/*100*/, "sub\0\0\0\0"/*101*/, "???\0\0\0\0"/*110*/, "cmp\0\0\0\0"/*111*/
	};
	const char modMsgs[4][64]=
	{
		"  MOD: 00 (Mem Mode, no disp)\n", "  MOD: 01 (Mem Mode, 8-bit disp)\n",
		"  MOD: 10 (Mem Mode, 16-bit disp)\n", "  MOD: 11 (Reg Mode, no disp)\n"
	};
	
	//WRITE bit width directive to output file
	fprintf(fOutP, "%s", "bits 16\n");
	
	//MASTER LOOP
	while(bytesProcessed < fInSz)
	{///Each iteration disassembles one instruction

		DEBUG_PRINT("Beginning work on instruction %i. ", instrsProcessed+1);
		DEBUG_PRINT("First byte: ");
		DEBUG_printBytesIn01s(instrP, 1, 0);
		DEBUG_PRINT("\n");

		//Set opForm, subOpVal, and mnemName
		if(0);
		else if(theseBitsMatch(instrP,"10010reg")){opForm=  S1oooooReg  ;strcpy(mnemName,"xchg");}///to:with REG
		else if(theseBitsMatch(instrP,"01000reg")){opForm=  S1oooooReg  ;strcpy(mnemName,"inc");}///to:with REG
		else if(theseBitsMatch(instrP,"01001reg")){opForm=  S1oooooReg  ;strcpy(mnemName,"dec");}///to:with REG
		else if(theseBitsMatch(instrP,"1110110w")){opForm=  S1oooooooW  ;strcpy(mnemName,"in");}///Fixed port
		else if(theseBitsMatch(instrP,"1110111w")){opForm=  S1oooooooW  ;strcpy(mnemName,"out");}///Variable port
		else if(theseBitsMatch(instrP,"1110010w")){opForm=  S2oooooooW_Datadata  ;strcpy(mnemName,"in");}///Fixed port
		else if(theseBitsMatch(instrP,"1110011w")){opForm=  S2oooooooW_Datadata  ;strcpy(mnemName,"out");}///Variable port
		else if(theseBitsMatch(instrP,"1010000w")){opForm=  S3oooooooW_AdDa_AdDa  ;strcpy(mnemName,"mov");}///MEM to ACC
		else if(theseBitsMatch(instrP,"1010001w")){opForm=  S3oooooooW_AdDa_AdDa  ;strcpy(mnemName,"mov");}///ACC to MEM
		else if(theseBitsMatch(instrP,"0000010w")){opForm=  S3oooooooW_AdDa_AdDa  ;strcpy(mnemName,"add");}///IMM to ACC
		else if(theseBitsMatch(instrP,"0001010w")){opForm=  S3oooooooW_AdDa_AdDa  ;strcpy(mnemName,"adc");}///IMM to ACC
		else if(theseBitsMatch(instrP,"0010110w")){opForm=  S3oooooooW_AdDa_AdDa  ;strcpy(mnemName,"sub");}///IMM (from) ACC
		else if(theseBitsMatch(instrP,"0001110w")){opForm=  S3oooooooW_AdDa_AdDa  ;strcpy(mnemName,"sbb");}///IMM (from) ACC
		else if(theseBitsMatch(instrP,"1011wreg")){opForm=  S3ooooWReg_Datadata_Datadatw  ;strcpy(mnemName,"mov");}///IMM to REG
		else if(theseBitsMatch(instrP,"100010dw")){opForm=  S4ooooooDW_MdRegRgm_Di_Di  ;strcpy(mnemName,"mov");}///RM to:from REG
		else if(theseBitsMatch(instrP,"1000011x")){opForm=  S4ooooooDW_MdRegRgm_Di_Di  ;strcpy(mnemName,"xchg");}///ALWAYS D
		else if(theseBitsMatch(instrP,"000000dw")){opForm=  S4ooooooDW_MdRegRgm_Di_Di  ;strcpy(mnemName,"add");}///RM to:from REG
		else if(theseBitsMatch(instrP,"001010dw")){opForm=  S4ooooooDW_MdRegRgm_Di_Di  ;strcpy(mnemName,"sub");}///RM to:from REG
		else if(theseBitsMatch(instrP,"001110dw")){opForm=  S4ooooooDW_MdRegRgm_Di_Di  ;strcpy(mnemName,"cmp");}///RM to:from REG
		else if(theseBitsMatch(instrP,"0011010w")){opForm=  S6oooooooW_DATADATA_Di_Di_Da_Da;  strcpy(mnemName, "xor");}
		else if(theseBitsMatch(instrP,"1100011w")){opForm=  S6oooooooW_MdSubRgm_Di_Di_Da_Da;  strcpy(mnemName, "mov");}///ALWAYS S
		else if(theseBitsMatch(instrP,"1111011w")){opForm=  S6oooooooW_MdSubRgm_Di_Di_Da_Da;  strcpy(mnemName, "test");}
		else if(theseBitsMatch(instrP,"100000xx"))        //S6oooooooW_MdSubRgm_Di_Di_Da_Da
		{
			//add, or, adc, sbb, and, sub, adc, cmp
			opForm = S6oooooooW_MdSubRgm_Di_Di_Da_Da;
			subOpVal = (instrP[1]&0b00111000)>>3;
			strcpy(mnemName, mnems100000[subOpVal]);
		}
		else{printf("ERROR slecting mnemonic. [Terminating...]"); return 1;}
		
		//WRITE newline to file for every instruction after the first
		if(bytesProcessed)fprintf(fOutP, "\n");
		
		//Select disassembly logic
		switch(opForm)
		{
			case S1oooooReg: //to:with REG
			{///xchg, inc, dec
				fillRegName(regName, instrP[0]&0b00000111, USE_WORD);
				instrSz = 1;
				
				DEBUG_PRINT("  opForm: S1oooooReg\n");
				DEBUG_PRINT("  Mnem: %s (to:with REG) Size: %i\n", mnemName, instrSz);
				
				//Build output string
				if( !strcmp(mnemName, "xchg") )
				{
					//(2 operands)
					sprintf(instrString, "%s ax, %s", mnemName, regName);
				}
				else
				{
					//(1 operand)
					sprintf(instrString, "%s %s", mnemName, regName);
				}
			}break;
			case S4ooooooDW_MdRegRgm_Di_Di: //RM to:from REG
			{///MOV, XCHG(D), ADD, SUB, CMP, OR, XOR
				D = theseBitsMatch(instrP, "xxxxxx1x");
				W = theseBitsMatch(instrP, "xxxxxxx1");
				modBitsVal = instrP[1]>>6;
				regBitsVal = (instrP[1]&0b00111000)>>3;
				rmBitsVal = (instrP[1]&0b00000111);
				dispSz = modBitsVal%3;
				instrSz = 2 + dispSz;
	
				DEBUG_PRINT("  opForm: S4ooooooDW_MdRegRgm_Di_Di\n");
				DEBUG_PRINT("  %s (RM TO:FROM REG)\n", mnemName);
				DEBUG_PRINT("  %s\n", D?"D":"!D");
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				DEBUG_PRINT(modMsgs[modBitsVal]);
				
				//DISP value calc
				if(modBitsVal == 0b01)
				{
					dispVal = valFromUCharP(instrP+2, USE_BYTE);
				}
				else if(modBitsVal == 0b10)
				{
					dispVal = valFromUCharP(instrP+2, USE_WORD);
				}
				else if(modBitsVal==0b00 && rmBitsVal==0b110)
				{
					//Special DIRECT ACCESS case
					instrSz+=2;
					dispVal = valFromUCharP(instrP+2, USE_WORD);
				}
				
				//REG name generation
				fillRegName(regName, regBitsVal, W);
				//RM name generation
				fillRmName(rmName, rmBitsVal, modBitsVal, W, dispVal);
				
				//Build output string
				if(D)
				{	//XXX REG, R/M
					sprintf(instrString, "%s %s, %s", mnemName, regName, rmName);
				}
				else
				{	//XXX R/M, REG
					sprintf(instrString, "%s %s, %s", mnemName, rmName, regName);
				}
			}break;
			case S6oooooooW_MdSubRgm_Di_Di_Da_Da: //IMM to RM
			{///MOV, add, or, adc, sbb, and, sub, adc, cmp
				S = theseBitsMatch(instrP, "xxxxxx1x");
				W = theseBitsMatch(instrP, "xxxxxxx1");
				modBitsVal = instrP[1]>>6;
				rmBitsVal = (instrP[1]&0b00000111);
				dispSz = modBitsVal%3;
				dataSz = 1 + (char)W;
				instrSz = 2 + dispSz + dataSz;
				
				DEBUG_PRINT("  %s\n", S?"S":"!S");
				DEBUG_PRINT("  %s\n", D?"D":"!D");
				DEBUG_PRINT(modMsgs[modBitsVal]);
				DEBUG_PRINT("  opForm: S6oooooooW_MdSubRgm_Di_Di_Da_Da\n");
				DEBUG_PRINT("  %s (IMM to RM)\n", mnemName);
				
				//DISP value calc
				if(modBitsVal == 0b01)
				{
					dispVal = valFromUCharP(instrP+2, USE_BYTE);
				}
				else if(modBitsVal == 0b10)
				{
					dispVal = valFromUCharP(instrP+2, USE_WORD);
				}
				else if(modBitsVal==0b00 && rmBitsVal==0b110)
				{
					//DIRECT ACCESS special case
					dispSz = 2;
					instrSz += instrSz;
					dispVal = valFromUCharP(instrP+2, USE_WORD);
				}
				
				//DATA
				dataVal = valFromUCharP(instrP+2+dispSz, W);
				
				//RM name generation
				fillRmName(rmName, rmBitsVal, modBitsVal, W, dispVal);
				
				//Build output string
				sprintf(instrString, "%s %s, %s", mnemName, rmName, dataVal);
			}break;
			case S3ooooWReg_Datadata_Datadatw: //IMM to REG
			{///MOV
				W = theseBitsMatch(instrP, "xxxx1xxx");
				regBitsVal = (instrP[0]&0b00000111);
				dataSz = 1 + (char)W;
				instrSz = 1 + dataSz;
				
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				DEBUG_PRINT("  opForm: S3ooooWReg_Datadata_Datadatw\n");
				DEBUG_PRINT("  %s (IMM to REG)\n", mnemName);
				
				//DATA
				dataVal = valFromUCharP(instrP+1, W);
				
				//REG name generation
				fillRegName(regName, regBitsVal, W);
				
				//Build output string
				sprintf(instrString, "%s %s, %i", mnemName, regName, dataVal);
			}break;
			default:
			{///ERROR
				printf("ERROR: Unrecognized instruction type. Terminating.\n");
				return 1;
			}
		}
		
		//WRITE instruction to output file
		fprintf(fOutP, instrString);
		DEBUG_PRINT("  Successfully disassembled -> \"%s\" [Writing...]\n", instrString);
		
		//Loop upkeep
		DEBUG_PRINT("  Instruction size: %i\n", instrSz);
		instrP += instrSz;
		instrsProcessed++;
		bytesProcessed = instrP-inBytes;
		DEBUG_PRINT("  Bytes processed: %i/%i\n", instrP-inBytes, fInSz);
	}
	//End of disassembling

	//Get output file size
	fseek(fOutP, 0L, SEEK_END); //Set file position to end
	int fOutSz = ftell(fOutP); //Store position (size)
	fseek(fOutP, 0L, SEEK_SET); //Return to start of file
	
	DEBUG_PRINT("Disassembly DONE.\n");
	DEBUG_PRINT("Total instructions: %i\n", instrsProcessed);
	DEBUG_PRINT("File IN size: %i\n", fInSz);
	DEBUG_PRINT("File OUT size: %i\n", fOutSz);

	//Print output file contents to stdout
	printf("\n\n::: CONTENTS OF OUTPUT FILE :::\n");
	printf("%s", *fOutP);
	
	///TODO: Compare bins
	//Done.
	fclose(fInP);
	fclose(fOutP);
	//system("pause");
	return 0;
}
//END MAIN

void fillRegName(char regName[], char regBitsVal, bool W)
{
	//                W? 01 R/M
	char regNameMatrix[2][2][8]=
	{// 000  001  010  011  100  101  110  111   <- R/M bit vals
		'a', 'c', 'd', 'b', 'a', 'c', 'd', 'b',  // !w
		'l', 'l', 'l', 'l', 'h', 'h', 'h', 'h',
		
		'a', 'c', 'd', 'b', 's', 'b', 's', 'd',   // w
		'x', 'x', 'x', 'x', 'p', 'p', 'i', 'i'
	};
	
	regName[0] = regNameMatrix[(char)W][0][regBitsVal];
	regName[1] = regNameMatrix[(char)W][1][regBitsVal];
	regName[2] = '\0';
	
	DEBUG_PRINT("  REG name: %s\n", regName);
}
void fillRmName(char rmName[], char rmBitsVal, char modBitsVal, bool W, short dispDA)
{
	if(modBitsVal == 0b11)
	{
		fillRegName(rmName, rmBitsVal, W);
		DEBUG_PRINT("  ^Actually an R/M; it's merely REG-flavored!\n");
	}
	else if(modBitsVal==0b00 && rmBitsVal==0b110)
	{
		//The special case
		sprintf(rmName, "[%i]", dispDA);
	}
	else
	{
		//We know we aren't MOD=11, and we aren't the special DIRECT ADDRESS case.
		//Besides that, this block is MOD agnostic.
		//See table 4.10 on page 4.20.
		if     (rmBitsVal==0b000){strcpy(rmName, "[bx + si");}
		else if(rmBitsVal==0b001){strcpy(rmName, "[bx + di");}
		else if(rmBitsVal==0b010){strcpy(rmName, "[bp + si");}
		else if(rmBitsVal==0b011){strcpy(rmName, "[bp + di");}
		else if(rmBitsVal==0b100){strcpy(rmName, "[si");}
		else if(rmBitsVal==0b101){strcpy(rmName, "[di");}
		else if(rmBitsVal==0b110){strcpy(rmName, "[bp");}
		else if(rmBitsVal==0b111){strcpy(rmName, "[bx");}
		else{printf("ERROR DISASSEMBLING R/M");}
		
		//But we need to CONCATENATE a little more!
		//Just the disp for MOD=01 and MOD=10, then final bracket
		if(modBitsVal==0b01 || modBitsVal==0b10)
		{
			if(dispDA >= 0)
			{
				strncat(rmName, " + ", 3);
			}
			else
			{
				dispDA = dispDA * (-1);
				strncat(rmName, " - ", 3);
			}
			//conc displacement as string
			char dispString[8];
			sprintf(dispString, "%i", dispDA);
			strncat(rmName, dispString, 8);
		}
		//Everyone here gets a bracket!
		strncat(rmName, "]", 2);
	}
	if(modBitsVal != 0b11)
	{
		//fillRegName will have already printed the value.
		DEBUG_PRINT("  RM name: %s\n", rmName);
	}
}
short valFromUCharP(unsigned char *charP, bool word)
{
	//Always returns signed value.
	if(word)
	{
		//Also use charP+1 to make a short
		return * ((short*)charP);
	}
	else
	{
		//Just use charP
		return (char)*charP;
	}
}
void DEBUG_printBytesIn01s(unsigned char *startP, int size, int columns)
{
	for(int i=0 ; i<size ; i++)
	{
		///Each iteration prints 1 byte and a space
		for(int j=7 ; j>=0 ; j--)
		{
			///Each iteration prints 1 bit
			printf("%i", (startP[i] >> j) & 1);
		}
		
		printf(" ");
		if( columns!=0 && i!=0 && !((i+1)%columns) )
		{
			printf("\n");
		}
	}	
}
bool theseBitsMatch(unsigned char* instrP, char opString[])
{
	//Takes in a pointer to an opcode bit and a string literal
	//representing an instruction format. Non-zero, non-one
	//chars in the string mask non-opcode bits. So "1011WReg",
	//for example, is a valid argument for opString.
	
	unsigned char checkedBit = 	0b10000000;
	bool bitNeedsChecking, bitMatches;
	
	for(int i=0 ; i<8 ; i++)
	{
		bitNeedsChecking = (opString[i]=='1') || (opString[i]=='0');		
		bitMatches = (bool)((*instrP)&checkedBit>>i) == (bool)(opString[i]=='1');
		
		if( bitNeedsChecking && !bitMatches )
		{
			return false;
		}
	}
	return true;
}