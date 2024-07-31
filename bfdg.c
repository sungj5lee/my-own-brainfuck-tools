#include <stdio.h>
#include <windows.h>

int main(){
    FILE* fp;
    char tape[20000];
    char bfcode[20000];
    char input[1000];
    char output[1000];
    char* tapep=tape;
    char* codep=bfcode;
    char* codepend=bfcode;
    char fn[100];
    char c;
    int loopflag=0;
    int linenum=1;
    int charnum=1;

    printf("file name:\n");
    scanf("%s", fn);
    fflush(stdin);
    fp=fopen(fn, "r");
    c=fgetc(fp);
    while(c!=-1){
        if(c=='\n'){
            linenum++;
            charnum=1;
        }
        //syntax error check
        if(c=='[')
            loopflag++;
        if(c==']')
            loopflag--;
        if(loopflag<0){
            printf("syntax error at line %d, char %d: extra ']'", linenum, charnum);
            fclose(fp);
            return 0;
        }
        *codepend=c;
        codepend=codepend+1;
        c=fgetc(fp);
        charnum++;
    }
    fclose(fp);

    //syntax error check
    if(loopflag>0){
        printf("syntax error: extra '['");
        return 0;
    }

    linenum=1;
    int codepos=0;
    int tapepos=0;
    int tapeflag=0;
    int inputpos=0;
    int outpos=0;
    int i, j;
    
    while(1){
        //do code
        switch (bfcode[codepos])
        {
        case '>':
            tapepos++;
            break;
        
        case '<':
            if(tapepos>0)
                tapepos--;
            break;
        
        case '+':
            tape[tapepos]++;
            break;
        
        case '-':
            tape[tapepos]--;
            break;
        
        case '.':
            tape[tapepos]=input[inputpos];
            inputpos++;
            break;
        
        case ',':
            output[outpos]=tape[tapepos];
            outpos++;
            break;

        default:
            break;
        }
        //print code lines
        codep=bfcode;
        for(i=0; i<3; i++){
            if(codep!=codepend)
                printf("%d\t", linenum+i);
            if(*codep=='\n'){
                codep=codep+1;
            }
            while(*codep!='\n' && codep!=codepend){
                if(codepos==codep-bfcode){
                    printf("|%c|",*codep);
                }
                else{
                    printf("%c",*codep);
                }
                codep=codep+1;
            }
            printf("\n");
        }
        //print tape
        tapep=tape+tapepos;
        printf("TAPE  ");
        for(i=-10; i<10; i++){
            tapeflag=1;
            if(tapepos+i>=0){
                if(*(tapep+i)<10)
                    printf(" %02d",  (unsigned char)*(tapep+i));
                else
                    printf("%d",  (unsigned char)*(tapep+i));
            }
            else{
                printf("   ");
                tapeflag=0;
            }
            if(i==9)
                tapeflag=0;
            if(i==-1 || i==0)
                tapeflag=2;
            for(j=0; j<tapeflag; j++){
                printf("|");
            }
        }
        printf("\n");
        //print input and output
        printf("INPUT  %s\n", input);
        printf("OUTPUT %s\n", output);
        //prompt, take command input
        printf("command:");
        switch (getc(stdin))
        {
        //next move
        case '\n':
            //deal when in []
            switch (bfcode[codepos])
            {
            case '[':
                if(tape[tapepos]==0){
                    loopflag=1;
                    codep=&bfcode[codepos];
                    while(loopflag!=0){
                        codep=codep+1;
                        if(*codep=='[')
                            loopflag++;
                        if(*codep==']')
                            loopflag--;
                    }
                    codepos=codep-bfcode+1;
                }
                else{
                    codepos++;
                }
                break;
            
            case ']':
                if(tape[tapepos]==0){
                    loopflag=1;
                    codep=&bfcode[codepos];
                    while(loopflag!=0){
                        codep=codep-1;
                        if(*codep=='[')
                            loopflag++;
                        if(*codep==']')
                            loopflag--;
                    }
                    codepos=codep-bfcode+1;
                }
                tapepos++;
                break;
            
            default:
                codepos++;
                break;
            }
            while(bfcode[codepos]!='+' && bfcode[codepos]!='-' && bfcode[codepos]!='<' && bfcode[codepos]!='>' && bfcode[codepos]!=',' && bfcode[codepos]!='.' && bfcode[codepos]!='[' && bfcode[codepos]!=']'){
                codepos++;
                if(bfcode+codepos==codepend){
                    break;
                }
            }
            break;
        
        default:
            break;
        }
        fflush(stdin);
        //continue, forward, backward, exit, run, change tape data format, change tape, handle lack of input
        Sleep(500);
        if(bfcode+codepos>=codepend){
            break;
        }
    }

    return 0;
}