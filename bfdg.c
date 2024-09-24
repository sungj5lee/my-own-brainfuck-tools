// TODO: check code for mistakes and warn user before executing(loops next together, +- next to each other etc)
// TODO: ask to ignore warning when warnings exist
// TODO: find files easier
// TODO: settings file?
// TODO: add features(go backward, scroll tape or code, pointer for code, inject tape or input at any time, set checkpoints, step count, show total step count, auto, auto til condition, better drawcommand)
// TODO: figure out good command format
// ---> (+/-: move default/opposite)(numbers: steps)(letters: a:auto mode, c: draw char, n: draw num, e: edit mode)
// ---> 100a: auto for 100 steps, -a: auto opposite way, -100a: auto opposite way for 100 steps, +/-: set movement to for/backwards
// ---> spaces execute next command ex)-100 100a: move 100 steps back then auto back
// ---> nothing: move 1 step
// ---> number: amount of steps
// ---> +/-: set movement to forwards or backwards
// ---> command letters: valid letters that may take a number in front of them as an argument
// ---> non command letters: invalid letters that are ignored. can be used to space out commands in one line
// ---> parsing - command is divided by +/-, numbers, and each command letter.
//                combining takes priority unless seperated by non command letters
// ---> command letters -----
//  a: auto moves one step at a time, takes number, no number autos until end or interupt by input, input will be taken as command unless its nothing(enter), enter will stop and not move one step
//  #: set breakpoint at current position to stop at. next command to cross stops at breakpoint, can take number and special command letters for conditions, + or none: equal, -: not equal, >/<: larger/smaller, >=/<=: le/se, can add multiple breakpoints
//  t: move tape pointer, takes number, isn't saved once moving
//  T: does t and saves position
//  c: move code pointer, takes number, isn't saved once moving
//  C: does c and saves position
//  e: edit current tape space, takes number, +/-: add/subtract, number with no sign: set to number, saved?
//  E: does e and saves tape?
//  z: undo previous command
//  Z: redo next command
//  i: move input pointer and add/change current input space, takes number, next char after is?
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
    CODE_POINTER_IDX,
    CODE_END_IDX,
    TAPE_POINTER_IDX,
    TAPE_START_IDX,
    INPUT_POINTER_IDX,
    INPUT_END_IDX,
    OUTPUT_POINTER_IDX,
    CURR_LINE_NUM,
    CURR_LINE_START_IDX,
    //for backwards executing
    INPUT_OVERRIDE_POINTER_IDX,
    LOOP_CYCLE_POINTER_IDX,
    //for stats
    STEP_COUNT,
};

enum
{
    EXIT =-1,
    STOP,
    FORWARD,
    BACKWARD,
    AUTO,
};

int isbfcode(char);
void arrayimagemovementhandler(int *, int, int, int);
void drawcode(int *, char *);
void drawtape(int *,unsigned char *);
void drawin(int *, char *);
void drawout(int *, char *);
void drawcommand(int *);
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
    int paramarr[9]={0};
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
    paramarr[CODE_END_IDX] = codepend - bfcode;
    paramarr[INPUT_END_IDX] = inputend - input;
    paramarr[CURR_LINE_NUM] = 1;

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
        drawcommand(paramarr);
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
    case '-':
    case '<':
    case '>':
    case ',':
    case '.':
    case '[':
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

    if(ppos<0+buffer){
        *arrstart=0;
    }
    else if(ppos<*arrstart+buffer){
        *arrstart=ppos-buffer;
    }
    else if(ppos>*arrstart+arrl-buffer){
        *arrstart=ppos+buffer;
    }
}

void drawtape(int *param, unsigned char *tape)
{
    int tapepos = param[TAPE_POINTER_IDX];
    int tapenum = param[TAPE_START_IDX];
    int i;

    char hlL_ch='{';
    char hlR_ch='}';
    char tp_ch='^';
    char sepr_ch=' ';

    printf("TAPE\t");
    for (i = tapenum; i < 30 + tapenum; i++)
    {
        if (i == tapepos)
            printf("%c", hlL_ch);
        else if (i == tapepos + 1)
            printf("%c", hlR_ch);
        else if (i == 0)
            printf(" ");
        else
            printf("%c", sepr_ch);

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
        printf("%c", hlR_ch);
    }
    else
    {
        printf("%c", sepr_ch);
    }
    printf("\n");

    printf("    \t");
    for (i = 0; i < tapepos - tapenum; i++)
    {
        printf("    ");
    }
    printf("  %c\n", tp_ch);
}
//TODO: generalize special input cases and clean code
void drawin(int *param, char *input)
{
    int inputpos = param[INPUT_POINTER_IDX];
    char *ip = input;

    printf("INPUT\t");
    while (*ip != 0)
    {
        if (ip - input == inputpos)
        {
            if(*ip=='\n'&&*(ip+1)==0){
                printf("{\\n}");
            }
            else if(*ip=='\n'&&*(ip+1)!=0){
                printf("{\\n}\n");
            }
            else{
                printf("{%c}", *ip);
            }
        }
        else
        {
            if(*ip=='\n'&&*(ip+1)==0){
                printf("\\n");
            }
            else if(*ip=='\n'&&*(ip+1)!=0){
                printf("\\n\n");
            }
            else{
                printf("%c", *ip);
            }
        }
        ip = ip + 1;
    }
    printf("\n");
}

