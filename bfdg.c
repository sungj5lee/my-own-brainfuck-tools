// TODO: check code for mistakes and warn user before executing(loops next together, +- next to each other etc)
// TODO: ask to ignore warning when warnings exist
// TODO: add features(go backward, scroll tape or code, pointer for code, inject tape or input at any time, set checkpoints, step count, show total step count, auto, auto til condition, better drawcommand)
// TODO: figure out good command format
// TODO: fix stuff(i/o display with newlines)
#include <stdio.h>
#include <stdlib.h>

typedef struct commandstrt
{
    int steps;
    int direction;
    char other[10];
} commandstruct;

enum
{
    MODE,
    CODEPOS,
    CODEENDPOS,
    TAPEPOS,
    TAPENUM,
    INPUTPOS,
    INPUTENDPOS,
    OUTPOS,
    LINENUM,
    LINESTARTNUM,
    INPUTOVERRIDEPOS,
    LOOPCYCLEPOS
};

enum
{
    EXIT =-1,
    STOP,
    FORWARD,
    BACKWARD,
    AUTO
};

int isbfcode(char);
void arrayimagemovementhandler(int *, int, int, int);
void drawcode(int *, char *);
void drawtape(int *,unsigned char *);
void drawin(int *, char *);
void drawout(int *, char *);
void drawcommand();
commandstruct takecommand();
void docommand(commandstruct, int *, char *, char *, char *, char *);

int main()
{
    FILE *fp;
    unsigned char tape[20000]={0};
    char bfcode[20000];
    char input[1000] = {0};
    char output[1000] = {0};
    unsigned char inputoverride[1000]={0};
    int loopcyclerecord[10000]={0};
    char *inputend = input;
    char *codep = bfcode;
    char *codepend = bfcode;
    char fn[100];
    char c;
    int paramarr[9];
    int loopflag = 0;
    int i, j;

    printf("file name:\n");
    scanf("%s", fn);
    fflush(stdin);

    printf("input:\n");
    fgets(input, 1000, stdin);
    fflush(stdin);
    while(*inputend!=0){
        inputend=inputend+1;
    }

    fp = fopen(fn, "r");
    c = fgetc(fp);
    while (c != -1)
    {
        if (c == '\n')
        {
            paramarr[CURR_LINE_NUM]++;
        }
        // syntax error check
        if (c == '[')
            loopflag++;
        if (c == ']')
            loopflag--;
        if (loopflag < 0)
        {
            printf("syntax error at line %d, extra ']'", paramarr[CURR_LINE_NUM]);
            fclose(fp);
            return 0;
        }
        *codepend = c;
        codepend = codepend + 1;
        c = fgetc(fp);
    }
    fclose(fp);

    // syntax error check
    if (loopflag > 0)
    {
        printf("syntax error: extra '['");
        return 0;
    }

    paramarr[MODE] = FORWARD;
    paramarr[CODEPOS] = 0;
    paramarr[CODEENDPOS] = codepend - bfcode;
    paramarr[TAPEPOS] = 0;
    paramarr[TAPENUM] = 0;
    paramarr[INPUTPOS] = 0;
    paramarr[INPUTENDPOS] = inputend - input;
    paramarr[OUTPOS] = 0;
    paramarr[LINENUM] = 1;
    paramarr[LINESTARTNUM] = 0;
    paramarr[INPUTOVERRIDEPOS] = 0;
    paramarr[LOOPCYCLEPOS] = 0;

    codep = bfcode;
    while (!isbfcode(*codep))
    {
        codep = codep + 1;
        if(*(codep-1)=='\n'){
            paramarr[CURR_LINE_NUM]++;
            paramarr[CURR_LINE_START_IDX]=codep-bfcode;
        }
    }
    paramarr[CODE_POINTER_IDX] = codep - bfcode;
    if (codep == codepend)
    {
        paramarr[MODE] = EXIT;
    }

    while (paramarr[MODE] != EXIT)
    {
        // draw
        drawcode(paramarr, bfcode);
        drawtape(paramarr, tape);
        drawin(paramarr, input);
        drawout(paramarr, output);
        drawcommand();
        // //continue, forward, backward, exit, run, change tape data format, change tape, handle lack of input
        docommand(takecommand(), paramarr, bfcode, tape, input, output);
        // break;
    }
    drawcode(paramarr, bfcode);
    drawtape(paramarr, tape);
    drawin(paramarr, input);
    drawout(paramarr, output);
    printf("ENDING\n");

    return 0;
}

