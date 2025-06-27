#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXOP 27

char name[16];
FILE *f;
FILE *fobj;

char line[80];
char label[7];
char op[7];
char operand[10];
char new_operand[7];
int indexed = 0;

char prog_name[7];
int start_addr = 0;
int prog_len = 0;
char obj_line[70];
char obj_code[7];
int locctr = 0;
int textpos = 0;

const char a_start[] = "START";
const char a_end[] = "END";
const char a_byte[] = "BYTE";
const char a_word[] = "WORD";
const char a_resb[] = "RESB";
const char a_resw[] = "RESW";

const char optab[26][2][6] =
    {{"ADD", "18"}, {"AND", "40"}, {"COMP", "28"}, {"DIV", "24"}, {"J", "3C"}, {"JEQ", "30"}, {"JGT", "34"}, {"JLT", "38"}, {"JSUB", "48"}, {"LDA", "00"}, {"LDCH", "50"}, {"LDL", "08"}, {"LDX", "04"}, {"MUL", "20"}, {"OR", "44"}, {"RD", "D8"}, {"RSUB", "4C"}, {"STA", "0C"}, {"STCH", "54"}, {"STL", "14"}, {"STSW", "E8"}, {"STX", "10"}, {"SUB", "1C"}, {"TD", "E0"}, {"TIX", "2C"}, {"WD", "DC"}};
const char hex_c[16] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
     'E', 'F'};

typedef struct sym_node
{
    char l[7];
    int v;
    struct sym_node *next;
} symNODE;

symNODE *symtab = NULL;

symNODE *insert(char *s, int r)
{
    symNODE *t = NULL;
    t = (struct sym_node *)malloc(sizeof(struct sym_node));
    if (t)
    {
        strcpy(t->l, s);
        t->v = r;
        t->next = symtab;
    }
    return t;
}

symNODE *search(symNODE *t, char *s)
{
    if (t)
    {
        if (strcmp(s, t->l) == 0)
        {
            return t;
        }
        else
        {
            return search(t->next, s);
        }
    }
    else
    {
        return NULL;
    }
}

symNODE *new_search(char *s)
{
    int i;
    strcpy(new_operand, s);
    for (i = strlen(new_operand); i < 6; i++)
        new_operand[i] = ' ';
    new_operand[6] = '\0';
    return search(symtab, new_operand);
}

char *lookup(char *s)
{
    int i = 0;
    int nf = 1;
    while ((i < MAXOP) && (nf))
    {
        if (strcmp(s, optab[i][0]) == 0)
            nf = 0;
        else
            i++;
    }
    if (i >= MAXOP)
        return NULL;
    else
        return (char *)optab[i][1];
}

int operand_len()
{
    int i, l;
    l = strlen(operand);
    // for (i=0; i<l; i++) printf("operand[%d] = [%c].\n", i, operand[i]);
    if (operand[0] == 'C')
        l -= 3;
    else if (operand[0] == 'X')
        l = (l - 3) / 2;
    // printf("find_length([%s] = %d.\n", operand, l);
    return l;
}

