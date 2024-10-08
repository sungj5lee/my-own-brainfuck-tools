// TODO: check code for mistakes and warn user before executing(loops next together, +- next to each other etc)
// TODO: ask to ignore warning when warnings exist
// TODO: find files easier
// TODO: settings file?
// TODO: define tape features
// TODO: add features(go backward, scroll tape or code, pointer for code, inject tape or input at any time, set checkpoints, auto, auto til condition, better drawcommand)
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
//  #: set breakpoint at current position to stop at. next command to cross stops at breakpoint, when used at execution, stops when tape pointer is at value matching condition, for tape, execution, can take positive number and special command letters for conditions, =/!: (is/not) equal, >/<: larger/smaller, just numbers is equal, can add multiple breakpoints, for multiple conditions, will first do AND, then OR, no option can erase existing breakpoints
//  a: auto moves one step at a time, takes number, no number autos until end or interupt by input, when interupted, will stop and not move a step
//  m: move to a certain step, takes number, resets to start if no number, starts from end if negative
//  t: move focus to tape, takes some numbers, 1 will only move to non null spaces, other is default
//  c: move focus to code, takes some numbers, 1 will move by lines, 2 will only move to brainfuck chars, other is default
//  i: move focus to input
//  p: sets pointer, takes number, set reletive to where you are, if not used, pointer will jump back to where it was once execution starts
//  e: edits where you are, you will be able to use quotation marks to add strings infront of the pointer, single quotes to overwrite, double quotes to insert, use \b to pop space you are on, other common escape chars work too, for tape, can take number, no sign sets to number, +/- adds/subtracts
//  q: exit, leaves above modes to normal execution mode, ends on execution mode
//  z: undo previous command
//  Z: redo next command
//  s: change settings, only takes format of {setting_name:value,}, writing default as value sets to default, just {default} defaults all, no {} sends to setting page
//  v: change tape visual, takes some numbers, no number cycles through options, 1 will show in base 16, 2 will show in chars, other is default

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct commandstrt
{
    int sign;
    int field;
    int num_value;
    char str_value[100];
    char cmd;
} commandstrt;

typedef struct breakpointstrt
{
    int idx;
    char condtion;
    int condition_value;
    struct breakpointstrt *next;
} breakpointstrt;

typedef struct settingstrt
{
    /* data */
} settingstrt;

enum param
{
    MODE,
    DIRECTION,
    FIELD,
    CODE_POINTER_IDX,
    CODE_END_IDX,
    TAPE_POINTER_IDX,
    TAPE_START_IDX,
    INPUT_POINTER_IDX,
    INPUT_END_IDX,
    OUTPUT_POINTER_IDX,
    CURR_LINE_NUM,
    CURR_LINE_START_IDX,
    COMMAND_POINTER_IDX,
    COMMAND_END_IDX,
    //for backwards executing
    INPUT_OVERRIDE_POINTER_IDX,
    LOOP_CYCLE_POINTER_IDX,
    //for stats
    STEP_COUNT,
    //always at end
    PARAM_SIZE,
};

enum direction
{
    STOP,
    FORWARD,
    BACKWARD,
};

enum command
{
    EXIT,
    DEFAULT_MOVEMENT,
    AUTO,
    BREAKPOINT,
    EDIT,
    SETTINGS,
    VISUAL,
};

typedef enum field
{
    EXECUTE,
    TAPE,
    CODE,
    INPUT_LINE,
    //always at end
    FIELD_SIZE,
} field_enum;

int isbfcode(char);
void arrayimagemovementhandler(int *, int, int);
void makedefaultcommandstrt(commandstrt *, int *);
void drawcode(int *, char *);
void drawtape(int *,unsigned char *);
void drawin(int *, char *);
void drawout(int *, char *);
void drawcommand(int *);
void takecommand(int *, commandstrt *);
void docommand(commandstrt *, int *, char *, char *, char *, char *);

