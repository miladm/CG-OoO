#include "uop_gen.h"

// Enable micro-ops (set to 1 for enable)
#define ENABLE_MICRO 0

// Temporary micro-op registers
#define MICRO0 "TEMP0"
#define MICRO1 "TEMP1"
#define MICRO2 "TEMP2"
#define MICRO3 "TEMP3"
#define MICRO4 "TEMP4"
#define MICRO5 "TEMP5"
#define MICRO6 "TEMP6"
#define MICRO7 "TEMP7"

FILE* out = 0;
FILE* out1 = 0;
std::stringstream ss;
staticCodeParser * _g_staticCode;
map <ADDRINT,string> insMap;
unsigned j;
int memOp, aluOp, fpOp, brOp;
//
//The three program flags for partial trace generation
int strt = 0;
int fin   = 0;
int warmUp = 0;
int runAllProgram = true;  //Ignores program flags if true
bool logEnable = false;    //Set true to run the log file. (recommended: false)

bool isFloat (INS);
bool isFDiv (INS);

//=======================================================
//  Analysis routines
//=======================================================

//Memory Write, Split
VOID dosAsmMEMWS (ADDRINT ip, ADDRINT instAddr, VOID * addr, int regCount, UINT32 isFloat, UINT32 isFDiv, UINT32 MAXR, UINT32 MAXW,
REG R1, REG R2, REG R3, REG R4, REG R5, REG R6, REG R7, REG R8,
REG W1, REG W2, REG W3, REG W4, REG W5, REG W6, REG W7, REG W8)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        int subAddr = 0;
        string original = insMap[ip].c_str ();
        string RRegs[8];
        string WRegs[8];
        string TRegs[8];
        string MORegs[8];
        MORegs[0] = MICRO0;
        MORegs[1] = MICRO1;
        MORegs[2] = MICRO2;
        MORegs[3] = MICRO3;
        MORegs[4] = MICRO4;
        MORegs[5] = MICRO5;
        MORegs[6] = MICRO6;
        MORegs[7] = MICRO7;
        int TRegPtr = 0;
        int RMask = 0x0;
        for (UINT32 i = 0; i < MAXR; i++) {
            if (i == 0) {
                RRegs[i] = REG_StringShort (R1);
                RMask |= 0x1;
            } else if (i == 1) {
                RRegs[i] = REG_StringShort (R2); 
                RMask |= 0x2;
            } else if (i == 2) {
                RRegs[i] = REG_StringShort (R3); 
                RMask |= 0x4;
            } else if (i == 3) {
                RRegs[i] = REG_StringShort (R4); 
                RMask |= 0x8;
            } else if (i == 4) {
                RRegs[i] = REG_StringShort (R5); 
                RMask |= 0x10;
            } else if (i == 5) {
                RRegs[i] = REG_StringShort (R6); 
                RMask |= 0x20;
            } else if (i == 6) {
                RRegs[i] = REG_StringShort (R7); 
                RMask |= 0x40;
            } else if (i == 7) {
                RRegs[i] = REG_StringShort (R8); 
                RMask |= 0x80;
            }
        }
        for (UINT32 i = 0; i < MAXW; i++) {
            if (i == 0) {
                WRegs[i] = REG_StringShort (W1);
            } else if (i == 1) {
                WRegs[i] = REG_StringShort (W2); 
            } else if (i == 2) {
                WRegs[i] = REG_StringShort (W3); 
            } else if (i == 3) {
                WRegs[i] = REG_StringShort (W4); 
            } else if (i == 4) {
                WRegs[i] = REG_StringShort (W5); 
            } else if (i == 5) {
                WRegs[i] = REG_StringShort (W6); 
            } else if (i == 6) {
                WRegs[i] = REG_StringShort (W7); 
            } else if (i == 7) {
                WRegs[i] = REG_StringShort (W8); 
            }
        }
        int MPointers = 0;
        int addrOps = 0;
        for (UINT32 i = 0; i < original.length (); i++) {
            if (original[i] == '[') {
                for (UINT32 k = i; k < original.length (); k++) {
                    if (original[k] == ']') {
                        break;
                    } else if ( (original[k] == '+') || (original[k] == '-') || (original[k] == '*')) {
                        addrOps++;
                    }
                }
                MPointers++;
                int RLength = 0;
                for (UINT32 ii = i; ii < original.length (); ii++) {
                    if ( (original[ii] == ']') || (original[ii] == ' ') || (original[ii] == '+') || (original[ii] == '-') || (original[ii] == '*')) {
                        RLength = ii-i-1;
                        break;
                    }
                }
                for (UINT32 j = 0; j < MAXR; j++) {
                    if (RRegs[j].compare (original.substr (i+1, RLength)) == 0) {
                        RMask &= ~ (0x1 << j);
                        TRegs[TRegPtr] = RRegs[j].c_str ();
                        TRegPtr++;
                        break;
                    }
                }
            }
        }
        TRegPtr = 0;
        int MORegPtr = 0;
        for (int i = 0; i < addrOps; i++) {
            fprintf (out1,"\nA,%llu-%i,", (long long unsigned int)instAddr, subAddr);	
            fprintf (out1,"%s#%d,",MORegs[0].c_str (), 2);
            ss << "\nA," << (long long unsigned int)instAddr << "-" << subAddr << ",";	
            ss << MORegs[0].c_str () << "#2,";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            aluOp++;
            subAddr++;
        }	
        for (int i = 0; i < MPointers; i++) {
            fprintf (out1,"\nR,%llu,%llu-%i,", (long long unsigned int)addr, (long long unsigned int)instAddr, subAddr);	
            fprintf (out1,"%s#%d,",TRegs[TRegPtr].c_str (), 1);
            fprintf (out1,"%s#%d,",MORegs[TRegPtr].c_str (), 2);
            ss << "\nR," << (long long unsigned int)addr << "," << (long long unsigned int)instAddr << "-" << subAddr << ",";	
            ss << TRegs[TRegPtr].c_str () << "#1,";
            ss << MORegs[TRegPtr].c_str () << "#2,";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            TRegPtr++;
            memOp++;
            subAddr++;
        }
        ss << "\n";
        if (isFDiv) {
            fprintf (out1,"\nD,%llu-%i,", (long long unsigned int)instAddr, subAddr);
            ss << "\nD," << (long long unsigned int)instAddr << "-" << subAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            fpOp++;
        } else if (isFloat) {
            fprintf (out1,"\nF,%llu-%i,", (long long unsigned int)instAddr, subAddr);
            ss << "\nF," << (long long unsigned int)instAddr << "-" << subAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            fpOp++;
        } else {
            fprintf (out1,"\nA,%llu-%i,", (long long unsigned int)instAddr, subAddr);
            ss << "\nA," << (long long unsigned int)instAddr << "-" << subAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            aluOp++;
        }
        subAddr++;
        for (UINT32 i = 0; i < MAXR; i++) {
            if (RMask & (0x1 << i)) {
                fprintf (out1,"%s#%d,", RRegs[i].c_str (), 1);
                ss << RRegs[i].c_str () << "#1,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            } else {
                fprintf (out1,"%s#%d,", MORegs[MORegPtr].c_str (), 1);
                ss << MORegs[MORegPtr].c_str () << "#1,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
                MORegPtr++;
            }
        }
        int MORegSPtr = MORegPtr;
        int WMask = 0;
        for (UINT32 i = 0; i < MAXW; i++) {
            bool flags = WRegs[i].compare ("rflags");
            if (flags == 0) {
                fprintf (out1,"%s#%d,", WRegs[i].c_str (), 2); 
                ss << WRegs[i].c_str () << "#2,"; 
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            } else {
                fprintf (out1,"%s#%d,", MORegs[MORegPtr].c_str (), 2);
                ss << MORegs[MORegPtr].c_str () << "#2,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
                MORegPtr++;
                WMask |= (0x1 << i);
            }
        }
        fprintf (out1,"\nW,%llu,%llu-%i,", (long long unsigned)addr, (long long unsigned)instAddr, subAddr);
        ss << "\nW," << (long long unsigned)addr << "," << (long long unsigned)instAddr << "-" << subAddr << ",";
        g_var.g_ins = ss.str ();
        //ss.str (std::string ());
        memOp++;
        subAddr++;
        for (UINT32 i = 0; i < MAXW; i++) {
            if (WMask & (0x1 << i)) {
                fprintf (out1,"%s#%d,", MORegs[MORegSPtr].c_str (), 1);
                ss << MORegs[MORegSPtr].c_str () << "#1,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
                MORegSPtr++;
            }
        }	    
        for (UINT32 i = 0; i < MAXR; i++) {
            if (! (RMask & (0x1 << i))) {
                fprintf (out1,"%s#%d,", RRegs[i].c_str (), 1);
                ss << RRegs[i].c_str () << "#1,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            }
        }
    }
    //ss.str (std::string ());
}
//Memory Write OP
VOID dosAsmMEMW (ADDRINT ip, ADDRINT instAddr, VOID * addr, int regCount, UINT32 isFloat, UINT32 isFDiv)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        if (isFloat) {
            fpOp++;
            if (isFDiv) {
                if (logEnable) fprintf (out, "\n--- D, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
                fprintf (out1,"\nD,%llu,", (long long unsigned int)instAddr);
                ss << "\nD," << (long long unsigned int)instAddr << ",";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            } else {
                if (logEnable) fprintf (out, "\n--- F, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
                fprintf (out1,"\nF,%llu,", (long long unsigned int)instAddr);
                ss << "\nF," << (long long unsigned int)instAddr << ",";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            }
        } else {
            if (logEnable) fprintf (out, "\n--- W, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
            if (logEnable) fprintf (out, "\n>> ");
            fprintf (out1,"\nW,%llu,%llu,", (long long unsigned)addr, (long long unsigned)instAddr);
            ss << "\nW," << (long long unsigned)addr << "," << (long long unsigned)instAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
        }
        memOp++;
    }
}
//Memory Write OP
VOID dosAsmMEMW1 (ADDRINT ip, ADDRINT instAddr, VOID * addr, int regCount, UINT32 isFloat, UINT32 isFDiv)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        if (isFloat) {
            fpOp++;
            if (isFDiv) {
                if (logEnable) fprintf (out, "\n--- D, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
            } else {
                if (logEnable) fprintf (out, "\n--- F, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
            }
        } else {
            if (logEnable) fprintf (out, "\n--- W, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
            if (logEnable) fprintf (out, "\n>> ");
        } 
        memOp++;
    }
}
//Memory Read OP
VOID dosAsmMEMR (ADDRINT ip, ADDRINT instAddr, VOID * addr, int regCount, UINT32 isFloat, UINT32 isFDiv)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        if (isFloat) {
            fpOp++;
            if (isFDiv) {
                if (logEnable) fprintf (out, "\n--- D, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
                fprintf (out1,"\nD,%llu,", (long long unsigned int)instAddr);
                ss << "\nD," << (long long unsigned int)instAddr << ",";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            } else {
                if (logEnable) fprintf (out, "\n--- F, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
                fprintf (out1,"\nF,%llu,", (long long unsigned int)instAddr);
                ss << "\nF," << (long long unsigned int)instAddr << ",";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            }
        } else {
            if (logEnable) fprintf (out, "\n--- R, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
            if (logEnable) fprintf (out, "\n>> ");
            fprintf (out1,"\nR,%llu,%llu,", (long long unsigned int)addr, (long long unsigned int)instAddr);
            ss << "\nR," << (long long unsigned int)addr << "," << (long long unsigned int)instAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
        }
        memOp++;
    }
}

//Memory Read OP
VOID dosAsmMEMR1 (ADDRINT ip, ADDRINT instAddr, VOID * addr, int regCount, UINT32 isFloat, UINT32 isFDiv)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        if (isFloat) {
            fpOp++;
            if (isFDiv) {
                if (logEnable) fprintf (out, "\n--- D, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
            } else {
                if (logEnable) fprintf (out, "\n--- F, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
            }
        } else { 
            if (logEnable) fprintf (out, "\n--- R, %d, IP:%ld, ADDR:%llu, ASM: (%s)\n", regCount, ip, (long long unsigned int)addr, insMap[ip].c_str ());
            if (logEnable) fprintf (out, "\n>> ");
        }
        memOp++;}
}
//Memory Read, Split
VOID dosAsmMEMRS (ADDRINT ip, ADDRINT instAddr, VOID * addr, int regCount, UINT32 isFloat, UINT32 isFDiv, UINT32 MAXR, UINT32 MAXW,
REG R1, REG R2, REG R3, REG R4, REG R5, REG R6, REG R7, REG R8)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        int subAddr = 0;
        string original = insMap[ip].c_str ();
        string RRegs[8];
        string TRegs[8];
        string MORegs[8];
        MORegs[0] = MICRO0;
        MORegs[1] = MICRO1;
        MORegs[2] = MICRO2;
        MORegs[3] = MICRO3;
        MORegs[4] = MICRO4;
        MORegs[5] = MICRO5;
        MORegs[6] = MICRO6;
        MORegs[7] = MICRO7;
        int TRegPtr = 0;
        int RMask = 0x0;
        for (UINT32 i = 0; i < MAXR; i++) {
            if (i == 0) {
                RRegs[i] = REG_StringShort (R1);
                RMask |= 0x1;
            } else if (i == 1) {
                RRegs[i] = REG_StringShort (R2); 
                RMask |= 0x2;
            } else if (i == 2) {
                RRegs[i] = REG_StringShort (R3); 
                RMask |= 0x4;
            } else if (i == 3) {
                RRegs[i] = REG_StringShort (R4); 
                RMask |= 0x8;
            } else if (i == 4) {
                RRegs[i] = REG_StringShort (R5); 
                RMask |= 0x10;
            } else if (i == 5) {
                RRegs[i] = REG_StringShort (R6); 
                RMask |= 0x20;
            } else if (i == 6) {
                RRegs[i] = REG_StringShort (R7); 
                RMask |= 0x40;
            } else if (i == 7) {
                RRegs[i] = REG_StringShort (R8); 
                RMask |= 0x80;
            }
        }
        int MPointers = 0;
        int addrOps = 0;
        for (UINT32 i = 0; i < original.length (); i++) {
            if (original[i] == '[') {
                for (UINT32 k = i; k < original.length (); k++) {
                    if (original[k] == ']') {
                        break;
                    } else if ( (original[k] == '+') || (original[k] == '-') || (original[k] == '*')) {
                        addrOps++;
                    }
                }
                MPointers++;
                int RLength = 0;
                for (UINT32 ii = i; ii < original.length (); ii++) {
                    if ( (original[ii] == ']') || (original[ii] == ' ') || (original[ii] == '+') || (original[ii] == '-') || (original[ii] == '*')) {
                        RLength = ii-i-1;
                        break;
                    }
                }
                for (UINT32 j = 0; j < MAXR; j++) {
                    if (RRegs[j].compare (original.substr (i+1, RLength)) == 0) {
                        RMask &= ~ (0x1 << j);
                        TRegs[TRegPtr] = RRegs[j].c_str ();
                        TRegPtr++;
                        break;
                    }
                }
            }
        }
        TRegPtr = 0;
        int MORegPtr = 0;
        for (int i = 0; i < addrOps; i++) {
            fprintf (out1,"\nA,%llu-%i,", (long long unsigned int)instAddr, subAddr);	
            fprintf (out1,"%s#%d,",MORegs[0].c_str (), 2);
            ss << "\nA," << (long long unsigned int)instAddr << "-" <<  subAddr << ",";
            ss << MORegs[0].c_str () << "#2,";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            aluOp++;
            subAddr++;
        }	
        for (int i = 0; i < MPointers; i++) {
            fprintf (out1,"\nR,%llu,%llu-%i,", (long long unsigned int)addr, (long long unsigned int)instAddr, subAddr);	
            fprintf (out1,"%s#%d,",TRegs[TRegPtr].c_str (), 1);
            fprintf (out1,"%s#%d,",MORegs[TRegPtr].c_str (), 2);
            ss << "\nR," << (long long unsigned int)addr << "," << (long long unsigned int)instAddr << "-" << subAddr << ",";	
            ss << TRegs[TRegPtr].c_str () << "#1,";
            ss << MORegs[TRegPtr].c_str () << "#2,";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            TRegPtr++;
            memOp++;
            subAddr++;
        }
        ss << "\n";
        if (isFDiv) {
            fprintf (out1,"\nD,%llu-%i,", (long long unsigned int)instAddr, subAddr);
            ss << "\nD," << (long long unsigned int)instAddr << "-" << subAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            fpOp++;
        } else if (isFloat) {
            fprintf (out1,"\nF,%llu-%i,", (long long unsigned int)instAddr, subAddr);
            ss << "\nF," << (long long unsigned int)instAddr << "-" << subAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            fpOp++;
        } else {
            fprintf (out1,"\nA,%llu-%i,", (long long unsigned int)instAddr, subAddr);
            ss << "\nA," << (long long unsigned int)instAddr << "-" << subAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
            aluOp++;
        }
        subAddr++;
        for (UINT32 i = 0; i < MAXR; i++) {
            if (RMask & (0x1 << i)) {
                fprintf (out1,"%s#%d,", RRegs[i].c_str (), 1);
                ss << RRegs[i].c_str () << "#1,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            } else {
                fprintf (out1,"%s#%d,", MORegs[MORegPtr].c_str (), 1);
                ss << MORegs[MORegPtr].c_str () << "#1,";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
                MORegPtr++;
            }
        }
    }
    //ss.str (std::string ());
}

//ALU OP
ADDRINT disAsmALU (ADDRINT ip, ADDRINT instAddr, UINT32 regCount, UINT32 isFloat, UINT32 isFDiv, UINT32 isBranch, ADDRINT brTarget, BOOL brTaken)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        if (isFloat) {
            fpOp++;
            if (isFDiv) {
                if (logEnable) fprintf (out, "\n--- D, %d, IP:%ld, ASM: (%s)\n", regCount, ip, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
                fprintf (out1,"\nD,%llu,", (long long unsigned int)instAddr);
                ss << "\nD," << (long long unsigned int)instAddr << ",";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            } else {
                if (logEnable) fprintf (out, "\n--- F, %d, IP:%ld, ASM: (%s)\n", regCount, ip, insMap[ip].c_str ());
                if (logEnable) fprintf (out, "\n>> ");
                fprintf (out1,"\nF,%llu,", (long long unsigned int)instAddr);
                ss << "\nF," << (long long unsigned int)instAddr << ",";
                g_var.g_ins = ss.str ();
                //ss.str (std::string ());
            }
        } else if (isBranch) {
            brOp++;
            if (logEnable) fprintf (out, "\n--- B, %d, IP:%ld, ASM: (%s)\n", regCount, ip, insMap[ip].c_str ());
            if (logEnable) fprintf (out, "\n>> ");
            fprintf (out1,"\nB,%llu,%d,%llu,", (long long unsigned int)instAddr,brTaken, (long long unsigned int)brTarget);
            ss << "\nB," << (long long unsigned int)instAddr << "," << brTaken << "," << (long long unsigned int)brTarget << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
        } else {
            if (logEnable) fprintf (out, "\n--- A, %d, IP:%ld, ASM: (%s)\n", regCount, ip, insMap[ip].c_str ());
            if (logEnable) fprintf (out, "\n>> ");
            fprintf (out1,"\nA,%llu,", (long long unsigned int)instAddr);
            ss << "\nA," << (long long unsigned int)instAddr << ",";
            g_var.g_ins = ss.str ();
            //ss.str (std::string ());
        }
        aluOp++;
    }
    //long int temp = instAddr-target;
    //if (isBranch==1 && temp < 0 && temp > -2000) printf ("%d, %lu, %lu, %lu, %ld\n", brOp, brTarget, brTaken, instAddr, temp);
    ADDRINT value=0;//dummy return
    return value;
}
// Process ALU Operands 
ADDRINT operands (ADDRINT ip, REG reg, int i, int regType)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
        if (logEnable) fprintf (out, "%s#%d, ", REG_StringShort (reg).c_str (), regType);
        fprintf (out1,"%s#%d,",  REG_StringShort (reg).c_str (), regType);
        ss << REG_StringShort (reg).c_str () << "#" << regType << ",";
        g_var.g_ins = ss.str ();
        //ss.str (std::string ());
    }
    ADDRINT value=0;//dummy return
    return value;
}
// Process ALU Operands 
ADDRINT operands1 (ADDRINT ip, REG reg, int i, int regType)
{
    if ( ( (warmUp == 1 || strt == 1) && fin == 0) || runAllProgram == true) {
	if (logEnable) fprintf (out, "%s#%d, ", REG_StringShort (reg).c_str (), regType);
    }
    ADDRINT value=0;//dummy return
    return value;
}

