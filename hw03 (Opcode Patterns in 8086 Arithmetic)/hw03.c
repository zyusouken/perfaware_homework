#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<time.h>
#define DEBUG_PRINT if(DEBUG)printf
#define USE_BYTE false
#define USE_WORD true
bool DEBUG=1;

enum ///Formats (Compatible mnemonics are listed below each format.)
{
//MAX size 1
	S1oooooooo, //Pure opcode
	///xlat, lahf, sahf, pushf, popf, aaa, daa, aas, das,
	///cbw, cwd, ret(within segment), ret(inter-segment),
	///int(type3), into, iret, clc, cmc, stc, cld, std, cli,
	///sti, hlt, wait, lock
	S1oooooReg, //to:with REG
	///XCHG, INC, DEC, push, pop
	S1oooSrooo, //Segment register
	///push, pop
	S1oooooooW, //Variable port?
	///IN, OUT
//MAX size 2
	S2oooooooW_Data, //Fixed port?
	///IN, OUT
	S2oooooooo_LABEL,
	///Lots of jumps
//MAX size 3
	S3ooooWReg_Data_Datw, //IMM to REG
	///MOV
	S3oooooooW_DaAd_DaAw, //IMM or MEM tofrom ACC (DaAd is Data||Address)
	///MOV, ADD, SUB, CMP(data size always 1)
//MAX size 4
	S4ooooooDW_MdRegRgm_Disp_Disp, //RM to:from REG
	///MOV, XCHG(D), ADD, SUB, CMP, OR, XOR
	S4oooooooo_MdUSrRgm_Disp_Disp, //U=subOp RM tofrom SEGREG
	///MOV
	S4oooooooo_MdRegRgm_Disp_Disp,
	///LEA, LDS, LES
	S4oooooooW_MdSubRgm_Disp_Disp, //U=subOp RM tofrom SEGREG
	///MOV
//MAX size 6
	S6oooooooW_DATADATA_Di_Di_Da_Da,
	///XOR
	S6ooooooSW_MdSubRgm_Di_Di_Da_Da //IMM to RM
	///mov (D instead of S), add, or, adc, sbb, and(S), sub, adc, cmp
};

short valFromUCharP(unsigned char *charP, bool W);
void fillRegName(char regName[], char regBitsVal, bool W);
void fillRmName(char rmName[], char rmBitsVal, char modBitsVal, bool W, short disp);
void DEBUG_printBytesIn01s(unsigned char *startP, int size, int columns);
void ERROR_TERMINATE(unsigned char *inBytesP,unsigned char *instrSizes,unsigned long long int instrsDone,unsigned char*fOutP,char msg[]);

unsigned long int instrsDone=0;
unsigned long int bytesDone=0;
bool W, S, D, V, Z;
bool MemAddrMode; //If false, dataOrAddr is immediate data.
short dispVal; //For displacement (including DIRECT ACCESS)
short dataOrAddr;
unsigned char dataSz;
unsigned char dispSz;
unsigned char instrSz;
unsigned char modBitsVal;
unsigned char opForm;
unsigned char regBitsVal;
unsigned char rmBitsVal;
unsigned char srBitsVal;
unsigned char subOpVal;
clock_t starttime;
char mnemName[8], regName[3], rmName[32], instrString[32];
const char srNames[4][3] = {"es", "cs", "ss", "ds"};
const char modMsgs[4][64]=
{
	"  MOD: 00 (Mem Mode, no disp)\n", "  MOD: 01 (Mem Mode, 8-bit disp)\n",
	"  MOD: 10 (Mem Mode, 16-bit disp)\n", "  MOD: 11 (Reg Mode, no disp)\n"
};