int isbfcode(char c)
{
    switch (c)
    {
    case '+':
        return 1;
    case '-':
        return 1;
    case '<':
        return 1;
    case '>':
        return 1;
    case ',':
        return 1;
    case '.':
        return 1;
    case '[':
        return 1;
    case ']':
        return 1;
    default:
        return 0;
    }
}

void drawcode(int *param, char *code)
{
    int i;
    int codepos = param[CODE_POINTER_IDX];
    int codeendpos = param[CODE_END_IDX];
    int linenum = param[CURR_LINE_NUM];
    int linestartnum = param[CURR_LINE_START_IDX];
    char *p = code + linestartnum;

    for (i = 0; i < 3; i++)
    {
        if (p - code < codeendpos && i == 0)
        {
            printf("%d\t", linenum + i);
        }
        else if (*p == '\n')
        {
            printf("%d\t", linenum + i);
            p = p + 1;
        }
        while (p - code < codeendpos && *p != '\n')
        {
            if (p - code == codepos)
                printf("{%c}", *p);
            else
                printf("%c", *p);
            p = p + 1;
        }
        printf("\n");
    }
}

void arrayimagemovementhandler(int *arrstart, int arrl, int ppos, int move)
{
    int buffer=1;
    ppos=ppos+move;

    if(*ppos<0+buffer){
        *arrstart=0;
    }
    else if(*ppos<*arrstart+buffer){
        *arrstart=*ppos-buffer;
    }
    else if(*ppos>*arrstart+arrl-buffer){
        *arrstart=*ppos+buffer;
    }
}

void drawtape(int *param, unsigned char *tape)
{
    int tapepos = param[TAPE_POINTER_IDX];
    int tapenum = param[TAPE_START_IDX];
    int i;

    printf("TAPE\t");
    for (i = tapenum; i < 30 + tapenum; i++)
    {
        if (i == tapepos)
            printf("{");
        else if (i == tapepos + 1)
            printf("}");
        else if (i == 0)
            printf(" ");
        else
            printf("|");

        if (tape[i] > 99)
        {
            printf("%d", tape[i]);
        }
        else
        {
            printf(" %02d", tape[i]);
        }
    }

    if (tapepos == tapenum + 30)
    {
        printf("}");
    }
    else
    {
        printf("|");
    }
    printf("\n");

    printf("    \t");
    for (i = 0; i < tapepos - tapenum; i++)
    {
        printf("    ");
    }
    printf("  ^\n");
}

void drawin(int *param, char *input)
{
    int inputpos = param[INPUT_POINTER_IDX];
    char *ip = input;

    printf("INPUT\t");
    while (*ip != 0)
    {
        if (ip - input == inputpos)
        {
            printf("{%c}", *ip);
        }
        else
        {
            printf("%c", *ip);
        }
        ip = ip + 1;
    }
    printf("\n");
}

void drawout(int *param, char *output)
{
    int outpos = param[OUT_POINTER_IDX];
    char *op = output;

    printf("OUTPUT\t");
    while (*op != 0)
    {
        if (op - output == outpos)
        {
            printf("{%c}", *op);
        }
        else
        {
            printf("%c", *op);
        }
        op = op + 1;
    }
    printf("\n");
}

void drawcommand()
{
    printf("command: ");
}

