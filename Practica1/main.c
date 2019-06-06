#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

typedef enum { false, true } bool;
struct stat st;

struct part{
    int status;
    char type;
    char fit;
    int start;
    int size;
    char name[16];
};

struct mbr{
    int size;
    time_t time;
    int id;
    char fit;
    struct part partition[4];
};


bool flag=true;
char cmd[256];
char err[256];
char dat[256];

void strip(char *str, int size );
int analize(char *str);
void limpiar(char temp[256]);
int crear_disco(int size, char fit, int unit, char path[256]);

int main(){
    while(flag){
        printf("$~:");
        fgets(cmd,256,stdin);
        start(cmd);
    }
    return 0;
}
void start(char comand[256]){

    strip(comand,256);
    if(analize(comand)==1){
        printf("error en comando: '%s', motivo: '%s'.\n",err,dat);
    }
    limpiar(comand);
}
void strip(char *str, int size ){
    for(int i=0;i<size;++i){
        if(str[i]=='\n'){
            str[i]='\0';
            return;
        }
    }
}

int analize(char *str){
    char delim[] =" :~\n";
    char *ptr=strtok(str,delim);
    char strs[20][256];
    int j=0;
    while(ptr != NULL){
        strcpy(strs[j],ptr);
        if(strs[j][0]=='"'){
            ptr=strtok(NULL,delim);
            strcat(strs[j]," ");
            strcat(strs[j],ptr);
        }
        ptr=strtok(NULL,delim);
        j++;
    }
    for(int i=0;i<j;i++){
        if(strs[i][0]!='"' && strs[i][0]!='/')
            minusculas(strs[i]);
    }
    return execute(strs,(j-1)/2);
}

int execute(char strs[20][256], int j){
    if(strcmp(strs[0],"mkdisk")==0){//crear disco
        int size=0;
        char fit='F';
        int unit=1;
        char path[256];
        for(int i=1;i<j+1;i++){
            if(strcmp(strs[2*i-1],"-size")==0){
                size=strtol(strs[2*i], NULL,10);
                if(size<=0) return errR(strs[0],"size not valid");
            }
            else if(strcmp(strs[2*i-1],"-fit")==0){
                if(strcmp(strs[2*i],"bf")==0) fit='B';
                else if(strcmp(strs[2*i],"ff")==0) fit='F';
                else if(strcmp(strs[2*i],"wf")==0) fit='W';
                else return errR(strs[0],"fit not valid");
            }
            else if(strcmp(strs[2*i-1],"-unit")==0){
                if(strcmp(strs[2*i],"k")==0) unit=0;
                else if(strcmp(strs[2*i],"m")==0) unit=1;
                else return errR(strs[0],"unit not valid");
            }
            else if(strcmp(strs[2*i-1],"-path")==0){
                strcpy(path, strs[2*i]);
            }
        }
        if(size==0) return errR(strs[0],"size not valid");
        if(path[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path);
        //printf("%i|%c|%i|%s\n",size,fit,unit,aux);
        return crear_disco(size,fit,unit,aux);

    }
    else if(strcmp(strs[0],"rmdisk")==0){//eliminar disco

    }
    else if(strcmp(strs[0],"fdisk")==0){//particiones disco

    }
    else if(strcmp(strs[0],"mount")==0){//montar disco

    }
    else if(strcmp(strs[0],"unmount")==0){//desmontar disco

    }
    else if(strcmp(strs[0],"pause")==0){//pequeña pausa

    }
    else if(strcmp(strs[0],"rep")==0){//reporte de discos

    }
    else if(strcmp(strs[0],"exec")==0){//ejecutar archivo de entrada
        if(strcmp(strs[1],"-path")==0){
            char dir[256];
            quitar(dir, strs[2]);
            FILE *entrada;
            if((entrada=fopen(dir,"rt"))!=NULL){

                char comando[256];
                fgets(comando,256,entrada);
                while(!feof(entrada)){
                    repla(comando);
                    printf("    %s",comando);
                    start(comando);
                    limpiar(comando);
                    fgets(comando,256,entrada);
                }
                fclose(entrada);
            }
            else {
                if(entrada !=NULL) fclose(entrada);
                return errR(strs[0],"path of file not found");
            }
        }
        else return errR(strs[0],"parameter not found");
    }
    else return errR(strs[0],"command not found");
    return 0;
}

void minusculas(char *s){
    while(*s!=NULL){
        if(*s>='A' && *s<='Z')
            *s+=(char)32;
        s++;
    }
}
int errR(char *strs,char *error){
    strcpy(err,strs);
    strcpy(dat,error);
    return 1;
}
void limpiar(char temp[256]){
    for(int i=0;i<256;i++)
        temp[i]=0x0;
}
void quitar(char word2[256],char word[256]){
    char temp[256];
    if(word[0]=='\''){
        strcpy(temp,&word[1]);
        strncpy(word2,temp,strlen(temp)-1);
        word2[strlen(temp)-1]=0;
    }
    else
        strcpy(word2,word);
}
void repla(char w[256]){
    for(int i=0;i<strlen(w);i++){
        if(w[i]=='\"') w[i]='\'';
    }
}
FILE* fopen_dir(char path[256]){
    for(int i=1;i<strlen(path);i++){
        if(path[i]=='/'){
            char cha[256];
            strncpy(cha,path,i);
            cha[i]=0;
            if (stat(cha, &st) == -1) {
                mkdir(cha, 0700);
            }
        }
    }
    return fopen(path,"wb");
}
int crear_disco(int size, char fit, int unit, char path[256]){
    FILE *disco;
    if((disco=fopen_dir(path))==NULL){
        if(disco !=NULL) fclose(disco);
        return errR("mkdisk","cant create disk");
    }
    int mult=1;
    if(unit==1) mult=1000;
    char UwU[size*1000*mult];
    for(int i=0;i<size*1000*mult;i++)
        UwU[i]=0x0;
    fwrite(UwU,1,sizeof(UwU),disco);
    fclose(disco);


    printf("****************************** DISCO CREADO ******************************\n\n");
    return 0;
}