VOID reset () {
    ss.str (std::string ());
}

VOID findInSfile (ADDRINT ip, ADDRINT insAddr) {
    if (!_g_staticCode->isInsIn_insMap (insAddr)) {
        g_var.stat.noMatchIns++;
        //if (g_var.missingInsList.find (insAddr) == g_var.missingInsList.end ())
        //	printf ("-- INS: %ld %lx\n", insAddr, insAddr);
        g_var.stat.missingInsList.insert (insAddr);
    } else {
        g_var.stat.matchIns++;
        g_var.g_ins = _g_staticCode->getIns (insAddr);
        //printf ("%s\n", _g_staticCode->getIns (insAddr, 1, 1, false).c_str ());
    }
}

//=======================================================
// Instrumentation routines
//=======================================================
VOID EmulateLoad (INS ins, VOID* v)
{
    INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) findInSfile,
            IARG_INST_PTR,
            IARG_ADDRINT, INS_Address (ins),
            IARG_END);
    //INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) reset,
    //	IARG_END);
    //pair<map<ADDRINT,string>::iterator,bool> ret;
    ///*MEMORY OPERATIONS*/
    //UINT32 memOperands = INS_MemoryOperandCount (ins);
    //for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    //{
    //    //printf ("%lx\t\t%s\tINS_COUNT=%d\n", INS_Opcode (ins), INS_Disassemble (ins).c_str (),INS_OperandCount (ins));
    //    if (INS_MemoryOperandIsRead (ins, memOp) && INS_Valid (ins))
    //    {
    //		bool split = (INS_Category (ins) == XED_CATEGORY_BINARY) ||
    //		             (INS_Category (ins) == XED_CATEGORY_LOGICAL) ||
    //		             (INS_Category (ins) == XED_CATEGORY_STRINGOP);
    //
    //		ADDRINT insAddr = INS_Address (ins);
    //		string temp = INS_Disassemble (ins);
    //		ret=insMap.insert (pair<ADDRINT,string> (insAddr,temp));
    //
    //		bool isFloatIns;
    //		isFloatIns = isFloat (ins);
    //
    //		bool isFDivIns;
    //		isFDivIns = isFDiv (ins);
    //
    //		UINT32 operandCount = INS_MaxNumRRegs (ins)+INS_MaxNumWRegs (ins); //INS_OperandCount (ins);
    //		
    //		if (split && ENABLE_MICRO) {
    //			INS_InsertPredicatedCall (
    //			    ins, IPOINT_BEFORE, AFUNPTR (dosAsmMEMRS),
    //			    IARG_INST_PTR,
    //				IARG_ADDRINT, insAddr,
    //				IARG_MEMORYOP_EA, memOp,
    //			    IARG_UINT32, operandCount,
    //				IARG_UINT32, isFloatIns,
    //				IARG_UINT32, isFDivIns,
    //				IARG_UINT32, INS_MaxNumRRegs (ins),
    //				IARG_UINT32, INS_MaxNumWRegs (ins),
    //				IARG_UINT32, INS_RegR (ins, 0),
    //				IARG_UINT32, INS_RegR (ins, 1),
    //				IARG_UINT32, INS_RegR (ins, 2),
    //				IARG_UINT32, INS_RegR (ins, 3),
    //				IARG_UINT32, INS_RegR (ins, 4),
    //				IARG_UINT32, INS_RegR (ins, 5),
    //				IARG_UINT32, INS_RegR (ins, 6),
    //				IARG_UINT32, INS_RegR (ins, 7),
    //				IARG_END);
    //
    //			INS_InsertPredicatedCall ( ins, IPOINT_BEFORE, AFUNPTR (dosAsmMEMR1),
    //			    IARG_INST_PTR,
    //				IARG_ADDRINT, insAddr,
    //				IARG_MEMORYOP_EA, memOp,
    //			    IARG_UINT32, operandCount,
    //				IARG_UINT32, isFloatIns,
    //				IARG_UINT32, isFDivIns,
    //				IARG_END);
    //			for (j = 0; j < INS_MaxNumRRegs (ins); j+=1) {
    //			        int regType=1; //READ
    //			        INS_InsertPredicatedCall (ins,
    //			          	IPOINT_BEFORE,
    //			          	AFUNPTR (operands1),
    //			          	IARG_INST_PTR,
    //			          	IARG_UINT32, INS_RegR (ins, j),
    //			          	IARG_UINT32, j,
    //			          	IARG_UINT32, regType,
    //			            IARG_END);
    //				        printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //								       REG_StringShort (INS_RegR (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //			}
    //			for (j = 0; j < INS_MaxNumWRegs (ins); j+=1) {
    //			        int regType=2; //WRITE
    //			        INS_InsertPredicatedCall (ins,
    //			          	IPOINT_BEFORE,
    //			          	AFUNPTR (operands),
    //			          	IARG_INST_PTR,
    //			          	IARG_UINT32, INS_RegW (ins, j),
    //			          	IARG_UINT32, j,
    //			          	IARG_UINT32, regType,
    //			            IARG_END);
    //				        printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //								       REG_StringShort (INS_RegW (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //			}
    //		} else {
    //			INS_InsertPredicatedCall (
    //			    ins, IPOINT_BEFORE, AFUNPTR (dosAsmMEMR),
    //			    IARG_INST_PTR,
    //				IARG_ADDRINT, insAddr,
    //				IARG_MEMORYOP_EA, memOp,
    //			    IARG_UINT32, operandCount,
    //				IARG_UINT32, isFloatIns,
    //				IARG_UINT32, isFDivIns,
    //				IARG_END);
    //			for (j = 0; j < INS_MaxNumRRegs (ins); j+=1) {
    //			    int regType=1; //READ
    //			    INS_InsertPredicatedCall (ins,
    //			      	IPOINT_BEFORE,
    //			      	AFUNPTR (operands),
    //			      	IARG_INST_PTR,
    //			      	IARG_UINT32, INS_RegR (ins, j),
    //			      	IARG_UINT32, j,
    //			      	IARG_UINT32, regType,
    //			        IARG_END);
    //				    printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //					REG_StringShort (INS_RegR (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //			}
    //			for (j = 0; j < INS_MaxNumWRegs (ins); j+=1) {
    //				int regType=2; //WRITE
    //				INS_InsertPredicatedCall (ins,
    //				  	IPOINT_BEFORE,
    //				  	AFUNPTR (operands),
    //				  	IARG_INST_PTR,
    //				  	IARG_UINT32, INS_RegW (ins, j),
    //				  	IARG_UINT32, j,
    //				  	IARG_UINT32, regType,
    //				    IARG_END);
    //					printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //					REG_StringShort (INS_RegW (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //			}
    //		}
    //	}
    //    if (INS_MemoryOperandIsWritten (ins, memOp) && INS_Valid (ins))
    //    {
    //
    //		bool split = (INS_Category (ins) == XED_CATEGORY_BINARY) ||
    //		             (INS_Category (ins) == XED_CATEGORY_LOGICAL) ||
    //		             (INS_Category (ins) == XED_CATEGORY_STRINGOP);
    //
    //		ADDRINT insAddr = INS_Address (ins);
    //		string temp = INS_Disassemble (ins);
    //		ret=insMap.insert (pair<ADDRINT,string> (insAddr,temp));
    //		
    //		bool isFloatIns;
    //		isFloatIns = isFloat (ins);
    //
    //		bool isFDivIns;
    //		isFDivIns = isFDiv (ins);
    //
    //		UINT32 operandCount = INS_MaxNumRRegs (ins)+INS_MaxNumWRegs (ins); //INS_OperandCount (ins);
    //		
    //		if (split && ENABLE_MICRO) {
    //			INS_InsertPredicatedCall (
    //			    ins, IPOINT_BEFORE, AFUNPTR (dosAsmMEMWS),
    //			    IARG_INST_PTR,
    //				IARG_ADDRINT, insAddr,
    //				IARG_MEMORYOP_EA, memOp,
    //			    IARG_UINT32, operandCount,
    //				IARG_UINT32, isFloatIns,
    //				IARG_UINT32, isFDivIns,
    //				IARG_UINT32, INS_MaxNumRRegs (ins),
    //				IARG_UINT32, INS_MaxNumWRegs (ins),
    //				IARG_UINT32, INS_RegR (ins, 0),
    //				IARG_UINT32, INS_RegR (ins, 1),
    //				IARG_UINT32, INS_RegR (ins, 2),
    //				IARG_UINT32, INS_RegR (ins, 3),
    //				IARG_UINT32, INS_RegR (ins, 4),
    //				IARG_UINT32, INS_RegR (ins, 5),
    //				IARG_UINT32, INS_RegR (ins, 6),
    //				IARG_UINT32, INS_RegR (ins, 7),
    //				IARG_UINT32, INS_RegW (ins, 0),
    //				IARG_UINT32, INS_RegW (ins, 1),
    //				IARG_UINT32, INS_RegW (ins, 2),
    //				IARG_UINT32, INS_RegW (ins, 3),
    //				IARG_UINT32, INS_RegW (ins, 4),
    //				IARG_UINT32, INS_RegW (ins, 5),
    //				IARG_UINT32, INS_RegW (ins, 6),
    //				IARG_UINT32, INS_RegW (ins, 7),
    //				IARG_END);
    //			INS_InsertPredicatedCall (
    //			    ins, IPOINT_BEFORE, AFUNPTR (dosAsmMEMW1),
    //			    IARG_INST_PTR,
    //				IARG_ADDRINT, insAddr,
    //				IARG_MEMORYOP_EA, memOp,
    //			    IARG_UINT32, operandCount,
    //				IARG_UINT32, isFloatIns,
    //				IARG_UINT32, isFDivIns,
    //				IARG_END);
    //			for (j = 0; j < INS_MaxNumWRegs (ins); j+=1) {
    //				int regType=2; //WRITE
    //				INS_InsertPredicatedCall (ins,
    //				  	IPOINT_BEFORE,
    //				  	AFUNPTR (operands1),
    //				  	IARG_INST_PTR,
    //				  	IARG_UINT32, INS_RegW (ins, j),
    //				  	IARG_UINT32, j,
    //				  	IARG_UINT32, regType,
    //				    IARG_END);
    //					printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //							REG_StringShort (INS_RegR (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //			}
    //			for (j = 0; j < INS_MaxNumRRegs (ins); j+=1) {
    //			    int regType=1; //READ
    //			    INS_InsertPredicatedCall (ins,
    //			      	IPOINT_BEFORE,
    //			      	AFUNPTR (operands1),
    //			      	IARG_INST_PTR,
    //			      	IARG_UINT32, INS_RegR (ins, j),
    //			      	IARG_UINT32, j,
    //			      	IARG_UINT32, regType,
    //			        IARG_END);
    //				    printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //							REG_StringShort (INS_RegW (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //			}
    //		} else {
    //			INS_InsertPredicatedCall (
    //			    ins, IPOINT_BEFORE, AFUNPTR (dosAsmMEMW),
    //			    IARG_INST_PTR,
    //				IARG_ADDRINT, insAddr,
    //				IARG_MEMORYOP_EA, memOp,
    //			    IARG_UINT32, operandCount,
    //				IARG_UINT32, isFloatIns,
    //			    IARG_UINT32, isFDivIns,
    //				IARG_END);
    //			for (j = 0; j < INS_MaxNumWRegs (ins); j+=1) {
    //				int regType=2; //WRITE
    //				INS_InsertPredicatedCall (ins,
    //				  	IPOINT_BEFORE,
    //				  	AFUNPTR (operands),
    //				  	IARG_INST_PTR,
    //				  	IARG_UINT32, INS_RegW (ins, j),
    //				  	IARG_UINT32, j,
    //				  	IARG_UINT32, regType,
    //				    IARG_END);
    //					printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType,
    //							REG_StringShort (INS_RegW (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, INS_Disassemble (ins).c_str ());
    //			}
    //			for (j = 0; j < INS_MaxNumRRegs (ins); j+=1) {
    //				int regType=1; //READ
    //				INS_InsertPredicatedCall (ins,
    //				  	IPOINT_BEFORE,
    //				  	AFUNPTR (operands),
    //				  	IARG_INST_PTR,
    //				  	IARG_UINT32, INS_RegR (ins, j),
    //				  	IARG_UINT32, j,
    //				  	IARG_UINT32, regType,
    //				    IARG_END);
    //				    printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType,
    //							REG_StringShort (INS_RegR (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, INS_Disassemble (ins).c_str ());
    // Count!
    //INS_InsertPredicatedCall (ins, IPOINT_BEFORE,
    //	 (AFUNPTR)memCount, IARG_END);
    //fprintf (trace1, "\t%lx\t\t%s\n", INS_Opcode (ins), INS_Disassemble (ins));
    //			}
    //		}
    //    }
    //}
    ///*NON-MEMORY OPERATIONS*/
    ////if (memOperands==0 && INS_OperandCount (ins) > 0 && INS_Valid (ins)) {
    //if (memOperands==0 || INS_Category (ins) == XED_CATEGORY_BINARY ||
    //	INS_Category (ins) == XED_CATEGORY_LOGICAL ||
    //	INS_Category (ins) == XED_CATEGORY_STRINGOP) {
    //    ADDRINT insAddr = INS_Address (ins);
    //    string temp = INS_Disassemble (ins);
    //    ret=insMap.insert (pair<ADDRINT,string> (insAddr,temp));
    //    
    //    bool isFloatIns;
    //    isFloatIns = isFloat (ins);
    //    bool isFDivIns;
    //    isFDivIns = isFDiv (ins);
    //    bool isBranchIns;
    //    if (INS_Category (ins) == XED_CATEGORY_COND_BR) {
    //        isBranchIns = true;
    //		//printf ("%lx\t\t%s\tINS_COUNT=%d\n", INS_Opcode (ins), INS_Disassemble (ins).c_str (),INS_OperandCount (ins));
    //    } else {
    //        isBranchIns = false;
    //    }
    //
    //    UINT32 operandCount = INS_MaxNumRRegs (ins)+INS_MaxNumWRegs (ins); //INS_OperandCount (ins);
    //	    INS_InsertPredicatedCall (ins,
    //	      	IPOINT_BEFORE,
    //	      	AFUNPTR (disAsmALU),
    //	      	IARG_INST_PTR,
    //	      	IARG_ADDRINT, insAddr,
    //			IARG_UINT32, operandCount,
    //			IARG_UINT32, isFloatIns,
    //	     	IARG_UINT32, isFDivIns,
    //	     	IARG_UINT32, isBranchIns,
    //			IARG_BRANCH_TARGET_ADDR,
    //			IARG_BRANCH_TAKEN,
    //			IARG_END);
    //    for (j = 0; j < INS_MaxNumRRegs (ins); j+=1) {
    //            int regType=1;//READ
    //            INS_InsertPredicatedCall (ins,
    //              	IPOINT_BEFORE,
    //              	AFUNPTR (operands),
    //              	IARG_INST_PTR,
    //              	IARG_UINT32, INS_RegR (ins, j),
    //              	IARG_UINT32, j,
    //              	IARG_UINT32, regType,
    //                IARG_END);
    //	            printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //						REG_StringShort (INS_RegR (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, INS_Disassemble (ins).c_str ());
    //    }
    //    for (j = 0; j < INS_MaxNumWRegs (ins); j+=1) {
    //		int regType=2;//WRITE
    //		INS_InsertPredicatedCall (ins,
    //		  	IPOINT_BEFORE,
    //		  	AFUNPTR (operands),
    //		  	IARG_INST_PTR,
    //		  	IARG_UINT32, INS_RegW (ins, j),
    //		  	IARG_UINT32, j,
    //		  	IARG_UINT32, regType,
    //		    IARG_END);
    //			printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, 
    //					REG_StringShort (INS_RegW (ins, j)).c_str ()); //INS_Disassemble (ins).c_str ());
    //		//printf ("Regggg (%d)%d: %d - %s\n",j,INS_OperandCount (ins),regType, INS_Disassemble (ins).c_str ());
    //    }
    //}
}

