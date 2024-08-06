#include <stdio.h>
#include <stdlib.h>

enum{
    MODE,
    CODEPOS,
    CODEENDPOS,
    TAPEPOS,
    TAPENUM,
    INPUTPOS,
    OUTPOS,
    LINENUM,
    LINESTARTNUM
};

int isbfcode(char);
void drawcode(int*, char*);
void drawtape(int*, char*);
void drawin(int*, char*);
void drawout(int*, char*);
void drawcommand();
char *takecommand();
void docommand(char*, int*, char*, char*, char*, char*);

int main(){
    FILE *fp;
    char tape[20000];
    char bfcode[20000];
    char input[1000]={0};
    char output[1000]={0};
    char *tapep=tape;
    char *codep=bfcode;
    char *codepend=bfcode;
    char fn[100];
    char c;
    int paramarr[9];
    int loopflag=0;
    int i, j;

    printf("file name:\n");
    scanf("%s", fn);
    fflush(stdin);

    printf("input:\n");
    scanf("%s", input);
    fflush(stdin);

    fp=fopen(fn, "r");
    c=fgetc(fp);
    while(c!=-1){
        if(c=='\n'){
            paramarr[LINENUM]++;
        }
        //syntax error check
        if(c=='[')
            loopflag++;
        if(c==']')
            loopflag--;
        if(loopflag<0){
            printf("syntax error at line %d, extra ']'", paramarr[LINENUM]);
            fclose(fp);
            return 0;
        }
        *codepend=c;
        codepend=codepend+1;
        c=fgetc(fp);
    }
    fclose(fp);

    //syntax error check
    if(loopflag>0){
        printf("syntax error: extra '['");
        return 0;
    }

    paramarr[MODE]=0;
    paramarr[CODEPOS]=0;
    paramarr[CODEENDPOS]=codepend-bfcode;
    paramarr[TAPEPOS]=0;
    paramarr[TAPENUM]=0;
    paramarr[INPUTPOS]=0;
    paramarr[OUTPOS]=0;
    paramarr[LINENUM]=1;
    paramarr[LINESTARTNUM]=0;

    codep=bfcode;
    while(!isbfcode(*codep)){
        codep=codep+1;
    }
    paramarr[CODEPOS]=codep-bfcode;
    if(codep==codepend){
        paramarr[MODE]=-1;
    }

    while(paramarr[MODE]!=-1){
        //draw
        drawcode(paramarr, bfcode);
        drawtape(paramarr, tape);
        drawin(paramarr, input);
        drawout(paramarr, output);
        drawcommand();
        // //continue, forward, backward, exit, run, change tape data format, change tape, handle lack of input
        docommand(takecommand(), paramarr, bfcode, tape, input, output);
        // break;
    }
    printf("ENDING\n");

    return 0;
}

int isbfcode(char c){
    switch(c){
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

void drawcode(int *param, char *code){
    int i;
    int codepos=param[CODEPOS];
    int codeendpos=param[CODEENDPOS];
    int linenum=param[LINENUM];
    int linestartnum=param[LINESTARTNUM];
    char *p=code+linestartnum;

    for(i=0; i<3; i++){
        if(p-code<codeendpos && i==0){
            printf("%d\t", linenum+i);
        }
        else if(*p=='\n'){
            printf("%d\t", linenum+i);
            p=p+1;
        }
        while(p-code<codeendpos && *p!='\n'){
            if(p-code==codepos)
                printf("{%c}", *p);
            else
                printf("%c", *p);
            p=p+1;
        }
        printf("\n");
    }
}

void drawtape(int *param, char *tape){
    int tapepos=param[TAPEPOS];
    int tapenum=param[TAPENUM];
    int i;

    printf("TAPE\t");
    for(i=tapenum; i<30+tapenum; i++){
        if(i==tapepos)
            printf("{");
        else if(i==tapepos+1)
            printf("}");
        else if(i==tapenum&&tapenum!=0)
                printf(" ");
        else
            printf("|");

        if(tape[i]>99){
            printf("%d", tape[i]);
        }
        else{
            printf(" %02d", tape[i]);
        }
    }

    if(tapepos==tapenum+30){
        printf("}");
    }
    else{
        printf("|");
    }
    printf("\n");

    printf("    \t");
    for(i=0; i<tapepos-tapenum; i++){
        printf("    ");
    }
    printf("  ^\n");
}

void drawin(int *param, char *input){
    int inputpos=param[INPUTPOS];
    char *ip=input;

    printf("INPUT\t");
    while(*ip!=0){
        if(ip-input==inputpos){
            printf("{%c}", *ip);
        }
        else{
            printf("%c", *ip);
        }
        ip=ip+1;
    }
    printf("\n");
}

void drawout(int *param, char *output){
    int outpos=param[OUTPOS];
    char *op=output;

    printf("OUTPUT\t");
    while(*op!=0){
        if(op-output==outpos){
            printf("{%c}", *op);
        }
        else{
            printf("%c", *op);
        }
        op=op+1;
    }
    printf("\n");
}

void drawcommand(){
    printf("command: ");
}

char *takecommand(){
    char *commandstr=malloc(sizeof(char)*10);

    // scanf("%s", commandstr);
    fgets(commandstr, 3, stdin);
    fflush(stdin);

    return commandstr;
}

void docommand(char *command, int *param, char *code, char *tape, char *input, char *output){
    int loopstack=0;

    if(command[0]=='\n'){
        switch (code[param[CODEPOS]])
        {
        case '+':
            tape[param[TAPEPOS]]++;
            break;
        
        case '-':
            tape[param[TAPEPOS]]--;
            break;
        
        case '<':
            if(param[TAPEPOS]>0){
                param[TAPEPOS]--;
            }
            break;
        
        case '>':
            param[TAPEPOS]++;
            break;
        
        case '.':
            output[param[OUTPOS]]=tape[param[TAPEPOS]];
            param[OUTPOS]++;
            break;
        
        case ',':
            tape[param[TAPEPOS]]=input[param[INPUTPOS]];
            param[INPUTPOS]++;
            break;
        
        case '[':
            if(tape[param[TAPEPOS]]==0){
                loopstack=1;
                while(loopstack!=0){
                    param[CODEPOS]++;
                    if(code[param[CODEPOS]]=='['){
                        loopstack++;
                    }
                    if(code[param[CODEPOS]]==']'){
                        loopstack--;
                    }
                }
            }
            break;
        
        case ']':
            if(tape[param[TAPEPOS]]!=0){
                loopstack=-1;
                while(loopstack!=0){
                    param[CODEPOS]--;
                    if(code[param[CODEPOS]]=='['){
                        loopstack++;
                    }
                    if(code[param[CODEPOS]]==']'){
                        loopstack--;
                    }
                }
            }
            break;
        
        default:
            break;
        }
    }

    do{
        param[CODEPOS]++;
        if(param[CODEPOS]==param[CODEENDPOS]){
            param[MODE]=-1;
            break;
        }
        if(code[param[CODEPOS]-1]=='\n'){
            param[LINENUM]++;
            param[LINESTARTNUM]=param[CODEPOS];
        }
    }while(!isbfcode(code[param[CODEPOS]]));
    free(command);
}
