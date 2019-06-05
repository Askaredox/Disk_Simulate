#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { false, true } bool;

bool flag=true;
char cmd[256];

void strip(char *str, int size );
int analize(char *str);

int main(){
    while(flag){
        printf("$~:");
        fgets(cmd,256,stdin);
        strip(cmd,256);
        //printf("\n");
        //printf("%s\n\n",cmd);
        if(analize(cmd)==1){
            printf("error en comando");
        }
    }
    return 0;
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
    char delim[] =" :~";
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
        minusculas(strs[i]);
    }
    return execute(strs,j);
}

int execute(char *strs[], int j){
    if(strcmp(strs,"mkdisk")==0){//crear disco
        return 0;
    }
    else if(strcmp(strs,"rmdisk")==0){//eliminar disco
        return 0;
    }
    else if(strcmp(strs,"fdisk")==0){//particiones disco
        return 0;
    }
    else if(strcmp(strs,"mount")==0){//montar disco
        return 0;
    }
    else if(strcmp(strs,"unmount")==0){//desmontar disco
        return 0;
    }
    else if(strcmp(strs,"pause")==0){//pequeña pausa
        return 0;
    }
    else if(strcmp(strs,"rep")==0){//reporte de discos
        return 0;
    }
    else if(strcmp(strs,"exec")==0){//ejecutar archivo de entrada
        return 0;
    }
    else
        return 1;
}

void minusculas(char *s){
    while(*s!=NULL){
        if(*s>='A' && *s<='Z')
            *s+=(char)32;
        s++;
    }
}