long unsigned imgInsCount=0;
VOID doImpCount ()
{
    imgInsCount++;
    unsigned long countRem = imgInsCount%BILLION;
    if (countRem == 0) {
        cout << " (IMG) :" << imgInsCount << "\n";
    }
}

long unsigned imgInsCallCount=0;
VOID doImpCallCount (BOOL isCall)
{
    if (isCall)
        imgInsCallCount++;
    unsigned long countRem = imgInsCount%BILLION;
    if (countRem == 0) {
        cout << " (CALL) :" << imgInsCallCount << "\n";
    }
}

long unsigned imgInsMemCount=0;
VOID doImpMemCount (UINT32 isMem)
{
    if (isMem > 0)
        imgInsMemCount++;
    unsigned long countRem = imgInsCount%BILLION;
    if (countRem == 0) {
        cout << " (MEM) :" << imgInsMemCount << "\n";
    }
}

VOID instr (INS ins, VOID* v) {
    INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) doImpCount,
            IARG_END);
    INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) doImpMemCount,
            IARG_UINT32, INS_MemoryOperandCount (ins),
            IARG_END);
    INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) doImpCallCount,
            IARG_BOOL, INS_IsCall (ins),
            IARG_END);
}

VOID SI_Img (IMG img, VOID * v) {
    for (SEC sec = IMG_SecHead (img); SEC_Valid (sec); sec = SEC_Next (sec)){
        for (RTN rtn = SEC_RtnHead (sec); RTN_Valid (rtn); rtn = RTN_Next (rtn)){

            RTN_Open (rtn);
            for (INS ins = RTN_InsHead (rtn); INS_Valid (ins); ins = INS_Next (ins))
            {
                EmulateLoad (ins, v);
                //instr (ins,v);
            }
            RTN_Close (rtn);
        }
    }
}