int readline()
{
    int i, j, l, x;

    fgets(line, 80, f);
    l = strlen(line);
    if ((l > 0) && (line[0] != '.'))
    {
        for (i = 0; i < 6; i++)
        {
            label[i] = line[i];
        }
        label[i] = '\0';
        while (line[i] == ' ')
            i++;
        j = 0;
        while ((line[i] != ' ') && (line[i] != '\0') && (line[i] != '\n') && (i < l))
        {
            op[j] = line[i];
            i++;
            j++;
        }
        op[j] = '\0';
        while (line[i] == ' ')
            i++;
        j = 0;
        while ((line[i] != ' ') && (line[i] != '\0') && (line[i] != '\n') && (i < l))
        {
            operand[j] = line[i];
            i++;
            j++;
        }
        operand[j] = '\0';
        indexed = 0;
        x = strlen(operand);
        if ((x > 2) && (operand[x - 2] == ',') && (operand[x - 1] == 'X'))
        {
            operand[x - 2] = '\0';
            indexed = 1;
        }
        //   printf("[%6X] Read a line: label=[%s], op=[%s], operand=[%s].\n",
        //       locctr, label, op, operand);
        return 1;
    }
    else
    {
        return 0;
    }
}
// Write your own pass1()
void pass1()
{
    int line_valid;
    symNODE *found;
    char *opcode;

    line_valid = readline();

    if (strcmp(op, a_start) == 0)
    {

        strcpy(prog_name, label);

        sscanf(operand, "%X", &start_addr);

        locctr = start_addr;

        line_valid = readline();
    }
    else
    {
        locctr = 0;
    }

    while (strcmp(op, a_end) != 0)
    {
        if (line_valid)
        { // If not comment line

            if (strcmp(label, "      ") != 0)
            {

                found = new_search(label);
                if (found != NULL)
                {
                    printf("Error: Duplicate symbol [%s]\n", label);
                }
                else
                {
                    // Insert (LABEL, LOCCTR) into SYMTAB
                    symtab = insert(label, locctr);
                }
            }

            opcode = lookup(op);
            if (opcode != NULL)
            {
                // Add 3 to LOCCTR (standard instruction length)
                // printf("loc_ctr: %04X\n", locctr);
                locctr += 3;
            }
            else if (strcmp(op, a_word) == 0)
            {
                // Add 3 to LOCCTR (3 bytes for WORD)
                // printf("loc_ctr: %04X\n", locctr);
                locctr += 3;
            }
            else if (strcmp(op, a_resw) == 0)
            {
                // Add 3 * #[OPERAND] to LOCCTR
                // printf("loc_ctr: %04X\n", locctr);
                locctr += (3 * atoi(operand));
            }
            else if (strcmp(op, a_resb) == 0)
            {
                // Add #[OPERAND] to LOCCTR
                // printf("loc_ctr: %04X\n", locctr);
                locctr += atoi(operand);
            }
            else if (strcmp(op, a_byte) == 0)
            {
                // Add length of constant
                // printf("loc_ctr: %04X\n", locctr);
                locctr += operand_len();
            }
            else
            {
                printf("Error: Invalid operation code [%s]\n", op);
            }
        }

        line_valid = readline();
    }

    prog_len = locctr - start_addr;
}

void print_symtab(symNODE *t)
{
    if (t)
    {
        print_symtab(t->next);
        printf("[%s] = [%5X]\n", t->l, t->v);
    }
}

void init_obj_line()
{
    int i;
    for (i = 0; i < 70; i++)
        obj_line[i] = ' ';
    obj_line[i] = '\0';
}

// Write your own wr_header()
void wr_header()
{
    // H + prog_name(6) + start_addr(6) + prog_len(6)
    fprintf(fobj, "H%-6s%06X%06X\n", prog_name, start_addr, prog_len);
}

void init_obj_code()
{
    for (int i = 0; i < 6; i++)
        obj_code[i] = ' ';
    obj_code[6] = '\0';
}

void conv_byte(int l, char *p, char *q)
{
    // printf("%c\n",p[0]);
    int i, j, k, max, c, m, n;
    if (p[0] == 'X')
    {
        max = 2 * l;
        for (i = 2, j = 0, k = 0; k < max; i++, j++, k++)
            q[j] = p[i];
        q[j] = '\0';
    }
    else if (p[0] == 'C')
    {
        max = l;
        for (i = 2, j = 0, k = 0; k < max; i++, k++)
        {
            c = (int)p[i];
            m = c / 16;
            q[j++] = hex_c[m];
            n = c % 16;
            q[j++] = hex_c[n];
        }
        q[j] = '\0';
    }
    else
    {
        printf("Error: wrong operand of BYTE!\n");
    }
}

void init_text()
{
    init_obj_line();
    sprintf(obj_line, "T%6X  ", locctr);
    for (int i = 1; i < 7; i++)
        if (obj_line[i] == ' ')
            obj_line[i] = '0';
    textpos = 9;
}

// Write your own wr_text()
void wr_text()
{
    // T + startaddress(6) + len(object code)(2) + ~~~~
    int i;
    int len = (textpos - 9) / 2;
    obj_line[7] = hex_c[len / 16];
    obj_line[8] = hex_c[len % 16];

    for (i = 0; i < textpos; i++)
        fprintf(fobj, "%c", obj_line[i]);
    fprintf(fobj, "\n");
}

