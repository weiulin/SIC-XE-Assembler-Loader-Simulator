#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cMIN -1
#define cLOAD 0
#define cSHOW 1
#define cUNLOAD 2
#define cEXIT 3
#define cRUN 4
#define cMAX 5

#define oMIN -1
#define oADD 0
#define oAND 1
#define oCOMP 2
#define oDIV 3
#define oJ 4
#define oJEQ 5
#define oJGT 6
#define oJLT 7
#define oJSUB 8
#define oLDA 9
#define oLDCH 10
#define oLDL 11
#define oLDX 12
#define oMUL 13
#define oOR 14
#define oRD 15
#define oRSUB 16
#define oSTA 17
#define oSTCH 18
#define oSTL 19
#define oSTSW 20
#define oSTX 21
#define oSUB 22
#define oTD 23
#define oTIX 24
#define oWD 25
#define oMAX 26

FILE *f;
char fname[20];
char tname[20];
char c_line[30];
char o_line[80];
char cmd[8];
int prog_len = 0;
int start_add = 0;
int first_add = 0;
int curr_add = 0;
char *memory = NULL;
int mem_size = 0;
int loaded = 0;
int op = 0;
int indexed = 0;
int operand = 0;
int running = 0;

int reg_A, reg_X, reg_L, reg_PC, reg_SW;

const char s_command[5][7] = {"load", "show", "unload", "exit", "run"};
const int MAXADD = 0x7FFF;
const int IKEY = 0x8000;

const char optab[26][3] = {"18", "40", "28", "24", "3C", "30", "34", "38",
                           "48", "00", "50", "08", "04", "20", "44", "D8", "4C",
                           "0C", "54", "14", "E8", "10", "1C", "E0", "2C", "DC"};

int lookup(char *s)
{
    int i;
    for (i = 0; i < oMAX; i++)
        if (strcmp(optab[i], s) == 0)
            return i;
    return -1;
}

int readline()
{
    int i = 0;
    int tmp = cMIN;

    printf("SIC Simulator> ");
    do
    {
        fgets(c_line, 30, stdin);
        i = strlen(c_line);
        c_line[i - 1] = '\0';
        // printf("Line = [%s], i = [%d]\n", c_line, i);
    } while (i <= 1);

    sscanf(c_line, "%s", cmd);
    // printf("Command = [%s]\n", cmd );
    // 修改---------------------------------------------------------------------------------------------------
    if (strcmp(cmd, s_command[0]) == 0)
    {
        tmp = cLOAD;
        sscanf(c_line, "%*s %s", fname);
        // printf("File name is %s.\n", fname);
    }
    else if (strcmp(cmd, s_command[1]) == 0)
        tmp = cSHOW;
    else if (strcmp(cmd, s_command[2]) == 0)
        tmp = cUNLOAD;
    else if (strcmp(cmd, s_command[3]) == 0)
        tmp = cEXIT;
    else if (strcmp(cmd, s_command[4]) == 0)
        tmp = cRUN;
    // printf("Command number is %d.\n", tmp);

    c_line[0] = '\0';
    cmd[0] = '\0';
    return tmp;
}

void rd_header()
{
    char tmp[7];
    int i, j, s;

    for (i = 7, j = 0; i < 13; i++, j++)
        tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &start_add);

    for (i = 13, j = 0; i < 19; i++, j++)
        tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &prog_len);

    s = prog_len * 2 + 1;
    memory = (char *)malloc(sizeof(char) * s);
    if (memory)
    {
        for (i = 0; i < s; i++)
            memory[i] = 'X';
        memory[s - 1] = '\0';
        loaded = 1;
        mem_size = s;
    }
    else
    {
        printf("Loading Failed! (Memory allocation error)\n");
    }
}

void rd_text()
{
    char tmp[7];
    int i, j, l, s;

    for (i = 1, j = 0; i < 7; i++, j++)
        tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &s);

    for (i = 7, j = 0; i < 9; i++, j++)
        tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &l);

    l = 9 + l * 2;
    for (i = 9, j = (s - start_add) * 2; i < l; i++, j++)
        memory[j] = o_line[i];
}

void rd_end()
{
    char tmp[7];
    int i, j;

    for (i = 1, j = 0; i < 7; i++, j++)
        tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &first_add);
}