/*
VOID Fini (INT32 code, VOID *v)
{
    fprintf (out1,"\n\nnumber of mem ops = %d\n", memOp);
    fprintf (out1,"number of int ops = %d\n", aluOp); 
    fprintf (out1,"number of fp ops  = %d\n", fpOp);
    fprintf (out1,"number of br ops  = %d\n", brOp);
    ss << "\n\nnumber of mem ops = %d\n", memOp);
    ss << "number of int ops = %d\n", aluOp); 
    ss << "number of fp ops  = %d\n", fpOp);
    ss << "number of br ops  = %d\n", brOp);
    printf ("\n\nnumber of mem ops = %d\n", memOp);
    printf ("number of int ops = %d\n", aluOp);
    printf ("number of fp ops  = %d\n", fpOp);
    printf ("number of br ops  = %d\n", brOp);
    //cout << "number of memory load operations =" << counter << "out of " << countAll << endl;
}
*/

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

/*
   INT32 Usage ()
   {
//cerr << "This tool demonstrates the use of SafeCopy" << endl;
//cerr << endl << KNOB_BASE::StringKnobSummary () << endl;
return -1;
}
*/

/* ===================================================================== */
/* Floating point div identifier                                             */
/* ===================================================================== */

bool isFDiv (INS ins)
{
    bool isFDiv;
    isFDiv = false;

    OPCODE oc = INS_Opcode (ins);

    if (oc == XED_ICLASS_DIVPD) isFDiv = true;
    if (oc == XED_ICLASS_DIVPS) isFDiv = true;
    if (oc == XED_ICLASS_DIVSD) isFDiv = true;
    if (oc == XED_ICLASS_DIVSS) isFDiv = true;
    if (oc == XED_ICLASS_FDIV) isFDiv = true; //Hopfeully not buggy? HA
    if (oc == XED_ICLASS_FDIVP) isFDiv = true;
    if (oc == XED_ICLASS_FDIVR) isFDiv = true;
    if (oc == XED_ICLASS_FDIVRP) isFDiv = true;
    if (oc == XED_ICLASS_FIDIV) isFDiv = true;
    if (oc == XED_ICLASS_FIDIVR) isFDiv = true; //I think this is dividing ints..
    if (oc == XED_ICLASS_FDIV) isFDiv = true; //Hopfeully not buggy? HA
    if (oc == XED_ICLASS_FDIVP) isFDiv = true;
    if (oc == XED_ICLASS_FDIVR) isFDiv = true;
    if (oc == XED_ICLASS_FDIVRP) isFDiv = true;
    if (oc == XED_ICLASS_FIDIV) isFDiv = true;
    if (oc == XED_ICLASS_FIDIVR) isFDiv = true; //I think this is dividing ints..

    return isFDiv;
}