/// //////////////////////////////////////// ///
/// ///               MAIN               /// ///
/// //////////////////////////////////////// ///
int main(int argc, char *argv[])
{	
	starttime=clock();
	//Get file(s)
	FILE *fInP = fopen(argv[1], "rb");
	FILE *fOutP = fopen(argv[2], "w");

	//Get file in size
	fseek(fInP, 0L, SEEK_END); //Set file position to end
	int fInSz = ftell(fInP); //Store position (file size)
	fseek(fInP, 0L, SEEK_SET);//Return to start of file
	
	//We'll store instr strings here until writing to out file.
	char instrStrings[fInSz][32];
	unsigned char instrSizes[fInSz];
	
	//These are the bytes after which we will write labels in the final asm.
	unsigned long int labelCount=0;
	unsigned long int labelIndex;
	bool labelExists;
	unsigned long int labelIndices[fInSz];
	
	//Get file contents
	unsigned char inBytes[fInSz];
	for(int i=0 ; i<fInSz ; i++)
	{
		inBytes[i] = fgetc(fInP);
	}

	/// /// ///DEBUG manual bin override:
	//unsigned char inBytes[2] = {0b00111101, 0b00000011}; fInSz = sizeof(inBytes);
	/// /// ///Note that file comparison at the end of the program will always fail.
	
	//For iterating through instructions
	unsigned char *instrP = inBytes;

	//In debug mode, print all bits of bin, in 4 columns.
	DEBUG_PRINT("\n\"%s\" bin size is %i. Contents:\n\n", argv[1], fInSz);
	DEBUG_printBytesIn01s(inBytes, fInSz, 4);
	DEBUG_PRINT("\n\n");
		
	//MASTER LOOP
	while(bytesDone < fInSz)
	{///Each iteration disassembles one instruction

		DEBUG_PRINT("Beginning work on instruction %i. ", instrsDone+1);
		DEBUG_PRINT("First byte: ");
		DEBUG_printBytesIn01s(instrP, 1, 0);
		DEBUG_PRINT("\n");

		//Set opForm, subOpVal, and mnemName
		if(0); 
		else if(0b11010111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"xlat");}///PURE
		else if(0b10011111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"lahf");}///PURE
		else if(0b10011110==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"sahf");}///PURE
		else if(0b10011100==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"pushf");}///PURE 
		else if(0b10011101==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"popf");}///PURE
		else if(0b00110111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"aaa");}///PURE
		else if(0b00100111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"daa");}///PURE
		else if(0b00111111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"aas");}///PURE
		else if(0b00101111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"das");}///PURE
		else if(0b10011000==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"cbw");}///PURE
		else if(0b10011001==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"cwd");}///PURE
		else if(0b11000011==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"ret");}///PURE(within segment)
		else if(0b11001011==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"ret");}///PURE(inter-segment)
		else if(0b11001100==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"int");}///PURE(type3)
		else if(0b11001110==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"into");}///PURE
		else if(0b11001111==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"iret");}///PURE
		else if(0b11111000==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"clc");}///PURE
		else if(0b11110101==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"cmc");}///PURE
		else if(0b11111001==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"stc");}///PURE
		else if(0b11111100==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"cld");}///PURE
		else if(0b11111101==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"std");}///PURE
		else if(0b11111010==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"cli");}///PURE
		else if(0b11111011==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"sti");}///PURE
		else if(0b11110100==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"hlt");}///PURE
		else if(0b10011011==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"wait");}///PURE
		else if(0b11110000==instrP[0]) /*NO MASK*/ {opForm=  S1oooooooo  ;strcpy(mnemName,"lock");}///PURE
		else if(0b10010000==(instrP[0]&0b11111000)){opForm=  S1oooooReg  ;strcpy(mnemName,"xchg");}///to:with REG
		else if(0b01010000==(instrP[0]&0b11111000)){opForm=  S1oooooReg  ;strcpy(mnemName,"push");}///to:with REG
		else if(0b01011000==(instrP[0]&0b11111000)){opForm=  S1oooooReg  ;strcpy(mnemName,"pop");}///to:with REG
		else if(0b01000000==(instrP[0]&0b11111000)){opForm=  S1oooooReg  ;strcpy(mnemName,"inc");}///to:with REG
		else if(0b01001000==(instrP[0]&0b11111000)){opForm=  S1oooooReg  ;strcpy(mnemName,"dec");}///to:with REG
		else if(0b00000110==(instrP[0]&0b11100111)){opForm=  S1oooSrooo  ;strcpy(mnemName,"push");}///segment register
		else if(0b00000111==(instrP[0]&0b11100111)){opForm=  S1oooSrooo  ;strcpy(mnemName,"pop");}///segment register
		else if(0b11101100==(instrP[0]&0b11111110)){opForm=  S1oooooooW  ;strcpy(mnemName,"in");}///Variable port
		else if(0b11101110==(instrP[0]&0b11111110)){opForm=  S1oooooooW  ;strcpy(mnemName,"out");}///Variable port
		else if(0b01110100==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"je");}
		else if(0b01111100==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jl");}
		else if(0b01111110==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jle");}
		else if(0b01110010==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jb");}
		else if(0b01110110==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jbe");}
		else if(0b01111010==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jp");}
		else if(0b01110000==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jo");}
		else if(0b01111000==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"js");}
		else if(0b01110101==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jnz");}
		else if(0b01111101==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jnl");}
		else if(0b01111111==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jg");}
		else if(0b01110011==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jnb");}
		else if(0b01110111==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"ja");}
		else if(0b01111011==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jnp");}
		else if(0b01110001==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jno");}
		else if(0b01111001==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jns");}
		else if(0b11100010==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"loop");}
		else if(0b11100001==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"loopz");}
		else if(0b11100000==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"loopnz");}
		else if(0b11100011==instrP[0]) /*NO MASK*/ {opForm=  S2oooooooo_LABEL  ;strcpy(mnemName,"jcxz");}
		else if(0b11100100==(instrP[0]&0b11111110)){opForm=  S2oooooooW_Data  ;strcpy(mnemName,"in");}///Fixed port
		else if(0b11100110==(instrP[0]&0b11111110)){opForm=  S2oooooooW_Data  ;strcpy(mnemName,"out");}///Fixed port
		else if(0b10100000==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"mov");}///MEM to ACC
		else if(0b10100010==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"mov");}///ACC to MEM
		else if(0b00000100==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"add");}///IMM to ACC
		else if(0b00010100==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"adc");}///IMM to ACC
		else if(0b00101100==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"sub");}///IMM (from) ACC
		else if(0b00011100==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"sbb");}///IMM (from) ACC
		else if(0b00111100==(instrP[0]&0b11111110)){opForm=  S3oooooooW_DaAd_DaAw  ;strcpy(mnemName,"cmp");}///IMM (from) ACC
		else if(0b10110000==(instrP[0]&0b11110000)){opForm=  S3ooooWReg_Data_Datw  ;strcpy(mnemName,"mov");}///IMM to REG
		else if(0b10001000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"mov");}///RM to:from REG
		else if(0b10000110==(instrP[0]&0b11111110)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"xchg");}///always D
		else if(0b00000000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"add");}///RM to:from REG
		else if(0b00010000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"adc");}///RM to:from REG
		else if(0b00101000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"sub");}///RM to:from REG
		else if(0b00011000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"sbb");}///RM to:from REG
		else if(0b00111000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"cmp");}///RM to:from REG
		else if(0b00001000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"or");}///RM to:from REG
		else if(0b00110000==(instrP[0]&0b11111100)){opForm=  S4ooooooDW_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"xor");}///RM to:from REG
		else if(0b11111110==(instrP[0]&0b11111110))        //S4oooooooW_MdSubRgm_Disp_Disp
		{
			opForm=  S4oooooooW_MdSubRgm_Disp_Disp;
			subOpVal = (instrP[1]&0b00111000)>>3;
			printf("11111110 SWOOCH\n");
			printf("11111110 SWOOCH\n");
			printf("11111110 SWOOCH\n");
			switch(subOpVal)
			{
				case 0b000:{strcpy(mnemName, "inc"); break;} //1111111w
				case 0b001:{strcpy(mnemName, "dec"); break;} //1111111w
				case 0b110:{strcpy(mnemName, "push"); break;} //11111111
				default:{printf("[ERROR selecting subOp for 1111111x opcode!]\n");}
			}
		}
		else if(0b10001110==(instrP[0]&0b11111110)){opForm=  S4oooooooW_MdSubRgm_Disp_Disp  ;strcpy(mnemName,"pop");}///to RM (sub?)
		else if(0b11110110==(instrP[0]&0b11111110)){opForm=  S4oooooooW_MdSubRgm_Disp_Disp  ;strcpy(mnemName,"neg");}
		else if(0b10001101==instrP[0]) /*NO MASK*/ {opForm=  S4oooooooo_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"lea");}
		else if(0b11000101==instrP[0]) /*NO MASK*/ {opForm=  S4oooooooo_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"lds");}
		else if(0b11000100==instrP[0]) /*NO MASK*/ {opForm=  S4oooooooo_MdRegRgm_Disp_Disp  ;strcpy(mnemName,"les");}
		else if(0b00110100==(instrP[0]&0b11111100)){opForm=  S6oooooooW_DATADATA_Di_Di_Da_Da;  strcpy(mnemName, "xor");}
		else if(0b11000110==(instrP[0]&0b11111110)){opForm=  S6ooooooSW_MdSubRgm_Di_Di_Da_Da;  strcpy(mnemName, "mov");}///always S(pseudo)
		else if(0b11110110==(instrP[0]&0b11111110)){opForm=  S6ooooooSW_MdSubRgm_Di_Di_Da_Da;  strcpy(mnemName, "test");}///always S(pseudo)
		else if(0b10000000==(instrP[0]&0b11111100))        //S6ooooooSW_MdSubRgm_Di_Di_Da_Da
		{
			//add, or, adc, sbb, and, sub, adc, cmp
			opForm = S6ooooooSW_MdSubRgm_Di_Di_Da_Da;
			subOpVal = (instrP[1]&0b00111000)>>3;
			switch(subOpVal)
			{
				case 0b000:{strcpy(mnemName, "add"); break;} //100000sw
				case 0b001:{strcpy(mnemName, "or"); break;} ///1000000w, always !S(pseudo)
				case 0b010:{strcpy(mnemName, "adc"); break;} //100000sw
				case 0b011:{strcpy(mnemName, "sbb"); break;} //100000sw
				case 0b100:{strcpy(mnemName, "and"); break;}///1000000w, always !S(pseudo)
				case 0b101:{strcpy(mnemName, "sub"); break;} //100000sw
				case 0b110:{strcpy(mnemName, "???"); break;} //????????
				case 0b111:{strcpy(mnemName, "cmp"); break;} //100000sw
				default:{printf("[ERROR selecting subOp for 100000xx opcode!]\n");}
			}
		}
		else{ERROR_TERMINATE(inBytes, instrSizes, instrsDone, fOutP, "ERROR SELECTING MNEMONIC."); return 1;}
	
		//Select disassembly logic
		switch(opForm)
		{
			case S1oooooooo: //PURE
			{
				instrSz=1;
				
				DEBUG_PRINT("  opForm: S1oooooooo (PURE)\n");
				DEBUG_PRINT("  %s (pure) Size: %i\n", mnemName, instrSz);
				
				//Build output string
				sprintf(instrStrings[instrsDone], "%s", mnemName);
			}break;
			case S1oooooReg: //to:with REG
			{///xchg, inc, dec, push, pop
				fillRegName(regName, instrP[0]&0b00000111, USE_WORD);
				instrSz = 1;
				
				DEBUG_PRINT("  opForm: S1oooooReg\n");
				DEBUG_PRINT("  %s (to:with REG) Size: %i\n", mnemName, instrSz);
				
				//Build output string
				if( !strcmp(mnemName, "xchg") )
				{
					//(2 operands)
					sprintf(instrStrings[instrsDone], "%s ax, %s", mnemName, regName);
				}
				else
				{
					//(1 operand)
					sprintf(instrStrings[instrsDone], "%s %s", mnemName, regName);
				}
			}break;
			case S1oooooooW: //variable port
			{///in, out
				W = (bool)(instrP[0] & 0b00000001);
				D = !(bool)(instrP[0] & 0b00000010);///pseudo, ACC is dest (for IN and OUT operand orders)
				instrSz = 1;
				
				DEBUG_PRINT("  opForm: S1oooooooW\n");
				DEBUG_PRINT("  %s (variable port) Size: %i\n", mnemName, instrSz);
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				DEBUG_PRINT("  %s\n", D?"D pseudo (from flipped xxxxxx0x bit)":"!D pseudo (from flipped xxxxxx1x bit)");
				
				//Build output string
				if(D)
				{
					sprintf(instrStrings[instrsDone], "%s %s, dx", mnemName, W?"ax":"al");
				}///                                  mne a_, dx
				else
				{
					sprintf(instrStrings[instrsDone], "%s dx, %s", mnemName, W?"ax":"al");
				}///                                  mne dx  a_  
			}break;
			case S2oooooooW_Data: //fixed port
			{///in, out
				W = (bool)(instrP[0] & 0b00000001);
				D = !(bool)(instrP[0] & 0b00000010);///pseudo, ACC is dest (for IN and OUT operand orders)
				instrSz = 2;
				
				DEBUG_PRINT("  opForm: S1oooooooW\n");
				DEBUG_PRINT("  %s (fixed port) Size: %i\n", mnemName, instrSz);
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				DEBUG_PRINT("  %s\n", D?"D pseudo (from flipped xxxxxx0x bit)":"!D pseudo (from flipped xxxxxx1x bit)");
				
				//DATA
				dataOrAddr = valFromUCharP(instrP+1, W);
				
				//Build output string
				if(D)
				{
					sprintf(instrStrings[instrsDone], "%s %s, %i", mnemName, W?"ax":"al", dataOrAddr);
				}///                                  mne a_, __
				else
				{
					sprintf(instrStrings[instrsDone], "%s %i, %s", mnemName, dataOrAddr, W?"ax":"al");
				}///                                  mne __, a_
			}break;
			case S1oooSrooo: //segment register
			{///push, pop
				srBitsVal = (instrP[0]&0b00011000)>>3;
				instrSz=1;

				DEBUG_PRINT("  opForm: S1oooSrooo\n");
				DEBUG_PRINT("  %s (segment register) Size: %i\n", mnemName, instrSz);
				
				//Build output string
				sprintf(instrStrings[instrsDone], "%s %s", mnemName, srNames[srBitsVal]);
			}break;
			case S4ooooooDW_MdRegRgm_Disp_Disp: //RM to:from REG
			{///MOV, XCHG(D), ADD, SUB, CMP, OR, XOR
				D = (bool)(instrP[0] & 0b00000010);
				W = (bool)(instrP[0] & 0b00000001);
				modBitsVal = instrP[1]>>6;
				regBitsVal = (instrP[1]&0b00111000)>>3;
				rmBitsVal = (instrP[1]&0b00000111);
				dispSz = modBitsVal%3;
				instrSz = 2 + dispSz;
	
				DEBUG_PRINT("  opForm: S4ooooooDW_MdRegRgm_Disp_Disp\n");
				DEBUG_PRINT("  %s (RM TO:FROM REG)\n", mnemName);
				DEBUG_PRINT("  %s\n", D?"D":"!D");
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				DEBUG_PRINT(modMsgs[modBitsVal]);
				printf("  dispSize = %i\n", dispSz);
				printf("  modBits = %i\n", modBitsVal);
				printf("  modBits%3 = %i\n", modBitsVal%3);
				
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
					sprintf(instrStrings[instrsDone], "%s %s, %s", mnemName, regName, rmName);
				}
				else
				{	//XXX R/M, REG
					sprintf(instrStrings[instrsDone], "%s %s, %s", mnemName, rmName, regName);
				}
			}break;
			case S6ooooooSW_MdSubRgm_Di_Di_Da_Da: //IMM to RM
			{///MOV, add, or, adc, sbb, and, sub, adc, cmp
				S = (bool)(instrP[0] & 0b00000010);
				W = (bool)(instrP[0] & 0b00000001);
				modBitsVal = instrP[1]>>6;
				rmBitsVal = (instrP[1]&0b00000111);
				dispSz = modBitsVal%3;
				
				switch (instrP[0] & 0b11111110)
				{
					//Cases here don't have/don't care about S for data size.
					case 0b11000110: ///mov
					case 0b11110110: ///test
					case 0b10000000: ///or, sub
					{
						dataSz = 1 + (char)(W);
					}break;
					default:
					{
						dataSz = 1 + (char)(W && !S);
					}
				}
				if( !strcmp(mnemName, "mov") )
				{
					//mov doesn't have S, only cares about W
					dataSz = 1 + (char)W;
				}
				else
				{
					//"and" doesn't have s, but may as well.
					
				}
				
				instrSz = 2 + dispSz + dataSz;
				
				DEBUG_PRINT("  opForm: S6ooooooSW_MdSubRgm_Di_Di_Da_Da\n");
				DEBUG_PRINT("  %s (IMM to RM)\n", mnemName);
				DEBUG_PRINT("  %s\n", S?"S":"!S");
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
					//DIRECT ACCESS special case
					dispSz = 2;
					instrSz += dispSz;
					dispVal = valFromUCharP(instrP+2, USE_WORD);
				}
				
				//DATA
				dataOrAddr = valFromUCharP(instrP+2+dispSz, (dataSz>1));
				MemAddrMode = false;
				
				//RM name generation
				fillRmName(rmName, rmBitsVal, modBitsVal, W, dispVal);
				
				//Build output string
				if(modBitsVal == 0b11)
				{
					sprintf(instrStrings[instrsDone], "%s %s, %i", mnemName, rmName, dataOrAddr);
				}
				else
				{
					sprintf(instrStrings[instrsDone], "%s %s, %s %i", mnemName, rmName, W?"word":"byte", dataOrAddr);
				}
			}break;
			case S3ooooWReg_Data_Datw: //IMM to REG
			{///MOV
				W = (bool)(instrP[0] & 0b00001000);
				regBitsVal = (instrP[0] & 0b00000111);
				dataSz = 1 + (char)W;
				instrSz = 1 + dataSz;
				
				DEBUG_PRINT("  opForm: S3ooooWReg_Data_Datw\n");
				DEBUG_PRINT("  %s (IMM to REG)\n", mnemName);
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				
				//DATA or ADDR
				dataOrAddr = valFromUCharP(instrP+1, (dataSz>1));
				
				//REG name generation
				fillRegName(regName, regBitsVal, W);
				
				//Build output string
				sprintf(instrStrings[instrsDone], "%s %s, %i", mnemName, regName, dataOrAddr);
			}break;
			case S3oooooooW_DaAd_DaAw: //MEM:IMM to:from ACC
			{///MOV, ADD, SUB, CMP(data size always 1)
				//NOTE: We're using "xxxxxx0x" to flip the 7th bit,
				//then using it as "D". If D, then ax is the Dest.
				//This seems like it'll work, but time will tell.
				W = (bool)(instrP[0] & 0b00000001);
				D = !(bool)(instrP[0] & 0b00000010);///pseudo
				dataSz = 1 + (char)W;
				instrSz = 1 + dataSz;
				
				DEBUG_PRINT("  opForm: S3ooooWReg_Data_Datw\n");
				DEBUG_PRINT("  %s (MEM:IMM to:from ACC)\n", mnemName);
				DEBUG_PRINT("  %s\n", W?"W":"!W");
				DEBUG_PRINT("  %s\n", D?"D pseudo (from flipped xxxxxx0x bit)":"!D pseudo (from flipped xxxxxx1x bit)");
				
				//DATA or ADDR
				dataOrAddr = valFromUCharP(instrP+1, (dataSz>1));
				MemAddrMode = (0b10100000==(instrP[0]&11111100));//101000xw
				
				
				//Build output string
				if(D) //Mem address mode needs [brackets]. Use AX if W, else AL if !W.
				{
					sprintf(instrStrings[instrsDone], "%s %s, %s%i%s", mnemName, W?"ax":"al", MemAddrMode?"[":"", dataOrAddr, MemAddrMode?"]":"");
				}///                                  mne a_,  [__]
				else
				{
					sprintf(instrStrings[instrsDone], "%s %s%i%s, %s", mnemName, MemAddrMode?"[":"", dataOrAddr, MemAddrMode?"]":"", W?"ax":"al");
				}///                                  mne  [__] , a_
			}break;
			case S4oooooooW_MdSubRgm_Disp_Disp: //RM
			{///push, pop
				W=(instrP[0]&0b00000001);
				modBitsVal = instrP[1]>>6;
				rmBitsVal = instrP[1]&0b00000111;
				dispSz = modBitsVal%3;
				instrSz = 2 + dispSz;
				
				DEBUG_PRINT("  opForm: S4oooooooW_MdSubRgm_Disp_Disp\n");
				DEBUG_PRINT("  %s (MEM:IMM to:from ACC)\n", mnemName);
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
				
				//RM name generation
				fillRmName(rmName, rmBitsVal, modBitsVal, W, dispVal);
				
				//Build output string
				if(0)//modBitsVal == 0b11)
				{
					sprintf(instrStrings[instrsDone], "%s %s, %i", mnemName, rmName, dispVal);
				}
				else
				{
					sprintf(instrStrings[instrsDone], "%s %s %s", mnemName, (W)?"word":"byte", rmName);
				}
			}break;
			case S4oooooooo_MdRegRgm_Disp_Disp:
			{///lea, lds, les
				W = true; //Assumed
				modBitsVal = instrP[1]>>6;
				regBitsVal = (instrP[1]&0b00111000)>>3;
				rmBitsVal = (instrP[1]&0b00000111);
				dispSz = modBitsVal%3;
				instrSz = 2 + dispSz;
			
				DEBUG_PRINT("  opForm: S4oooooooW_MdSubRgm_Disp_Disp\n");
				DEBUG_PRINT("  %s (MEM:IMM to:from ACC)\n", mnemName);
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
				
				sprintf(instrStrings[instrsDone], "%s %s, %s", mnemName, regName, rmName);
			}break;
			case S2oooooooo_LABEL: //to label
			{
				dataOrAddr = valFromUCharP(instrP+1, USE_BYTE);
				instrSz = 2;
				
				DEBUG_PRINT("  opForm: S2oooooooo_LABEL\n");
				DEBUG_PRINT("  %s (to label)\n", mnemName);
				DEBUG_PRINT("  Offset to label: %i\n", dataOrAddr);
				
				//Note that dataOrAddr is a byte offset for a label in this case.
				labelIndex = bytesDone+instrSz+dataOrAddr;
				
				//Check for existence of this label.
				labelExists = false;
				for(int i=0 ; i<labelCount ; i++)
				{
					if(labelIndices[i] == labelIndex)
					{
						labelExists=true;
					}
				}
				if(!labelExists)
				{
					labelIndices[labelCount] = labelIndex;
					labelCount++;
				}
				
				//Build output string
				sprintf(instrStrings[instrsDone], "%s label%i", mnemName, labelIndex);
			}break;
			default:
			{///ERROR
				printf("ERROR: Unrecognized instruction type. Terminating.\n");
				return 1;
			}
		}
		
		//Loop upkeep
		DEBUG_PRINT("  Successfully disassembled -> \"%s\" [Writing...]\n", instrStrings[instrsDone]);		
		DEBUG_PRINT("  Instruction size: %i\n  ", instrSz);
		DEBUG_printBytesIn01s(instrP, instrSz, 0); DEBUG_PRINT("\n");
		instrP += instrSz;
		bytesDone += instrSz;
		//instrLengths[instrsDone] = strlen(instrString);
		instrSizes[instrsDone++] = instrSz;
		DEBUG_PRINT("  Bytes processed: %i/%i\n", instrP-inBytes, fInSz);
	}//End of disassembling

	if(labelCount)
	{//Sort labels
		bool labelsSorted = false;
		unsigned long int labelBucket;
		while(!labelsSorted)
		{
			//Each iteration makes one swapping pass
			labelsSorted = true;
			for(int i=0 ; i<(labelCount-1) ; i++)
			{
				if(labelIndices[i] > labelIndices[i+1])
				{
					//Switch i and i+1
					labelBucket = labelIndices[i];
					labelIndices[i] = labelIndices[i+1];
					labelIndices[i+1] = labelBucket;
					labelsSorted = false;
				}
			}
		}
	}
	
	//Write to output file
	unsigned long int bytesWritten = 0;
	unsigned long int labelsWritten = 0;
	fprintf(fOutP, "%s", "bits 16"); //Bit width directive counts as 0 bytes
	for(int i=0 ; i<instrsDone ; i++)
	{//Each iteration writes 1 instr
		fprintf(fOutP, "\n%s", instrStrings[i]);
		bytesWritten += instrSizes[i];
		
		if(labelsWritten < labelCount)
		{
			//labelIndices[] are byte numbers that need labels after
			//Labels are named after the index of the preceding byte
			if(labelIndices[labelsWritten] == bytesWritten)
			{
				fprintf(fOutP, "\nlabel%i:", bytesWritten);
				labelsWritten++;
			}
		}
	}

	//Get output file size
	fseek(fOutP, 0, SEEK_END); //Set file position to end
	int fOutSz = ftell(fOutP); //Store offset (file out size)

	DEBUG_PRINT("Disassembly DONE.\n");
	DEBUG_PRINT("Total instructions: %i\n", instrsDone);
	DEBUG_PRINT("Total labels: %i\n", labelCount);
	DEBUG_PRINT("File IN size: %i\n", fInSz);
	DEBUG_PRINT("File OUT size: %i\n", fOutSz);

	//Print output file contents to stdout
	printf("\n\n::: CONTENTS OF OUTPUT FILE :::\n");
	printf("%s", *fOutP);

	//Done.
	fclose(fInP);
	fclose(fOutP);
	//system("pause");
	DEBUG_PRINT("\n\nExecution time: %d(ms?)\n", clock()-starttime);
	return 0;
}
//END MAIN



void ERROR_TERMINATE(unsigned char *inBytesP,unsigned char *instrSizes,unsigned long long int instrsDone,unsigned char*fOutP,char msg[])
{
	printf("[%s]\n", msg);
	printf("[%s]\n", msg);
	printf("[%s]\n", msg);
	DEBUG=true;
	printf("[DEBUG mode activated.]\n");
	printf("[Printing instr bytes so far...]\n\n");
	
	for(int i=0 ; i<instrsDone ; i++)
	{
		DEBUG_PRINT("[Instruction %i]\n", i+1);
		DEBUG_printBytesIn01s(inBytesP, instrSizes[i], 0);
		DEBUG_PRINT("\n");
		inBytesP += instrSizes[i];
	}
	printf("Next 6 unprocessed bytes:\n");
	DEBUG_printBytesIn01s(inBytesP, 6, 0);
	
	//Print output file contents to stdout
	printf("\n\n::: CONTENTS OF OUTPUT FILE :::\n");
	printf("%s", *fOutP);
	
	printf("\n\n[Terminating...]\n");
	//exit(69);
}
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