int main()
{
    FILE *fp;
    unsigned char tape[20000]={0};
    char bfcode[20000];
    char input[1000] = {0};
    char output[1000] = {0};
    unsigned char inputoverride[1000]={0};
    int loopcyclerecord[10000]={0};
    commandstrt cmdhistory[10000];
    breakpointstrt *breakpointarr[FIELD_SIZE];
    char *inputend = input;
    char *codep = bfcode;
    char *codepend = bfcode;
    char fn[100];
    char c;
    int paramarr[PARAM_SIZE]={0};
    int loopflag = 0;
    int i, j;
    
    printf("file name:\n");
    gets(fn);
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

    paramarr[MODE] = DEFAULT_MOVEMENT;
    paramarr[DIRECTION] = FORWARD;
    paramarr[FIELD] = EXECUTE;
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

    while (paramarr[MODE] != EXIT && paramarr[FIELD]==EXECUTE)
    {
        // draw
        printf("%s\n",fn);
        drawcode(paramarr, bfcode);
        drawtape(paramarr, tape);
        drawin(paramarr, input);
        drawout(paramarr, output);
        drawcommand(paramarr);
        // //continue, forward, backward, exit, run, change tape data format, change tape, handle lack of input
        takecommand(paramarr, cmdhistory);
        docommand(cmdhistory, paramarr, bfcode, tape, input, output);
        // break;
    }
    printf("%s\n",fn);
    drawcode(paramarr, bfcode);
    drawtape(paramarr, tape);
    drawin(paramarr, input);
    drawout(paramarr, output);
    drawcommand(paramarr);

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
    int codewindowlen=5;
    char *p = code + linestartnum;

    for (i = 0; i < codewindowlen; i++)
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

void arrayimagemovementhandler(int *arrstart, int arrl, int ppos)
{
    int buffer=1;

    if(ppos<0+buffer){
        *arrstart=0;
    }
    else if(ppos<*arrstart+buffer){
        *arrstart=ppos-buffer;
    }
    else if(ppos>*arrstart+(arrl-1)-buffer){
        *arrstart=ppos+buffer-(arrl-1);
    }
}

void drawtape(int *param, unsigned char *tape)
{
    int tapelen=30;
    arrayimagemovementhandler(&param[TAPE_START_IDX], tapelen, param[TAPE_POINTER_IDX]);

    int tapepos = param[TAPE_POINTER_IDX];
    int tapenum = param[TAPE_START_IDX];
    int i;

    char hlL_ch='{';
    char hlR_ch='}';
    char tp_ch='^';
    char sepr_ch=' ';
    char mark_ch='|';

    printf("TAPE\t");
    for (i = tapenum; i < tapelen + tapenum; i++)
    {
        if (i == tapepos)
            printf("%c", hlL_ch);
        else if (i == tapepos + 1)
            printf("%c", hlR_ch);
        else if (i == 0)
            printf(" ");
        else if (i%5==0)
            printf("%c", mark_ch);
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
    printf("  %c @%d\n", tp_ch, tapepos);
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
    if(inputpos==param[INPUT_END_IDX]){
        printf("{}");
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

void drawcommand(int *param)
{
    char *step_format="(step %d)";
    char *direction_format="(%c%c)";
    char *comment_format="command: ";
    char direction_char='+';
    char field_char='X';
    char *command_str=(char *)malloc(sizeof(char)*100);

    switch (param[DIRECTION])
    {
    case FORWARD:
        direction_char='+';
        break;
    case BACKWARD:
        direction_char='-';
        break;
    default:
        break;
    }

    switch (param[MODE])
    {
    case EXIT:
        if(param[FIELD]==EXECUTE){
            comment_format="Exiting: ";
        }
        break;
    case AUTO:
        comment_format="Auto-moving...(Press any key to stop)";
        break;
    case EDIT:
        comment_format="Input :";
        break;
    default:
        break;
    }

    switch (param[FIELD])
    {
    case EXECUTE:
        field_char='X';
        break;
    case TAPE:
        field_char='T';
        break;
    case CODE:
        field_char='C';
        break;
    case INPUT_LINE:
        field_char='I';
        break;
    default:
        break;
    }

    printf(step_format, param[STEP_COUNT]);
    printf(direction_format, direction_char, field_char);
    printf(comment_format);
}

void takecommand(int *param, commandstrt *cmdarr)
{
    int i;
    char c;
    char commandstr[1000]={0};
    char *bracketstartp=commandstr;
    char *bracketendp=commandstr;
    char *letterp=commandstr;
    char *nump=commandstr;
    commandstrt* command=&cmdarr[param[COMMAND_POINTER_IDX]];
    int scancheck;
    char cmd_letters[]="#amtcipeqzZsv";
    char start_letters[]="\'\"({";
    char end_letters[]="\'\")}";
    char condition_letters[]="=!<>";
    char number_letters[]="+-0123456789";
    int bracket_moveflag=1;
    int letter_moveflag=1;
    int num_moveflag=1;

    fgets(commandstr, 1000, stdin);
    while (1)
    {
        if(*commandstr=='\n'){
            makedefaultcommandstrt(command,param);
            command->num_value=1;
            command++;
            param[COMMAND_END_IDX]++;
            break;
        }

        if(bracket_moveflag){
            if(strchr(start_letters, *bracketstartp)){
                makedefaultcommandstrt(command, param);
                command->cmd=*bracketendp;
                strncpy(command->str_value, bracketstartp, bracketendp-bracketstartp);
                command++;
                param[COMMAND_END_IDX]++;
                bracketendp++;
                bracketstartp=bracketendp;
            }
            
            bracketstartp=strpbrk(bracketstartp, start_letters);
            while(bracketstartp!=NULL){
                bracketendp=strchr(bracketstartp, end_letters[strchr(start_letters, *bracketstartp)-start_letters]);
                if(bracketendp!=NULL){
                    bracketendp++;
                    if((*bracketstartp=='\'' || *bracketstartp=='\"') && *bracketendp=='e'){
                        break;
                    }
                    if(*bracketstartp=='{' && *bracketendp=='s'){
                        break;
                    }
                }

                bracketstartp=strpbrk(bracketstartp, start_letters);
            }
            bracket_moveflag=0;
            if(letterp!=NULL){
                letter_moveflag=1;
            }
            if(nump!=NULL){
                num_moveflag=1;
            }
        }
        
        if(letter_moveflag){
            if(strchr(cmd_letters, *letterp) && !(bracketstartp<=letterp && letterp<=bracketendp)){
                makedefaultcommandstrt(command, param);
                command->cmd=*letterp;
                if(*letterp=='#' && strchr(condition_letters, *(letterp-1))){
                    command->str_value[0]=*(letterp-1);
                    command->str_value[1]='\0';
                }
                command++;
                param[COMMAND_END_IDX]++;
                letterp++;
            }

            letterp=strpbrk(letterp, cmd_letters);
            while(bracketstartp<=letterp && letterp<=bracketendp && letterp!=NULL){
                bracket_moveflag=1;
                letterp=strpbrk(letterp, cmd_letters);
            }
            letter_moveflag=0;
            if(nump!=NULL){
                num_moveflag=1;
            }
        }

        if(num_moveflag){
            nump=strpbrk(nump, number_letters);
            while(nump!=NULL){
                if(nump>letterp && letterp!=NULL){
                    letter_moveflag=1;
                    break;
                }
                if(nump>bracketstartp && bracketstartp!=NULL){
                    while(bracketstartp<nump && nump<bracketendp && nump!=NULL){
                        nump++;
                        nump=strpbrk(nump, number_letters);
                    }
                    if(bracketstartp!=NULL){
                        bracket_moveflag=1;
                    }
                    break;
                }
                makedefaultcommandstrt(command, param);
                if((*nump=='+' || *nump=='-') && !isdigit(*(nump+1))){
                    command->cmd=*nump;
                }
                if(((*nump=='+' || *nump=='-') && isdigit(*(nump+1))) || isdigit(*nump)){
                    command->num_value=atoi(nump);
                }

                while(isdigit(*(nump+1))){
                    nump++;
                }
                nump++;
                if(strpbrk(nump, "#amtcpev")!=NULL && isdigit(*(nump-1))){
                    command->cmd=*nump;
                    num_moveflag=0;
                    if(letterp!=NULL){
                        letter_moveflag=1;
                    }
                    letterp++;
                    break;
                }
                if(strpbrk(nump, condition_letters)!=NULL && isdigit(*(nump-1)) && *(nump+1)=='#'){
                    command->cmd=*(nump+1);
                    command->str_value[0]=*nump;
                    command->str_value[1]='\0';
                    num_moveflag=0;
                    if(letterp!=NULL){
                        letter_moveflag=1;
                    }
                    letterp++;
                    break;
                }

                param[COMMAND_END_IDX]++;
                command++;
                nump=strpbrk(nump, number_letters);
            }

            if(nump==NULL){
                if(letterp<bracketstartp && letterp!=NULL){
                    letter_moveflag=1;
                }
                else if(bracketstartp!=NULL){
                    bracket_moveflag=1;
                }
            }
            num_moveflag=0;
        }
        
        if(bracketstartp==NULL && letterp==NULL && nump==NULL){
            break;
        }
    }

}

void docommand(commandstrt *cmdarr, int *param, char *code, char *tape, char *input, char *output)
{
    int loopstack = 0;
    enum direction command_direction;
    int i;
    commandstrt* command;
    char chp;

    while(param[COMMAND_POINTER_IDX]<param[COMMAND_END_IDX]){
        command=cmdarr+param[COMMAND_POINTER_IDX];
        (command+1)->field=command->field;
        (command+1)->sign=command->sign;

        switch (command->cmd)
        {
        case 'a':
            param[MODE]=AUTO;
            break;
        case 'd':
            param[MODE]=DEFAULT_MOVEMENT;
            break;
        case 'e':
            param[MODE]=EDIT;
            break;
        case '#':
            param[MODE]=BREAKPOINT;
            break;
        case 'v':
            param[MODE]=VISUAL;
            break;
        case 's':
            param[MODE]=SETTINGS;
            break;
        case 'q':
            param[MODE]=EXIT;
            break;
        
        default:
            param[MODE]=DEFAULT_MOVEMENT;
            break;
        }

        if((command->sign==FORWARD && command->num_value>0) || (command->sign==BACKWARD && command->num_value<0)){
            command_direction=FORWARD;
        }
        else if((command->sign==BACKWARD && command->num_value>0) || (command->sign==FORWARD && command->num_value<0)){
            command_direction=BACKWARD;
        }
        else{
            command_direction=STOP;
        }

        if (command_direction==FORWARD)
        {
            for (i = 0; i < command->num_value; i++)
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
                } while(!isbfcode(code[param[CODE_POINTER_IDX]]));
                param[STEP_COUNT]++;
                if (param[MODE]==EXIT)
                {
                    break;
                }
            }
        }
        if (command_direction==BACKWARD)
        {
            for (i = 0; i < command->num_value; i++)
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
        
        param[COMMAND_POINTER_IDX]++;
    }
}

void makedefaultcommandstrt(commandstrt *cmdp, int *param){
    cmdp->sign=param[DIRECTION];
    cmdp->field=param[FIELD];
    cmdp->cmd='d';
    cmdp->num_value=0;
    strcpy(cmdp->str_value, "");
}
