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
    char *time;
    int id;
    char fit;
    struct part partition[4];
};

struct ebr{
    int status;
    char fit;
    int start;
    int size;
    int next;
    char name[16];
};

bool flag=true;
char cmd[256];
char err[256];
char dat[256];

void strip(char *str, int size );
int analize(char *str);
void limpiar(char temp[256]);
int crear_disco(int size, char fit, int unit, char path[256]);
int crear_part(int size, char fit, int unit, char type, char path[256], char name[256]);

int main(){
    while(flag){
        printf("$~:");
        gets(cmd,256,stdin);
        repla(cmd);
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
    for(int i=0;i<20;i++) limpiar(strs[i]);
    while(ptr != NULL){
        strcpy(strs[j],ptr);
        if(strs[j][0]==39){
            ptr=strtok(NULL,delim);
            if(ptr==NULL){
                j++;break;
            }
            strcat(strs[j]," ");
            strcat(strs[j],ptr);
        }
        ptr=strtok(NULL,delim);
        j++;
    }
    for(int i=0;i<j;i++){
        if(strs[i][0]!='\'' && strs[i][0]!='/')
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
        if(strcmp(strs[1],"-path")==0){
            char dir[256];
            quitar(dir, strs[2]);
            int status;
            for(int i=1;i<strlen(dir);i++)
                if(dir[i]==' ') dir[i]='_';
            if (stat(dir, &st) == -1)
                status=remove(dir);
            if(status==0){
                printf("****************************** DISCO ELIMINADO ******************************\n\n");
            }
            else return errR(strs[0],"path of file not found");
        }
        else return errR(strs[0],"parameter not found");
    }
    else if(strcmp(strs[0],"fdisk")==0){//particiones disco
        int size=0;
        int unit=1;
        char path[256];
        char type='P';
        char fit='W';
        char del[5];
        char name[256];
        int add=0;
        char exec='c';
        for(int i=1;i<j+1;i++){
            if(strcmp(strs[2*i-1],"-size")==0){
                size=strtol(strs[2*i], NULL,10);
                if(size<=0) return errR(strs[0],"size not valid");
            }
            else if(strcmp(strs[2*i-1],"-unit")==0){
                if(strcmp(strs[2*i],"b")==0) unit=0;
                else if(strcmp(strs[2*i],"k")==0) unit=1;
                else if(strcmp(strs[2*i],"m")==0) unit=2;
                else return errR(strs[0],"unit not valid");
            }
            else if(strcmp(strs[2*i-1],"-path")==0){
                strcpy(path, strs[2*i]);
            }
            else if(strcmp(strs[2*i-1],"-type")==0){
                if(strcmp(strs[2*i],"p")==0) type='P';
                else if(strcmp(strs[2*i],"e")==0) type='E';
                else if(strcmp(strs[2*i],"l")==0) type='L';
                else return errR(strs[0],"type not valid");
            }
            else if(strcmp(strs[2*i-1],"-fit")==0){
                if(strcmp(strs[2*i],"bf")==0) fit='B';
                else if(strcmp(strs[2*i],"ff")==0) fit='F';
                else if(strcmp(strs[2*i],"wf")==0) fit='W';
                else return errR(strs[0],"fit not valid");
            }
            else if(strcmp(strs[2*i-1],"-delete")==0){
                exec='d';
                if(strcmp(strs[2*i],"fast")==0) strcpy(del,"fast");
                else if(strcmp(strs[2*i],"full")==0) strcpy(del,"full");
                else return errR(strs[0],"fit not valid");
            }
            else if(strcmp(strs[2*i-1],"-name")==0){
                strcpy(name, strs[2*i]);
            }
            else if(strcmp(strs[2*i-1],"-add")==0){
                exec='a';
                add=strtol(strs[2*i], NULL,10);
            }
            else {
                printf("%s\n",strs[2*i-1]);
                return errR(strs[0],"parameter not found");
            }
        }
        if(path[0]==0) return errR(strs[0],"path is not valid");
        if(name[0]==0) return errR(strs[0],"path is not valid");
        char aux_p[256];
        quitar(aux_p, path);
        repla2(aux_p);
        char aux_n[256];
        quitar(aux_n, name);
        if(exec=='c'){
            printf("%i|%c|%i|%c|%s|%s\n",size, fit, unit, type, aux_p, aux_n);
            return crear_part(size, fit, unit, type, aux_p, aux_n);
        }
        else if(exec=='a'){
            printf("%i|%i|%s|%s\n",add, unit, aux_p, aux_n);
            return add_part(add, unit, aux_p, aux_n);
        }
        else if(exec=='d'){
            printf("%s|%s|%s\n",del, aux_p, aux_n);
            return del_part(del, aux_p, aux_n);
        }
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
                    if(comando[0]!='\n' && comando[0]!='#'){
                        repla(comando);
                        printf("    %s",comando);
                        start(comando);
                    }
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
void repla2(char w[256]){
    for(int i=0;i<strlen(w);i++){
        if(w[i]=='\ ') w[i]='_';
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
        if(path[i]==' ') path[i]='_';
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

    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';

    struct mbr mabore;
    mabore.size=size*1000*mult;
    mabore.time=time_str;
    mabore.fit=fit;
    mabore.id=(rand()%1000)+1;

    printf("*********MBR**********\n");
    printf("size: %i bytes\n",mabore.size);
    printf("time: %s \n",mabore.time);
    printf("fit: %c \n",mabore.fit);
    printf("id: %i \n",mabore.id);

    fseek(disco,0,SEEK_SET);
    fwrite(&mabore,sizeof(mabore),1,disco);
    fclose(disco);

    printf("****************************** DISCO CREADO ******************************\n\n");
    return 0;
}
int crear_part(int size, char fit, int unit, char type, char path[256], char name[256]){
    FILE *fich;
    struct mbr mabore;
    for(int i=1;i<strlen(path);i++)
        if(path[i]==' ') path[i]='_';
    if((fich = fopen(path,"rb"))==NULL){
        if(fich !=NULL) fclose(fich);
        return errR("fdisk","cant open disk");
    }
    fread(&mabore,sizeof(mabore),1,fich);
    struct part particion;
    particion.status=1;
    particion.type=type;
    particion.fit=fit;
    particion.start=get_pos(particion,mabore);
    particion.size=size;
    strcpy(particion.name,name);
    fclose(fich);
    return 0;
}
int get_pos(struct part particion,struct mbr mabore){
    if(mabore.partition[0].status==0){
        mabore.partition[0]=particion;
        return sizeof(mabore);
    }
    else if(mabore.partition[1].status==0){

    }
}
int add_part(int add, int unit, char path[256], char name[256]){

    return 0;
}
int del_part(char del[4], char path[256], char name[256]){

    return 0;
}