// done
/* Write your own s_load here. -------------------------------------------------------------------------------------------*/
void s_load()
{
    // 確認檔案是否重新load
    if (loaded)
    {
        printf("Error: already loaded in memory!\n");
        return;
    }

    f = fopen(fname, "r");
    if (!f)
    {
        printf("Error: Cannot open file %s!\n", fname);
        return;
    }

    while (fgets(o_line, 80, f))
    {
        switch (o_line[0])
        {
        case 'H': // Header record
            rd_header();
            break;
        case 'T': // Text record
            rd_text();
            break;
        case 'E': // End record
            rd_end();
            break;
        }
    }

    fclose(f);
    printf("%s is loaded successfully.(Starts at %x, Length = %X.)\n", fname, start_add, prog_len);
}

// done
/* Write your own s_show here. ---------------------------------------------------------------------------------------------*/
void s_show()
{
    int i;

    if (!loaded)
    {
        printf("Error: No program is loaded!\n");
        return;
    }

    // 內容
    // printf("Memory Contents:\n");
    for (i = 0; i < mem_size - 1; i += 2)
    {
        if (i % 32 == 0)
        {
            if (i > 0)
                printf("\n");
            printf("%06X", start_add + (i / 2));
        }
        if (i % 8 == 0)
            printf("  ");
        printf("%c%c", memory[i], memory[i + 1]);
    }
    printf("\n");
}

// done
/* Write your own s_unload here. -------------------------------------------------------------------------------------*/
void s_unload()
{

    if (!loaded)
    {
        printf("Error: No program is loaded!\n");
        return;
    }

    free(memory);
    memory = NULL;
    loaded = 0;
    mem_size = 0;
    prog_len = 0;
    start_add = 0;
    first_add = 0;
    printf("%s is unloaded successfully.\n", fname);
}

void init_run()
{
    reg_A = 0;
    reg_X = 0;
    reg_L = 0;
    reg_PC = first_add;
    reg_SW = 0;
    curr_add = (first_add - start_add) * 2;
    running = 1;
}

void get_op()
{
    char s[3];
    char t[5];
    // printf("current address: %X, reg_PC: %X\n", curr_add, reg_PC);

    s[0] = memory[curr_add];
    s[1] = memory[curr_add + 1];
    s[2] = '\0';
    op = lookup(s);
    t[0] = memory[curr_add + 2];
    t[1] = memory[curr_add + 3];
    t[2] = memory[curr_add + 4];
    t[3] = memory[curr_add + 5];
    t[4] = '\0';
    // printf("current instruction: [%s%s]\n", s, t);
    sscanf(t, "%X", &operand);
    if (operand >= IKEY)
    {
        indexed = 1;
        operand -= IKEY;
    }
    else
    {
        indexed = 0;
    }
    curr_add += 6;
    reg_PC += 3;
}

int get_value(int r, int x)
{
    int tmp = 0;
    char s[7];
    int i, j;
    if (x)
        r += reg_X;
    i = (r - start_add) * 2;
    for (j = 0; j < 6; j++)
        s[j] = memory[i++];
    s[6] = '\0';
    sscanf(s, "%X", &tmp);
    // printf("Value = %X\n", tmp);
    return tmp;
}

int get_byte(int r, int x)
{
    int tmp = 0;
    char s[3];
    int i, j;
    if (x)
        r += reg_X;
    i = (r - start_add) * 2;
    j = 0;
    s[j++] = memory[i++];
    s[j++] = memory[i++];
    s[2] = '\0';
    sscanf(s, "%X", &tmp);
    // printf("LDCH: r = [%X], x = [%d], tmp = [%c]\n", r, x, tmp);
    return tmp;
}

void put_byte(int k, int r, int x)
{
    int tmp = 0;
    char s[3];
    int i, j;
    if (x)
        r += reg_X;
    sprintf(s, "%02X", k);
    i = (r - start_add) * 2;
    j = 0;
    memory[i++] = s[j++];
    ;
    memory[i++] = s[j++];
    ;
}

void put_value(int k, int r, int x)
{
    int tmp = 0;
    char s[7];
    int i, j;
    // printf("put_value ( %X, %X, %X ) starts.\n", k, r, x);
    if (x)
        r += reg_X;
    sprintf(s, "%06X", k);
    i = (r - start_add) * 2;
    for (j = 0; j < 6; j++)
        memory[i++] = s[j];
    // printf("put_value ( %X, %X, %X ) finishes.\n", k, r, x);
}

void show_reg()
{
    printf("Register A  = [%06X];\n", reg_A);
    printf("Register X  = [%06X];\n", reg_X);
    printf("Register L  = [%06X];\n", reg_L);
    printf("Register SW = [%06X];\n", reg_SW);
    printf("Register PC = [%06X];\n", reg_PC);
}