commandstruct takecommand()
{
    //current command: {number}{f|b|a|e}
    char c;
    char commandstr[100];
    char *commandp=commandstr;
    commandstruct command = {1, 1, "\n"};
    int scancheck;

    c=getc(stdin);
    if(c=='-'||c=='+'){
        *commandp=c;
        commandp=commandp+1;
        c=getc(stdin);
    }
    while('0'<=c&&c<='9'){
        *commandp=c;
        commandp=commandp+1;
        c=getc(stdin);
    }
    commandp=command.other;
    while(1){
        *commandp=c;
        commandp=commandp+1;
        if(c=='\n'){
            *commandp=0;
            break;
        }
        c=getc(stdin);
    }
    scancheck=sscanf(commandstr, "%d", &command.steps);
    fflush(stdin);
    if(scancheck==0){
        command.steps=1;
    }
    if(command.steps<0){
        command.direction=-1;
        command.steps=command.steps*-1;
    }

    return command;
}

void docommand(commandstruct command, int *param, char *code, char *tape, char *input, char *output)
{
    //MODE -1 exit, 0 stop, 1 forward, 2 backward, 3 auto 
    int loopstack = 0;
    int i;
    int exitflag=0;
    char *chp;

    chp=command.other;
    while(*chp!=0){
        if(*chp=='f'||*chp=='F'){
            param[MODE]=FORWARD;
            break;
        }
        if(*chp=='b'||*chp=='B'){
            param[MODE]=BACKWARD;
            break;
        }
        if(*chp=='a'||*chp=='A'){
            param[MODE]=AUTO;
            break;
        }
        if(*chp=='e'||*chp=='E'||*chp=='q'||*chp=='Q'){
            exitflag=1;
            break;
        }
        chp=chp+1;
    }

    if ((param[MODE]==FORWARD&&command.direction>0)||(param[MODE]==BACKWARD&&command.direction<0)||(param[MODE]==AUTO&&command.direction>0))
    {
        for (i = 0; i < command.steps; i++)
        {
            loopstack=0;

            do
            {
                param[CODEPOS]--;
                if (param[CODEPOS] == 0)
                {
                    //TODO:change to just stop
                    param[MODE] = EXIT;
                    break;
                }
                if (code[param[CODEPOS]] == '\n')
                {
                    param[LINENUM]--;
                    param[LINESTARTNUM] = param[CODEPOS];
                    while(code[param[LINESTARTNUM-1]!='\n']&&param[LINESTARTNUM]!=0)
                    {
                        param[LINESTARTNUM]--;
                    }
                }
            } while (!isbfcode(code[param[CODEPOS]]));

            switch (code[param[CODEPOS]])
            {
            case '+':
                tape[param[TAPEPOS]]++;
                break;

            case '-':
                tape[param[TAPEPOS]]--;
                break;

            case '<':
                if (param[TAPEPOS] > 0)
                {
                    param[TAPEPOS]--;
                }
                break;

            case '>':
                param[TAPEPOS]++;
                break;

            case '.':
                output[param[OUTPOS]] = tape[param[TAPEPOS]];
                param[OUTPOS]++;
                break;

            case ',':
                if(param[INPUTPOS]==param[INPUTENDPOS]){
                    printf("more input needed:");
                    fgets(input+param[INPUTENDPOS], 1000, stdin);
                    while(input[param[INPUTENDPOS]]!=0){
                        param[INPUTENDPOS]++;
                    }
                }
                tape[param[TAPEPOS]] = input[param[INPUTPOS]];
                param[INPUTPOS]++;
                break;

            case '[':
                if (tape[param[TAPEPOS]] == 0)
                {
                    loopstack = 1;
                    while (loopstack != 0)
                    {
                        param[CODEPOS]++;
                        if (code[param[CODEPOS]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODEPOS]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODEPOS]-1] == '\n')
                        {
                            param[LINENUM]++;
                            param[LINESTARTNUM] = param[CODEPOS];
                        }
                    }
                }
                break;

            case ']':
                if (tape[param[TAPEPOS]] != 0)
                {
                    loopstack = -1;
                    while (loopstack != 0)
                    {
                        param[CODEPOS]--;
                        if (code[param[CODEPOS]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODEPOS]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODEPOS]] == '\n')
                        {
                            param[LINENUM]--;
                            param[LINESTARTNUM] = param[CODEPOS];
                            while(code[param[LINESTARTNUM-1]!='\n']&&param[LINESTARTNUM]!=0)
                            {
                                param[LINESTARTNUM]--;
                            }
                        }
                    }
                }
                break;

            default:
                break;
            }

            do
            {
                param[CODEPOS]++;
                if (param[CODEPOS] == param[CODEENDPOS])
                {
                    param[MODE] = EXIT;
                    break;
                }
                if (code[param[CODEPOS] - 1] == '\n')
                {
                    param[LINENUM]++;
                    param[LINESTARTNUM] = param[CODEPOS];
                }
            } while (!isbfcode(code[param[CODEPOS]]));
            if (param[MODE]==EXIT)
            {
                break;
            }
        }
    }
    if ((param[MODE]==FORWARD&&command.direction<0)||(param[MODE]==BACKWARD&&command.direction>0)||(param[MODE]==AUTO&&command.direction<0))
    {
        for (i = 0; i < command.steps; i++)
        {
            loopstack=0;

            do
            {
                param[CODEPOS]--;
                if (param[CODEPOS] == 0)
                {
                    //TODO:change to just stop
                    param[MODE] = EXIT;
                    break;
                }
                if (code[param[CODEPOS]] == '\n')
                {
                    param[LINENUM]--;
                    param[LINESTARTNUM] = param[CODEPOS];
                    while(code[param[LINESTARTNUM-1]!='\n']&&param[LINESTARTNUM]!=0)
                    {
                        param[LINESTARTNUM]--;
                    }
                }
            } while (!isbfcode(code[param[CODEPOS]]));

            switch (code[param[CODEPOS]])
            {
            case '+':
                tape[param[TAPEPOS]]++;
                break;

            case '-':
                tape[param[TAPEPOS]]--;
                break;

            case '>':
                if (param[TAPEPOS] > 0)
                {
                    param[TAPEPOS]--;
                }
                break;

            case '<':
                param[TAPEPOS]++;
                break;

            case '.':
                output[param[OUTPOS]] = 0;
                param[OUTPOS]--;
                break;

            case ',':
                //TODO: choose between forgeting or keeping past inputs
                //TODO: save overwritten spaces
                param[INPUTPOS]--;
                break;

            case '[':
                //TODO: add loop cycle tracker
                if (tape[param[TAPEPOS]] == 0)
                {
                    loopstack = 1;
                    while (loopstack != 0)
                    {
                        param[CODEPOS]++;
                        if (code[param[CODEPOS]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODEPOS]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODEPOS]-1] == '\n')
                        {
                            param[LINENUM]++;
                            param[LINESTARTNUM] = param[CODEPOS];
                        }
                    }
                }
                break;

            case ']':
                if (tape[param[TAPEPOS]] != 0)
                {
                    loopstack = -1;
                    while (loopstack != 0)
                    {
                        param[CODEPOS]--;
                        if (code[param[CODEPOS]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODEPOS]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODEPOS]] == '\n')
                        {
                            param[LINENUM]--;
                            param[LINESTARTNUM] = param[CODEPOS];
                            while(code[param[LINESTARTNUM-1]!='\n']&&param[LINESTARTNUM]!=0)
                            {
                                param[LINESTARTNUM]--;
                            }
                        }
                    }
                }
                break;

            default:
                break;
            }

            if (param[MODE]==EXIT)
            {
                break;
            }
        }
    }
    
    if(exitflag>0){
        param[MODE]=-1;
    }
    
    if(exitflag>0){
        param[MODE]=-1;
    }
}