/* ===================================================================== */
/* Floating point identifier                                             */
/* ===================================================================== */

bool isFloat (INS ins)
{
    bool isFloat;
    isFloat = false;

    OPCODE oc = INS_Opcode (ins); 

    if (oc == XED_ICLASS_MOVSS) isFloat = true;
    if (oc == XED_ICLASS_MOVAPS) isFloat = true;

    if (oc == XED_ICLASS_F2XM1) isFloat = true; //2^x - 1
    if (oc == XED_ICLASS_FABS) isFloat = true;
    if (oc == XED_ICLASS_FADD) isFloat = true;
    if (oc == XED_ICLASS_FADDP) isFloat = true;
    if (oc == XED_ICLASS_FCMOVB) isFloat = true; //conditional moves...
    if (oc == XED_ICLASS_FCMOVBE) isFloat = true;
    if (oc == XED_ICLASS_FCMOVE) isFloat = true;
    if (oc == XED_ICLASS_FCMOVNB) isFloat = true;
    if (oc == XED_ICLASS_FCMOVNBE) isFloat = true;
    if (oc == XED_ICLASS_FCMOVNE) isFloat = true;
    if (oc == XED_ICLASS_FCMOVNU) isFloat = true;
    if (oc == XED_ICLASS_FCMOVU) isFloat = true;
    if (oc == XED_ICLASS_FCOM) isFloat = true; //fpcompares...not counting
    if (oc == XED_ICLASS_FCOMI) isFloat = true;
    if (oc == XED_ICLASS_FCOMIP) isFloat = true;
    if (oc == XED_ICLASS_FCOMP) isFloat = true;
    if (oc == XED_ICLASS_FCOMPP) isFloat = true;
    if (oc == XED_ICLASS_FDIV) isFloat = true; //Hopfeully not buggy? HA
    if (oc == XED_ICLASS_FDIVP) isFloat = true;
    if (oc == XED_ICLASS_FDIVR) isFloat = true;
    if (oc == XED_ICLASS_FDIVRP) isFloat = true;
    if (oc == XED_ICLASS_FFREE) isFloat = true;
    if (oc == XED_ICLASS_FFREEP) isFloat = true;
    if (oc == XED_ICLASS_FIADD) isFloat = true;
    if (oc == XED_ICLASS_FICOM) isFloat = true;
    if (oc == XED_ICLASS_FICOMP) isFloat = true;
    if (oc == XED_ICLASS_FIDIV) isFloat = true;
    if (oc == XED_ICLASS_FIDIVR) isFloat = true; //I think this is dividing ints..
    if (oc == XED_ICLASS_FIMUL) isFloat = true;
    if (oc == XED_ICLASS_FISUB) isFloat = true;
    if (oc == XED_ICLASS_FISUBR) isFloat = true;
    if (oc == XED_ICLASS_FMUL) isFloat = true;
    if (oc == XED_ICLASS_FMULP) isFloat = true;
    if (oc == XED_ICLASS_FNINIT) isFloat = true;
    if (oc == XED_ICLASS_FNSAVE) isFloat = true; //These seem like adds...
    if (oc == XED_ICLASS_FNSTCW) isFloat = true;
    if (oc == XED_ICLASS_FNSTENV) isFloat = true;
    if (oc == XED_ICLASS_FNSTSW) isFloat = true;
    if (oc == XED_ICLASS_FPREM) isFloat = true; //Partial remainder..
    if (oc == XED_ICLASS_FPREM1) isFloat = true;
    if (oc == XED_ICLASS_FRNDINT) isFloat = true;  //round to integer
    if (oc == XED_ICLASS_FSCALE) isFloat = true; //some sort of rounding operation  
    if (oc == XED_ICLASS_FSQRT) isFloat = true; //
    if (oc == XED_ICLASS_FST) isFloat = true; //floating point store
    if (oc == XED_ICLASS_FSTP) isFloat = true; //floating point store & pop
    if (oc == XED_ICLASS_FSUB) isFloat = true;  //various types of subtracts
    if (oc == XED_ICLASS_FSUBP) isFloat = true;
    if (oc == XED_ICLASS_FSUBR) isFloat = true; //reverse subtract
    if (oc == XED_ICLASS_FSUBRP) isFloat = true;
    if (oc == XED_ICLASS_FTST) isFloat = true; //compare (test) 
    if (oc == XED_ICLASS_FUCOM) isFloat = true; //unordered compare floating point values
    if (oc == XED_ICLASS_FUCOMI) isFloat = true;
    if (oc == XED_ICLASS_FUCOMIP) isFloat = true;
    if (oc == XED_ICLASS_FUCOMP) isFloat = true;
    if (oc == XED_ICLASS_FUCOMPP) isFloat = true;
    if (oc == XED_ICLASS_FXSAVE) isFloat = true;
    if (oc == XED_ICLASS_FXTRACT) isFloat = true; //split exp and mant
    if (oc == XED_ICLASS_FYL2X) isFloat = true; //y*log_2 (x)
    if (oc == XED_ICLASS_FYL2XP1) isFloat = true; //y*log_2 (x+1)
    if (oc == XED_ICLASS_PFACC) isFloat = true;
    if (oc == XED_ICLASS_PFADD) isFloat = true;
    if (oc == XED_ICLASS_PFCMPEQ) isFloat = true;
    if (oc == XED_ICLASS_PFCMPGE) isFloat = true;
    if (oc == XED_ICLASS_PFCMPGT) isFloat = true;
    if (oc == XED_ICLASS_PFCPIT1) isFloat = true;
    if (oc == XED_ICLASS_PFMAX) isFloat = true;
    if (oc == XED_ICLASS_PFMIN) isFloat = true;
    if (oc == XED_ICLASS_PFMUL) isFloat = true;
    if (oc == XED_ICLASS_PFNACC) isFloat = true;
    if (oc == XED_ICLASS_PFPNACC) isFloat = true;
    if (oc == XED_ICLASS_PFRCP) isFloat = true;
    if (oc == XED_ICLASS_PFRCPIT2) isFloat = true;
    if (oc == XED_ICLASS_PFRSQIT1) isFloat = true;
    if (oc == XED_ICLASS_PFSQRT) isFloat = true;
    if (oc == XED_ICLASS_PFSUB) isFloat = true;
    if (oc == XED_ICLASS_PFSUBR) isFloat = true;
    if (oc == XED_ICLASS_PI2FD) isFloat = true;
    if (oc == XED_ICLASS_PI2FW) isFloat = true;
    if (oc == XED_ICLASS_PINSRB) isFloat = true;
    if (oc == XED_ICLASS_PINSRD) isFloat = true;
    if (oc == XED_ICLASS_PINSRQ) isFloat = true;
    if (oc == XED_ICLASS_PINSRW) isFloat = true;
    if (oc == XED_ICLASS_SHLD) isFloat = true;//double precision shift left
    if (oc == XED_ICLASS_SHRD) isFloat = true;
    if (oc == XED_ICLASS_ADDPD) isFloat = true;
    if (oc == XED_ICLASS_ADDPS) isFloat = true;
    if (oc == XED_ICLASS_ADDSD) isFloat = true;
    if (oc == XED_ICLASS_ADDSS) isFloat = true;
    if (oc == XED_ICLASS_ADDSUBPD) isFloat = true;
    if (oc == XED_ICLASS_ADDSUBPS) isFloat = true;
    if (oc == XED_ICLASS_DIVPD) isFloat = true;
    if (oc == XED_ICLASS_DIVPS) isFloat = true;
    if (oc == XED_ICLASS_DIVSD) isFloat = true;
    if (oc == XED_ICLASS_DIVSS) isFloat = true;
    if (oc == XED_ICLASS_DPPD) isFloat = true;
    if (oc == XED_ICLASS_DPPS) isFloat = true;
    if (oc == XED_ICLASS_F2XM1) isFloat = true; //2^x - 1
    if (oc == XED_ICLASS_FABS) isFloat = true;
    if (oc == XED_ICLASS_FADD) isFloat = true;
    if (oc == XED_ICLASS_FADDP) isFloat = true;
    if (oc == XED_ICLASS_FCOS) isFloat = true; //I feel bad only counting for 1
    if (oc == XED_ICLASS_FDIV) isFloat = true; //Hopfeully not buggy? HA
    if (oc == XED_ICLASS_FDIVP) isFloat = true;
    if (oc == XED_ICLASS_FDIVR) isFloat = true;
    if (oc == XED_ICLASS_FDIVRP) isFloat = true;
    if (oc == XED_ICLASS_FIDIV) isFloat = true;
    if (oc == XED_ICLASS_FIDIVR) isFloat = true; //I think this is dividing ints..
    if (oc == XED_ICLASS_FIMUL) isFloat = true;
    if (oc == XED_ICLASS_FISUB) isFloat = true;
    if (oc == XED_ICLASS_FISUBR) isFloat = true;
    if (oc == XED_ICLASS_FMUL) isFloat = true;
    if (oc == XED_ICLASS_FMULP) isFloat = true;
    if (oc == XED_ICLASS_FPATAN) isFloat = true;  //Compute the arctan and divide
    if (oc == XED_ICLASS_FPREM) isFloat = true; //Partial remainder..
    if (oc == XED_ICLASS_FPREM1) isFloat = true;
    if (oc == XED_ICLASS_FPTAN) isFloat = true;
    if (oc == XED_ICLASS_FRNDINT) isFloat = true;  //round to integer
    if (oc == XED_ICLASS_FSCALE) isFloat = true; //some sort of rounding operation  
    if (oc == XED_ICLASS_FSIN) isFloat = true; //sin
    if (oc == XED_ICLASS_FSINCOS) isFloat = true; //compute both the sin and cos 
    if (oc == XED_ICLASS_FSQRT) isFloat = true; //
    if (oc == XED_ICLASS_FSUB) isFloat = true;  //various types of subtracts
    if (oc == XED_ICLASS_FSUBP) isFloat = true;
    if (oc == XED_ICLASS_FSUBR) isFloat = true; //reverse subtract
    if (oc == XED_ICLASS_FSUBRP) isFloat = true;
    if (oc == XED_ICLASS_FYL2X) isFloat = true; //y*log_2 (x)
    if (oc == XED_ICLASS_FYL2XP1) isFloat = true; //y*log_2 (x+1)
    if (oc == XED_ICLASS_HADDPD) isFloat = true; 
    if (oc == XED_ICLASS_HADDPS) isFloat = true;
    if (oc == XED_ICLASS_HSUBPD) isFloat = true;
    if (oc == XED_ICLASS_HSUBPS) isFloat = true; 
    if (oc == XED_ICLASS_MULPD) isFloat = true;
    if (oc == XED_ICLASS_MULPS) isFloat = true;
    if (oc == XED_ICLASS_MULSD) isFloat = true;
    if (oc == XED_ICLASS_MULSS) isFloat = true;
    if (oc == XED_ICLASS_RCPPS) isFloat = true; //reciprocal
    if (oc == XED_ICLASS_RCPSS) isFloat = true;
    if (oc == XED_ICLASS_ROUNDPD) isFloat = true;  //packed round to ints
    if (oc == XED_ICLASS_ROUNDPS) isFloat = true;
    if (oc == XED_ICLASS_ROUNDSD) isFloat = true;
    if (oc == XED_ICLASS_ROUNDSS) isFloat = true;
    if (oc == XED_ICLASS_RSQRTPS) isFloat = true; //recip square root
    if (oc == XED_ICLASS_RSQRTSS) isFloat = true;
    if (oc == XED_ICLASS_SQRTPD) isFloat = true; //float square roots
    if (oc == XED_ICLASS_SQRTPS) isFloat = true;
    if (oc == XED_ICLASS_SQRTSD) isFloat = true;
    if (oc == XED_ICLASS_SQRTSS) isFloat = true;
    if (oc == XED_ICLASS_SUBPD) isFloat = true;
    if (oc == XED_ICLASS_SUBPS) isFloat = true;
    if (oc == XED_ICLASS_SUBSD) isFloat = true; 
    if (oc == XED_ICLASS_SUBSS) isFloat = true; //substract

    return isFloat;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

void uop_gen (FILE* _outFile, staticCodeParser &g_staticCode)
{
    // Write to a file since cout and cerr maybe closed by the application
    memOp = 0;
    aluOp = 0;
    fpOp = 0;
    //if (logEnable) out = fopen ("trace_nbody.out", "w");
    out1 = _outFile;
    _g_staticCode = &g_staticCode;

    // Initialize pin & symbol manager
    // Register EmulateLoad to be called to instrument instructions
    //IMG_AddInstrumentFunction (SI_Img, 0);
    //PIN_AddFiniFunction (Fini, 0);
}


VOID getBrIns (ADDRINT insAddr, BOOL hasFT, ADDRINT tgAddr, ADDRINT ftAddr, BOOL isTaken, BOOL isCall, BOOL isRet, BOOL isJump, BOOL isDirBrOrCallOrJmp) {
//    if (!_g_staticCode->isInsIn_insMap (insAddr)) {
//        g_var.stat.noMatchIns++;
//        //if (g_var.missingInsList.find (insAddr) == g_var.missingInsList.end ())
//        //	printf ("-- INS: %ld %lx\n", insAddr, insAddr);
//        g_var.stat.missingInsList.insert (insAddr);
//    } else {
        g_var.stat.matchIns++;
        string s;
        if (_g_staticCode->hasIns (insAddr)) {
            if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW BR: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
            if (g_var.g_core_type == BASICBLOCK) {
                dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
                bbInstruction* g_insObj = g_var.getNewIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                g_insObj->setBrAtr (tgAddr, ftAddr, hasFT, isTaken, isCall, isRet, isJump, isDirBrOrCallOrJmp);
                g_insObj->setInsType (BR);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
                if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
                if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
                g_insObj->setBB (g_bbObj);
            } else {
                dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                g_insObj->setBrAtr (tgAddr, ftAddr, hasFT, isTaken, isCall, isRet, isJump, isDirBrOrCallOrJmp);
                g_insObj->setInsType (BR);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
            }
            g_var.g_ins = s;
        }
        //printf ("%s\n", _g_staticCode->getIns (insAddr, 1, 1, false).c_str ());
//    }
}

VOID getMemIns (ADDRINT insAddr, ADDRINT memAccessSize, ADDRINT memAddr, BOOL isStackRd, BOOL isStackWr, BOOL isMemRead) {
//    if (!_g_staticCode->isInsIn_insMap (insAddr)) {
//        g_var.stat.noMatchIns++;
//        //if (g_var.missingInsList.find (insAddr) == g_var.missingInsList.end ())
//        //	printf ("-- INS: %ld %lx\n", insAddr, insAddr);
//        g_var.stat.missingInsList.insert (insAddr);
//    } else {
        g_var.stat.matchIns++;
        string s;
        if (_g_staticCode->hasIns (insAddr)) {
            if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW MEM: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
            if (g_var.g_core_type == BASICBLOCK) {
                dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
                bbInstruction* g_insObj = g_var.getNewIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                MEM_TYPE mType = (isMemRead == true ? LOAD : STORE);
                g_insObj->setMemAtr (mType, memAddr, memAccessSize, isStackRd, isStackWr);
                g_insObj->setInsType (MEM);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
                if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
                if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
                g_insObj->setBB (g_bbObj);
            } else {
                dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                MEM_TYPE mType = (isMemRead == true ? LOAD : STORE);
                g_insObj->setMemAtr (mType, memAddr, memAccessSize, isStackRd, isStackWr);
                g_insObj->setInsType (MEM);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
            }
            g_var.g_ins = s;
        }
        //printf ("%s\n", _g_staticCode->getIns (insAddr, 1, 1, false).c_str ());
//    }
}

VOID getIns (ADDRINT insAddr) {
//    if (!_g_staticCode->isInsIn_insMap (insAddr)) {
//        g_var.stat.noMatchIns++;
//        //if (g_var.missingInsList.find (insAddr) == g_var.missingInsList.end ())
//        //	printf ("-- INS: %ld %lx\n", insAddr, insAddr);
//        g_var.stat.missingInsList.insert (insAddr);
//    } else {
        g_var.stat.matchIns++;
        string s;
        if (_g_staticCode->hasIns (insAddr)) {
            if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW INS: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
            if (g_var.g_core_type == BASICBLOCK) {
                dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
                if (g_bbObj == NULL) return;
                bbInstruction* g_insObj = g_var.getNewIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                g_insObj->setInsType (ALU);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
                if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
                if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
                g_insObj->setBB (g_bbObj);
            } else {
                dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                g_insObj->setInsType (ALU);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
            }
            g_var.g_ins = s;
        }
        //printf ("%s\n", _g_staticCode->getIns (insAddr, 1, 1, false).c_str ());
//    }
}

VOID getNopIns (ADDRINT insAddr) {
//    if (!_g_staticCode->isInsIn_insMap (insAddr)) {
//        g_var.stat.noMatchIns++;
//        //if (g_var.missingInsList.find (insAddr) == g_var.missingInsList.end ())
//        //	printf ("-- INS: %ld %lx\n", insAddr, insAddr);
//        g_var.stat.missingInsList.insert (insAddr);
//    } else {
        g_var.stat.matchIns++;
        string s;
        if (_g_staticCode->hasIns (insAddr)) {
            if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW NOP: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
            if (g_var.g_core_type == BASICBLOCK) {
                dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
                bbInstruction* g_insObj = g_var.getNewIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                g_insObj->setInsType (NOP);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
                if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
                if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
                g_insObj->setBB (g_bbObj);
            } else {
                dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
                stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
                staticIns->copyRegsTo (g_insObj);
                g_insObj->setInsType (NOP);
                g_insObj->setInsAddr (insAddr);
                g_insObj->setInsID (g_var.g_seq_num++);
                g_insObj->setWrongPath (g_var.g_wrong_path);
            }
            g_var.g_ins = s;
        }
        //printf ("%s\n", _g_staticCode->getIns (insAddr, 1, 1, false).c_str ());
//    }
}

void getBBhead (ADDRINT bb_tail_ins_addr, BOOL is_tail_br) {
    if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW BB: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_bb_seq_num << std::endl;
    dynBasicblock* g_bbObj = g_var.getNewCacheBB ();
    g_bbObj->setBBID (g_var.g_bb_seq_num++);
    g_bbObj->setBBbrAddr (is_tail_br, bb_tail_ins_addr);
}

void get_bb_header (INS bb_tail_ins) {
    if (INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins)) {
        if (INS_HasFallThrough (bb_tail_ins)) {
            INS_InsertCall (bb_tail_ins, IPOINT_AFTER, (AFUNPTR) getBBhead,
                    IARG_ADDRINT, INS_Address (bb_tail_ins),
                    IARG_BOOL, INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins),
                    IARG_END);
        }
        INS_InsertCall (bb_tail_ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) getBBhead,
                IARG_ADDRINT, INS_Address (bb_tail_ins),
                IARG_BOOL, INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins),
                IARG_END);
    } else {
        INS_InsertCall (bb_tail_ins, IPOINT_BEFORE, (AFUNPTR) getBBhead,
                IARG_ADDRINT, INS_Address (bb_tail_ins),
                IARG_BOOL, INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins),
                IARG_END);
    }
}