void add_text(int n, char *p)
{
    int const max = 69;
    int k = n * 2;
    int i;
    if ((textpos + k) > max)
    {
        wr_text();
        init_text();
    }
    for (i = 0; i < k; i++)
        obj_line[textpos++] = p[i];
}

// Write your own wr_end()
void wr_end()
{
    // E + start_addr(6)
    fprintf(fobj, "E%06X\n", start_addr);
}

// Write your own pass2()
void pass2()
{
    int line_valid;
    symNODE *found;
    char *opcode;
    int operand_addr;

    line_valid = readline();

    if (strcmp(op, a_start) == 0)
    {
        locctr = start_addr;
        line_valid = readline();
    }

    wr_header();

    init_text();

    while (strcmp(op, a_end) != 0)
    {
        if (line_valid)
        { // If this is not a comment line
            opcode = lookup(op);
            if (opcode != NULL)
            {
                if (strlen(operand) > 0)
                {
                    found = new_search(operand);
                    if (found != NULL)
                    {
                        operand_addr = found->v;
                        // Set X bit if indexed addressing(|= 0x8000) + 2^15
                        if (indexed)
                            operand_addr += 32768;
                    }
                    else
                    {
                        printf("Error: Undefined symbol [%s]\n", operand);
                        operand_addr = 0;
                    }
                }
                else
                {
                    operand_addr = 0;
                }
                //----------------------------------------------
                if (textpos + 6 > 69)
                {
                    wr_text();
                    init_text();
                }

                init_obj_code();
                sprintf(obj_code, "%s%04X", opcode, operand_addr);
                printf("obj_code: %04X [%s]\n", locctr, obj_code);
                add_text(3, obj_code);
                locctr += 3;
            }
            else if (strcmp(op, a_byte) == 0)
            {
                //----------------------------------------------
                if (textpos + (operand_len() * 2) > 69)
                {
                    wr_text();
                    init_text();
                }

                init_obj_code();
                conv_byte(operand_len(), operand, obj_code);
                // printf("obj_code: %04X [%s]\n", locctr, obj_code);
                add_text(operand_len(), obj_code);
                locctr += operand_len();
            }
            else if (strcmp(op, a_word) == 0)
            {
                //----------------------------------------------
                if (textpos + 6 > 69)
                {
                    wr_text();
                    init_text();
                }

                init_obj_code();
                sprintf(obj_code, "%06X", atoi(operand));
                // printf("obj_code: %04X [%s]\n", locctr, obj_code);
                add_text(3, obj_code);
                locctr += 3;
            }
            else if (strcmp(op, a_resw) == 0)
            {
                if (textpos > 9)
                {
                    wr_text();
                }
                locctr += 3 * atoi(operand);
                init_text();
            }
            else if (strcmp(op, a_resb) == 0)
            {
                if (textpos > 9)
                {
                    wr_text();
                }
                locctr += atoi(operand);
                init_text();
            }
        }

        line_valid = readline();
    }

    if (textpos > 9)
        wr_text();

    wr_end();
}

int main(int argc, char *argv[])
{
    int t = argc;
    char fname[20];
    int i = 0;
    if (t == 2)
    {
        f = fopen(argv[1], "r");
        if (f)
        {
            printf("... Assembling %s!\n", argv[1]);
            pass1();
            printf("...... End of Pass 1; Program length = %6X.\n", prog_len);
            printf("...... Contents in SymbTab:\n");
            print_symtab(symtab);
            fclose(f);
            strcpy(fname, argv[1]);
            for (i = 0; (fname[i] != '.') && (fname[i] != '\0'); i++)
                ;
            fname[i++] = '.';
            fname[i++] = 'o';
            fname[i++] = 'b';
            fname[i++] = 'j';
            fname[i] = '\0';
            f = fopen(argv[1], "r");
            fobj = fopen(fname, "w");
            printf("...... Start of Pass 2.\n");
            pass2();
            printf("Assembling succeeded.  %s is generated.\n", fname);
            fclose(f);
            fclose(fobj);
        }
        else
        {
            printf("Assemble syntax: [assemble soure_file_name]\n");
        } // f
    }
    else
    {
        printf("Assemble syntax: [assemble soure_file_name]\n");
    } // t
}