/* Write your own s_run here. -----------------------------------------------------------------*/
void s_run()
{
    char input;
    if (!loaded)
    {
        printf("Error: No program is loaded!\n");
        return;
    }

    printf("Start running the program.\n");
    init_run();

    while (running)
    {

        get_op();

        switch (op)
        {
        case oADD: // ADD done
            reg_A += get_value(operand, indexed);
            break;
        case oAND: // AND done
            reg_A &= get_value(operand, indexed);
            break;
        case oDIV: // DIV done
            if (get_value(operand, indexed) != 0)
                reg_A /= get_value(operand, indexed);
            break;
        case oLDA: // LDA done
            reg_A = get_value(operand, indexed);
            break;
        case oLDCH: // LDCH done
            reg_A = (reg_A & 0xFFFF00) | get_byte(operand, indexed);
            break;
        case oLDL: // LDL done
            reg_L = get_value(operand, indexed);
            break;
        case oLDX: // LDX done
            reg_X = get_value(operand, indexed);
            break;
        case oMUL: // MUL done
            reg_A *= get_value(operand, indexed);
            break;
        case oOR: // OR done
            reg_A |= get_value(operand, indexed);
            break;
        case oRSUB: // RSUB done
            if (reg_L == 0)
            {
                // 如果reg_L = 0直接結束
                running = 0;
            }
            else
            {
                reg_PC = reg_L;
                curr_add = (reg_L - start_add) * 2;
            }
            break;
        case oSTA: // STA done
            put_value(reg_A, operand, indexed);
            break;
        case oSTCH: // STCH done
            put_byte(reg_A & 0xFF, operand, indexed);
            break;
        case oSTL: // STL done
            put_value(reg_L, operand, indexed);
            break;
        case oSTX: // STX done
            put_value(reg_X, operand, indexed);
            break;
        case oSUB: // SUB done
            reg_A -= get_value(operand, indexed);
            break;
        case oCOMP: // COMP done
        {
            int tmp = get_value(operand, indexed);
            if (reg_A < tmp)
                reg_SW = -1;
            else if (reg_A > tmp)
                reg_SW = 1;
            else
                reg_SW = 0;
        }
        break;
        case oJ: // J done
            reg_PC = operand;
            curr_add = (operand - start_add) * 2;
            break;
        case oJEQ: // JEQ done
            if (reg_SW == 0)
            {
                reg_PC = operand;
                curr_add = (operand - start_add) * 2;
            }
            break;
        case oJGT: // JGT done
            if (reg_SW > 0)
            {
                reg_PC = operand;
                curr_add = (operand - start_add) * 2;
            }
            break;
        case oJLT: // JLT done
            if (reg_SW < 0)
            {
                reg_PC = operand;
                curr_add = (operand - start_add) * 2;
            }
            break;
        case oJSUB: // JSUB done
            reg_L = reg_PC;
            reg_PC = operand;
            curr_add = (operand - start_add) * 2;
            break;
        case oRD: // RD
            printf("Please input a character: ");
            scanf(" %c", &input);
            reg_A = (reg_A & 0xFFFF00) | (input & 0xFF);
            break;
        case oTD: // TD
            reg_SW = 1;
            break;
        case oTIX: // TIX
            reg_X++;
            {
                int tmp = get_value(operand, indexed);
                if (reg_X < tmp)
                    reg_SW = -1;
                else if (reg_X > tmp)
                    reg_SW = 1;
                else
                    reg_SW = 0;
            }
            break;
        case oWD: // WD
            if (reg_A >= 32 && reg_A <= 126)
                printf("Output a character: [%c]\n", reg_A);
            break;
        default:
            printf("Error: Invalid operation code!\n");
            running = 0;
            break;
        }

        if (reg_PC >= start_add + prog_len)
            running = 0;
    }

    show_reg();
    printf("Program execution ended!\n");
}

int main()
{
    int comm = 0;

    comm = readline();
    while (comm != cEXIT)
    {
        switch (comm)
        {
        case cLOAD:
            s_load();
            break;
        case cSHOW:
            s_show();
            break;
        case cUNLOAD:
            s_unload();
            break;
        case cRUN:
            s_run();
            break;
        default:
            printf("Unknown Command!\n");
            break;
        }
        comm = cMIN;
        comm = readline();
    }
    if (loaded)
        s_unload();
}