void get_uop (INS ins) {
//    if (INS_IsBranchOrCall (ins) || INS_IsFarRet (ins) || INS_IsRet (ins)) { //TODO put is back
    if (INS_IsBranchOrCall (ins)) {
        if (INS_HasFallThrough (ins)) {
            INS_InsertCall (ins, IPOINT_AFTER, (AFUNPTR) getBrIns,
                    IARG_ADDRINT, INS_Address (ins),
                    IARG_BOOL, INS_HasFallThrough (ins),
                    IARG_BRANCH_TARGET_ADDR, 
                    IARG_FALLTHROUGH_ADDR,
                    IARG_BRANCH_TAKEN,
                    IARG_BOOL, INS_IsCall (ins) || INS_IsFarCall (ins),
                    IARG_BOOL, INS_IsRet (ins) || INS_IsFarRet (ins),
                    IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsFarJump (ins) || (INS_IsBranch (ins) && INS_HasFallThrough (ins)),
                    IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsDirectBranchOrCall (ins),
                    IARG_END);
        }
        INS_InsertCall (ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) getBrIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_BOOL, INS_HasFallThrough (ins),
                IARG_BRANCH_TARGET_ADDR, 
                IARG_FALLTHROUGH_ADDR,
                IARG_BRANCH_TAKEN,
                IARG_BOOL, INS_IsCall (ins) || INS_IsFarCall (ins),
                IARG_BOOL, INS_IsRet (ins) || INS_IsFarRet (ins),
                IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsFarJump (ins) || (INS_IsBranch (ins) && INS_HasFallThrough (ins)),
                IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsDirectBranchOrCall (ins),
                IARG_END);
        /*
        //capture mem u-op
        if (INS_IsMemoryWrite (ins) || INS_IsMemoryRead (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
        IARG_ADDRINT, INS_Address (ins),
        IARG_MEMORYWRITE_SIZE,
        IARG_MEMORYWRITE_EA,
        IARG_END);
        }
        */
    } else if (INS_IsMemoryWrite (ins)) {
        BOOL isMemRead;
        isMemRead = false;
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_MEMORYWRITE_SIZE,
                IARG_MEMORYWRITE_EA,
                IARG_BOOL, INS_IsStackRead (ins),
                IARG_BOOL, INS_IsStackWrite (ins),
                IARG_BOOL, isMemRead,
                IARG_END);
    } else if (INS_IsMemoryRead (ins)) {
        bool isMemRead;
        isMemRead = true;
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_MEMORYREAD_SIZE,
                IARG_MEMORYREAD_EA,
                IARG_BOOL, INS_IsStackRead (ins),
                IARG_BOOL, INS_IsStackWrite (ins),
                IARG_BOOL, isMemRead,
                IARG_END);
    } else if (INS_IsNop (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getNopIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_END);
    } else {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_END);
        /*
        //capture mem u-op
        if (INS_IsMemoryWrite (ins) || INS_IsMemoryRead (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
        IARG_ADDRINT, INS_Address (ins),
        IARG_MEMORYWRITE_SIZE,
        IARG_MEMORYWRITE_EA,
        IARG_END);
        */
    }
}


    /****************************************
     * GARBAGE CODE
     ***************************************/




    //cout << "register 0 = " << REG_StringShort (INS_OperandReg (ins, 0)) << endl;
    //if (INS_OperandCount (ins) > 0)
    //    cout << "register 0 = " << REG_StringShort (INS_OperandReg (ins, 0)) << endl;
    //INS_InsertPredicatedCall (
    //    ins, IPOINT_BEFORE, (AFUNPTR)printRegs,
    //    IARG_INST_PTR,
    //    IARG_MEMORYOP_EA, memOp,
    //    IARG_END);

    //if (milad == REG_INVALID ())
    // Find the instructions that move a value from memory to a register
    //if (INS_Opcode (ins) == XED_ICLASS_MOV &&
    //    INS_IsMemoryRead (ins) 
    //    && INS_OperandIsReg (ins, 0) 
    //    && INS_OperandIsMemory (ins, 1))
    //{
    //    cout << "register 0 = " << REG_StringShort (REG (INS_OperandReg (ins, 0))) << endl;
    //    // op0 <- *op1



    //else if (INS_Opcode (ins) == XED_ICLASS_MOV &&
    //      (INS_IsMemoryWrite (ins) && 
    //      INS_OperandIsReg (ins, 1) && 
    //      INS_OperandIsMemory (ins, 0)))
    //{
    //    counter++;
    //    // op0 <- *op1
    //    INS_InsertPredicatedCall (ins,
    //                   IPOINT_BEFORE,
    //                   AFUNPTR (DoStore),
    //                   IARG_UINT32,
    //                   REG (INS_OperandReg (ins, 0)),
    //                   IARG_MEMORYWRITE_EA,
    //                   IARG_RETURN_REGS,
    //                   INS_OperandReg (ins, 0),
    //                   IARG_END);
    //    //INS_Delete (ins);
    //}


    // Delete the instruction





    //if (i == 0)
    //if (reg != REG_INVALID ()) 
    //*out << "\nA," << regCount << "," << hex << ip << "," << insMap[ip].c_str () << "," << REG_StringShort (reg) << "#" << regType << ", ";
    //*out << "\nA," << regCount << "," << hex << ip << "," << insMap[ip].c_str () << "," << "#" << regType << ", ";
    //else	*out << "\nA," << " " << ",";
    //else
    //if (reg != REG_INVALID ()) 
    //*out << REG_StringShort (reg) << "#" << regType << ", ";
    //*out << "#" << regType << ", ";
    //else	*out << " " << ",";

    /*
    // Process Memory Reads
    VOID RecordMemRead (ADDRINT ip, VOID * addr, REG reg)
    {
    //if (reg != REG_INVALID ())
     *out << "\nR," << hex << ip << "," << insMap[ip].c_str () << ",\t\t\t" << REG_StringShort (reg) << "," << addr;
    //else
    //	*out << "\nR," << hex << ip << "," << insMap[ip].c_str () << " " << "," << addr;
    //fprintf (trace,"%p: R\n", ip);
    //fprintf (trace1, "%p\t%s\n", ip, insMap[ (ADDRINT)ip]);
    }

    // Process Memory Writes
    VOID RecordMemWrite (ADDRINT ip, VOID * addr, REG reg)
    {
    //if (reg != REG_INVALID ())
     *out << "\nW," << hex << ip << "," << insMap[ip].c_str () << ",\t\t\t" << REG_StringShort (reg) << "," << addr;
    //else
    //	*out << "\nW," << hex << ip << "," << insMap[ip].c_str () << " " << "," << addr;
    //INS instr = insMap[ip];
    //fprintf (trace1, "%p\t%s\n", ip, insMap[ (ADDRINT)ip]);
    }




    IARG_END);
    }
    *//* 
         for (int i = 0; i < INS_OperandCount (ins); i++) {
         INS_InsertPredicatedCall (ins,
         IPOINT_BEFORE,
         AFUNPTR (DoIns),
         IARG_UINT32,
         REG (INS_OperandReg (ins, i)),
         IARG_UINT32,
         i,
         IARG_END);

         if (INS_RegR (ins,i)) {
         printf ("r%d > READ\n",i);
         } else if (INS_RegW (ins,i)) {
         printf ("r%d > WRITE\n",i);
         } else if (INS_OperandIsImmediate (ins,i)) {
         printf ("r%d > CONSTANT\n",i);
         INS_Delete (ins);
         break;
         } else if (INS_OperandWrittenOnly (ins,i)) {
         printf ("r%d > WRITE-ONLY\n",i);
         INS_Delete (ins);
         break;
         } else if (INS_OperandReadOnly (ins,i)) {
         printf ("r%d > READ-ONLY\n",i);
         INS_Delete (ins);
         break;
         } else if (INS_OperandReadAndWritten (ins,i)) {
         printf ("r%d > R/W\n",i);
         INS_Delete (ins);
         break;
         } else { 
         printf ("nothing?\n");
         if (INS_IsMemoryWrite (ins))
         printf ("r%d > MEM-R\n",i);
         if (INS_IsMemoryWrite (ins))
         printf ("r%d > MEM-W\n",i);
         INS_Delete (ins);
         break; //seg
         }
         */
//}
//}
//}

	/*   bool flag;
	   if (INS_Valid (ins) &&
	       INS_OperandCount (ins) <= 3 && 
	      ( (INS_RegR (ins,0) || INS_RegW (ins,0)) &&
	       (INS_RegR (ins,1) || INS_RegW (ins,1)) &&
	       (INS_RegR (ins,1) || INS_RegW (ins,1)))) {
	   for (int i=0; i < INS_OperandCount (ins); i++) {
	   printf ("%lx\t\t%s\t*INS_COUNT=%d\n", INS_Opcode (ins), INS_Disassemble (ins).c_str (),INS_OperandCount (ins));
	   printf ("%lx\t\t%s\t INS_COUNT=%d\n", INS_Opcode (ins), insMap[insAddr].c_str ()); //,INS_OperandCount (ins));
	      INS_InsertPredicatedCall (ins,
	        	IPOINT_BEFORE,
	        	AFUNPTR (DoIns),
			IARG_INST_PTR,
	        	IARG_UINT32, REG (INS_OperandReg (ins, i)),
	        	IARG_UINT32, i,

*/