void drawout(int *param, char *output)
{
    int outpos = param[OUTPUT_POINTER_IDX];
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

            switch (code[param[CODE_POINTER_IDX]])
            {
            case '+':
                tape[param[TAPE_POINTER_IDX]]++;
                break;

            case '-':
                tape[param[TAPE_POINTER_IDX]]--;
                break;

            case '<':
                if (param[TAPE_POINTER_IDX] > 0)
                {
                    param[TAPE_POINTER_IDX]--;
                }
                break;

            case '>':
                param[TAPE_POINTER_IDX]++;
                break;

            case '.':
                output[param[OUTPUT_POINTER_IDX]] = tape[param[TAPE_POINTER_IDX]];
                param[OUTPUT_POINTER_IDX]++;
                break;

            case ',':
                if(param[INPUT_POINTER_IDX]==param[INPUT_END_IDX]){
                    printf("more input needed:");
                    fgets(input+param[INPUT_END_IDX], 1000, stdin);
                    while(input[param[INPUT_END_IDX]]!=0){
                        param[INPUT_END_IDX]++;
                    }
                }
                tape[param[TAPE_POINTER_IDX]] = input[param[INPUT_POINTER_IDX]];
                param[INPUT_POINTER_IDX]++;
                break;

            case '[':
                if (tape[param[TAPE_POINTER_IDX]] == 0)
                {
                    loopstack = 1;
                    while (loopstack != 0)
                    {
                        param[CODE_POINTER_IDX]++;
                        if (code[param[CODE_POINTER_IDX]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODE_POINTER_IDX]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODE_POINTER_IDX]-1] == '\n')
                        {
                            param[CURR_LINE_NUM]++;
                            param[CURR_LINE_START_IDX] = param[CODE_POINTER_IDX];
                        }
                    }
                }
                break;

            case ']':
                if (tape[param[TAPE_POINTER_IDX]] != 0)
                {
                    loopstack = -1;
                    while (loopstack != 0)
                    {
                        param[CODE_POINTER_IDX]--;
                        if (code[param[CODE_POINTER_IDX]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODE_POINTER_IDX]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODE_POINTER_IDX]] == '\n')
                        {
                            param[CURR_LINE_NUM]--;
                            param[CURR_LINE_START_IDX] = param[CODE_POINTER_IDX];
                            while(code[param[CURR_LINE_START_IDX-1]!='\n']&&param[CURR_LINE_START_IDX]!=0)
                            {
                                param[CURR_LINE_START_IDX]--;
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
                param[CODE_POINTER_IDX]++;
                if (param[CODE_POINTER_IDX] == param[CODE_END_IDX])
                {
                    param[MODE] = EXIT;
                    break;
                }
                if (code[param[CODE_POINTER_IDX] - 1] == '\n')
                {
                    param[CURR_LINE_NUM]++;
                    param[CURR_LINE_START_IDX] = param[CODE_POINTER_IDX];
                }
            } while (!isbfcode(code[param[CODE_POINTER_IDX]]));
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
                param[CODE_POINTER_IDX]--;
                if (param[CODE_POINTER_IDX] == 0)
                {
                    //TODO:change to just stop
                    param[MODE] = EXIT;
                    break;
                }
                if (code[param[CODE_POINTER_IDX]] == '\n')
                {
                    param[CURR_LINE_NUM]--;
                    param[CURR_LINE_START_IDX] = param[CODE_POINTER_IDX];
                    while(code[param[CURR_LINE_START_IDX-1]!='\n']&&param[CURR_LINE_START_IDX]!=0)
                    {
                        param[CURR_LINE_START_IDX]--;
                    }
                }
            } while (!isbfcode(code[param[CODE_POINTER_IDX]]));

            switch (code[param[CODE_POINTER_IDX]])
            {
            case '+':
                tape[param[TAPE_POINTER_IDX]]++;
                break;

            case '-':
                tape[param[TAPE_POINTER_IDX]]--;
                break;

            case '>':
                if (param[TAPE_POINTER_IDX] > 0)
                {
                    param[TAPE_POINTER_IDX]--;
                }
                break;

            case '<':
                param[TAPE_POINTER_IDX]++;
                break;

            case '.':
                output[param[OUTPUT_POINTER_IDX]] = 0;
                param[OUTPUT_POINTER_IDX]--;
                break;

            case ',':
                //TODO: choose between forgeting or keeping past inputs
                //TODO: save overwritten spaces
                param[INPUT_POINTER_IDX]--;
                break;

            case '[':
                //TODO: add loop cycle tracker
                if (tape[param[TAPE_POINTER_IDX]] == 0)
                {
                    loopstack = 1;
                    while (loopstack != 0)
                    {
                        param[CODE_POINTER_IDX]++;
                        if (code[param[CODE_POINTER_IDX]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODE_POINTER_IDX]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODE_POINTER_IDX]-1] == '\n')
                        {
                            param[CURR_LINE_NUM]++;
                            param[CURR_LINE_START_IDX] = param[CODE_POINTER_IDX];
                        }
                    }
                }
                break;

            case ']':
                if (tape[param[TAPE_POINTER_IDX]] != 0)
                {
                    loopstack = -1;
                    while (loopstack != 0)
                    {
                        param[CODE_POINTER_IDX]--;
                        if (code[param[CODE_POINTER_IDX]] == '[')
                        {
                            loopstack++;
                        }
                        if (code[param[CODE_POINTER_IDX]] == ']')
                        {
                            loopstack--;
                        }
                        if (code[param[CODE_POINTER_IDX]] == '\n')
                        {
                            param[CURR_LINE_NUM]--;
                            param[CURR_LINE_START_IDX] = param[CODE_POINTER_IDX];
                            while(code[param[CURR_LINE_START_IDX-1]!='\n']&&param[CURR_LINE_START_IDX]!=0)
                            {
                                param[CURR_LINE_START_IDX]--;
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
}
