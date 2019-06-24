#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

typedef enum { false, true } bool;
struct stat st;

struct Montaje{
    char path[256];
    char id[4][5];
    char part[4][256];
} mon[200];
struct Part{
    int status;
    bool mounted;
    char type;
    char fit;
    int start;
    int size;
    char name[16];
    int tt;
};
struct mbr{
    int size;
    char time[30];
    int id;
    char fit;
    struct Part partition[5];
};
struct ebr{
    int status;
    char fit;
    int start;
    int size;
    int next;
    char name[16];
};

struct Super_Block{//80bytes
    int inode_count;
    int block_count;
    int free_block_count;
    int free_inode_count;
    time_t time_m;
    time_t time_um;
    int m_count;
    int magic;
    int inode_size;
    int block_size;
    int first_inode;
    int first_block;
    int bm_inode_start;
    int bm_block_start;
    int inode_start;
    int block_start;
    int n;
    int start;
};
struct Journaling{//224
    int operation;
    int tipo;
    char name[150];
    int content;
    time_t time;
    int uid;
    int gid;
    int permisos;
};
struct Inode{//112
    int uid;
    int gid;
    int size_file;
    time_t r_time;
    time_t c_time;
    time_t w_time;
    int block[15];
    char type;
    int perm;
};
struct Content{
    char name[12];
    int inode;
};
struct Block_dir{//64
    struct Content cont[4];
};
struct Block_file{//64
    char cont[64];
};
struct Block_point{//64
    int pointer[16];
};
struct User{
    int UID;
    int GID;
    char path[255];
    char part[255];
}usuario={0,0,"",""};

bool flag=true;
char cmd[256];
char err[256];
char dat[256];
int mon_num=0;
int counter=0;
char *ls="";

void start(char comand[256]);
void strip(char *str, int size );
int analize(char *str);
int execute(char strs[20][256], int j);
void minusculas(char *s);
int errR(char *strs,char *error);
void limpiar(char temp[256]);
void quitar(char word2[256],char word[256]);
void repla(char w[256]);
void repla2(char w[256]);
FILE* fopen_dir(char path[256]);
FILE* fopen_dir_txt(char path[256]);
int crear_disco(int size, char fit, int unit, char path[256]);
int crear_part(int size, char fit, int unit, char type, char path[256], char name[256]);
int set_pos(struct Part *particion,struct mbr *mabore);
int add_part(int add, int unit, char path[256], char name[256]);
int del_part(char del[4], char path[256], char name[256]);
void escribir_mabore(struct mbr *mabore,FILE *fichero);
void do_mbr_graph(struct mbr *mabore,FILE *rep,char text[256],char png[256], char path[256]);
char* concat(const char *s1, const char *s2);
void cl(char temp[20]);
void do_disk_graph(struct mbr *mabore,FILE *rep,char text[256],char png[256],char path[256]);
int put_ebr(struct ebr *exbore, FILE *fich,struct Part *particion);
char* read_file(char str[40][12], char *name, int *puntero, int *dirs, int directorio, struct Super_Block *sb, FILE *fich, bool let);
int crear_carpeta_i(struct Inode *inodo, bool p, int uid, int gid, char name[12], char *path,struct Super_Block *sb, FILE *fich);
int crear_carpeta(bool p, int uid, int gid, char name[12], char str[40][12], int *puntero, int *dirs, int directorio, struct Super_Block *sb, FILE *fich, int padre, bool let);
void update_dir(struct Block_dir *dir, int num,struct Super_Block *sb,FILE *fich);
char* leer(struct Block_dir *carpeta,char *name,int directorio, struct Super_Block *sb, FILE *fich, bool let);
bool permiso(int owner, int group,int permiso,int get);
char* read_file_i(struct Inode *inodo, char *path,struct Super_Block *sb, FILE *fich);

int MKDISK(char strs[20][256], int j);
int RMDISK(char strs[20][256]);
int FDISK(char strs[20][256], int j);
int MOUNT(char strs[20][256], int j);
int UNMOUNT(char strs[20][256]);
int PAUSE();
int REP(char strs[20][256], int j);
int EXEC(char strs[20][256]);

/**********************************************************************************************************************************************************/
int main(){
    for(int i=0;i<200;i++){
        for(int j=0;j<4;j++){
            limpiar(mon[i].part[j]);
            for(int k=0;k<5;k++)
                mon[i].id[j][k]=0;
        }
        limpiar(mon[i].path);
    }
    while(flag){
        printf("$~:");
        fgets(cmd,256,stdin);
        repla(cmd);
        start(cmd);
    }
    return 0;
}
void start(char comand[256]){
    strip(comand,256);
    if(analize(comand)==1){
        printf("    error en comando: '%s', motivo: '%s'.\n",err,dat);
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
    while(ptr != NULL && *ptr!='#'){
        strcpy(strs[j],ptr);
        int temp=strlen(strs[j]);
        if(strs[j][0]==39 && strs[j][temp-1]!=39){
            while(strs[j][temp-1]!=39){
                ptr=strtok(NULL,delim);
                strcat(strs[j]," ");
                strcat(strs[j],ptr);
                temp=strlen(strs[j]);
            }
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
    if(strcmp(strs[0],"mkdisk")==0)         return MKDISK(strs,j);  //make disk
    else if(strcmp(strs[0],"rmdisk")==0)    return RMDISK(strs);    //remove disk
    else if(strcmp(strs[0],"fdisk")==0)     return FDISK(strs,j);   //edit disk
    else if(strcmp(strs[0],"mount")==0)     return MOUNT(strs,j);   ///montar particion *no monta logicas*
    else if(strcmp(strs[0],"unmount")==0)   return UNMOUNT(strs);   ///desmonar particion *no desmonta logicas*
    else if(strcmp(strs[0],"pause")==0)     return PAUSE();         //pausa enter key
    else if(strcmp(strs[0],"rep")==0)       return REP(strs,j);     //reportes
    else if(strcmp(strs[0],"exec")==0)      return EXEC(strs);      //ejecutar archivo de entrada
    else if(strcmp(strs[0],"mkfs")==0)      return MKFS(strs,j);    //make format
    else if(strcmp(strs[0],"login")==0)     return LOGIN(strs,j);   //login
    else if(strcmp(strs[0],"logout")==0)    return LOGOUT();        //logout
    else if(strcmp(strs[0],"mkgrp")==0)     return MKGRP(strs,j);   //make group
    else if(strcmp(strs[0],"mkusr")==0)     return MKUSR(strs,j);   //make user
    else if(strcmp(strs[0],"mkdir")==0)     return MKDIR(strs,j);   ///make directory *solo crea hasta primer indirecto*
    else if(strcmp(strs[0],"mkfile")==0)    return MKFILE(strs,j);  ///make file *tamaño máximo de contenido de archivo 100000bytes*
    else if(strcmp(strs[0],"rem")==0)       return REM(strs,j);     ///remove a file or directory *error a la hora de de eliminacion recursiva con indirectos*
    else if(strcmp(strs[0],"mv")==0)        return MV(strs,j);      //move a file or directory
    else if(strcmp(strs[0],"loss")==0)      return LOSS(strs,j);    //simule a destruction of the disk
    else if(strcmp(strs[0],"recovery")==0)  return RECOVERY(strs,j);//recovery of the disk
    else if(strcmp(strs[0],"exit")==0)      exit(0);
    else                                    return errR(strs[0],"command not found");
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
    for(unsigned int i=0;i<strlen(w);i++){
        if(w[i]=='\"') w[i]='\'';
    }
}
void repla2(char w[256]){
    for(unsigned int i=0;i<strlen(w);i++){
        if(w[i]==32) w[i]='_';
    }
}
FILE* fopen_dir(char path[256]){
    for(unsigned int i=1;i<strlen(path);i++){
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
FILE* fopen_dir_txt(char path[256]){
    for(unsigned int i=1;i<strlen(path);i++){
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
    return fopen(path,"wt");
}
int crear_disco(int size, char fit, int unit, char path[256]){

    FILE *disco;
    if((disco=fopen_dir(path))==NULL){
        if(disco !=NULL) fclose(disco);
        return errR("mkdisk","cant create disk");
    }
    int mult=1;
    if(unit==1) mult=1024;

    int UwU=size*1024*mult;
    fseek(disco,UwU,SEEK_SET);
    fputc(0,disco);

    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';

    struct mbr mabore;
    mabore.size=size*1024*mult;
    strcpy(mabore.time,time_str);
    mabore.fit=fit;
    mabore.id=(rand()%99999999)+1;

    printf("*********MBR**********\n");
    printf("size: %i bytes\n",mabore.size);
    printf("time: %s \n",mabore.time);
    printf("fit: %c \n",mabore.fit);
    printf("id: %i \n",mabore.id);

    escribir_mabore(&mabore,disco);
    fclose(disco);

    printf("****************************** DISCO CREADO ******************************\n\n");
    return 0;
}
int crear_part(int size, char fit, int unit, char type, char path[256], char name[256]){
    FILE *fich;
    struct mbr mabore;
    for(unsigned int i=1;i<strlen(path);i++)
        if(path[i]==' ') path[i]='_';
    if((fich = fopen(path,"rb+"))==NULL){
        if(fich !=NULL) fclose(fich);
        return errR("fdisk","cant open disk");
    }
    fread(&mabore,sizeof(mabore),1,fich);
    int temp;
    if(unit==0) temp=1;
    else if(unit==1)temp=1024;
    else if(unit==2)temp=1024*1024;
    if(type=='L'){
        for(int i=0;i<4;i++){
            if(mabore.partition[i].type=='E'){
                struct ebr exbore;
                exbore.fit=fit;
                strcpy(exbore.name,name);
                exbore.size=size*temp;
                exbore.status=1;
                mabore.partition[i].tt+=2;
                put_ebr(&exbore,fich,&mabore.partition[i]);
                break;
            }
        }
    }
    else{
        struct Part particion;
        particion.status=1;
        particion.type=type;
        particion.fit=fit;
        particion.mounted=false;
        particion.size=size*temp;
        if(particion.type=='E'){
            for(int i=0;i<4;i++){
                if(mabore.partition[i].type=='E')
                    return errR("fdisk","already an Expanded partition allocated");
            }
        }
        if(mabore.partition[3].status==1)
            return errR("fdisk","already an 4 partitions allocated");
        strcpy(particion.name,name);
        if(set_pos(&particion,&mabore)!=0){
            return 1;
        }
        if(particion.type=='E'){
            struct ebr exbore;
            exbore.fit='0';
            strcpy(exbore.name,"LIBRE");
            exbore.next=-1;
            exbore.size=(particion.size)-sizeof(exbore);
            exbore.start=particion.start;
            exbore.status=0;
            for(int i=0;i<4;i++)
                if(mabore.partition[i].type=='E') {
                    mabore.partition[i].tt=2;
                    break;
                }
            fseek(fich,particion.start,SEEK_SET);
            fwrite(&exbore,sizeof(exbore),1,fich);
        }
        //printf("fit: %c \n",mabore.fit);
        //printf("id: %i \n",mabore.id);
    }
    escribir_mabore(&mabore,fich);

    fclose(fich);
    return 0;
}
int set_pos(struct Part *particion,struct mbr *mabore){
    int size_ant=sizeof(*mabore);
    if(mabore->fit=='F'){
        for(int i=0;i<5;i++){
            if(mabore->partition[i].status!=0){
                if(mabore->partition[i].start-size_ant>particion->size){
                    particion->start=size_ant;
                    mabore->partition[4]=*particion;
                    break;
                }
                else{
                    size_ant=mabore->partition[i].start+mabore->partition[i].size;
                }
            }
            else{
                if(mabore->size-size_ant>particion->size){
                    particion->start=size_ant;
                    mabore->partition[4]=*particion;
                    break;
                }
                else{
                    return errR("fdisk","not enough space in disk");
                }
            }
        }
    }
    else if(mabore->fit=='W'){
        int space=0;
        int start_space=size_ant;
        for(int i=0;i<5;i++){
            if(mabore->partition[i].status!=0){
                if(mabore->partition[i].start-size_ant>particion->size && space<mabore->partition[i].start-size_ant){
                    space=mabore->partition[i].start-size_ant;
                    start_space=size_ant;
                }
                size_ant=mabore->partition[i].start+mabore->partition[i].size;
            }
            else{
                if(mabore->size-size_ant>particion->size && space<mabore->size-size_ant){
                    space=mabore->size-size_ant;
                    start_space=size_ant;
                }
            }
        }
        if(space!=0){
            particion->start=start_space;
            mabore->partition[4]=*particion;
        }
        else return errR("fdisk","not enough space in disk");
    }
    else if(mabore->fit=='B'){
        int space=0x7fffffff;
        int start_space=size_ant;
        for(int i=0;i<5;i++){
            if(mabore->partition[i].status!=0){
                if(mabore->partition[i].start-size_ant>particion->size && space>mabore->partition[i].start-size_ant){
                    space=mabore->partition[i].start-size_ant;
                    start_space=size_ant;
                }
                size_ant=mabore->partition[i].start+mabore->partition[i].size;
            }
            else{
                if(mabore->size-size_ant>particion->size && space>mabore->size-size_ant){
                    space=mabore->size-size_ant;
                    start_space=size_ant;
                }
            }
        }
        if(space!=0x7fffffff){
            particion->start=start_space;
            mabore->partition[4]=*particion;
        }
        else return errR("fdisk","not enough space in disk");
    }
    struct Part k;
    for(int i=1;i<5;i++){
        for(int j=0;j<4;j++){
            if((mabore->partition[j].start==0 && mabore->partition[j+1].start!=0)||mabore->partition[j].start>mabore->partition[j+1].start){
                if(mabore->partition[j+1].start!=0){
                    k=mabore->partition[j+1]; mabore->partition[j+1]=mabore->partition[j]; mabore->partition[j]=k;
                }
            }
        }
    }
    return 0;
}
int add_part(int add, int unit, char path[256], char name[256]){
    FILE *fich;
    struct mbr mabore;
    for(unsigned int i=1;i<strlen(path);i++)
        if(path[i]==' ') path[i]='_';
    if((fich = fopen(path,"rb+"))==NULL){
        if(fich !=NULL) fclose(fich);
        return errR("fdisk","cant open disk");
    }
    fread(&mabore,sizeof(mabore),1,fich);
    int temp;
    if(unit==0) temp=1;
    else if(unit==1)temp=1024;
    else if(unit==2)temp=1024*1024;

    if(add<0){//quitar memoria a particion
        for(int i=0;i<4;i++){
            if(strcmp(mabore.partition[i].name,name)==0){
                if(mabore.partition[i].size+(add*temp)>0){
                    mabore.partition[i].size+=(add*temp);
                    break;
                }
                else errR("fdisk","cant remove that amount of memory");
            }
            if(i==3){
                for(int j=0;j<4;j++){
                    if(mabore.partition[j].type=='E'){
                        struct ebr temp_;
                        fseek(fich,mabore.partition[j].start,SEEK_SET);
                        fread(&temp_,sizeof(temp_),1,fich);
                        int ss=temp_.next;
                        while(ss!=-1){
                            if(strcmp(temp_.name,name)==0){
                                if(temp_.size+(add*temp)>0){

                                    temp_.size+=(add*temp);
                                    int nex=temp_.next;
                                    temp_.next=temp_.start+temp_.size;
                                    fseek(fich,temp_.start,SEEK_SET);
                                    fwrite(&temp_,sizeof(temp_),1,fich);

                                    struct ebr exbore;
                                    exbore.fit='0';
                                    strcpy(exbore.name,"LIBRE");
                                    exbore.next=nex;
                                    exbore.start=temp_.next;
                                    exbore.size=-(add*temp);
                                    exbore.status=0;
                                    for(int i=0;i<4;i++)
                                        if(mabore.partition[i].type=='E') {
                                            mabore.partition[i].tt=2;
                                            break;
                                        }
                                    fseek(fich,exbore.start,SEEK_SET);
                                    fwrite(&exbore,sizeof(exbore),1,fich);

                                    fclose(fich);
                                    return 0;
                                }
                                else errR("fdisk","cant remove that amount of memory");
                            }
                            else{
                                fseek(fich,ss,SEEK_SET);
                                fread(&temp_,sizeof(temp_),1,fich);
                                ss=temp_.next;
                            }
                        }
                    }
                }
                return errR("fdisk","no such partition exists");
            }
        }
    }
    else{//agregar memoria
        for(int i=0;i<4;i++){
            if(strcmp(mabore.partition[i].name,name)==0){
                int temporal=0;
                if(mabore.partition[i+1].status!=0)
                    temporal=mabore.partition[i+1].start;
                else
                    temporal=mabore.size;
                if(mabore.partition[i].size+add*temp<temporal){
                    mabore.partition[i].size+=(add*temp);
                    break;
                }
                if(i==3){
                    for(int j=0;j<4;j++){
                        if(mabore.partition[j].type=='E'){
                            struct ebr temp_;
                            fseek(fich,mabore.partition[j].start,SEEK_SET);
                            fread(&temp_,sizeof(temp_),1,fich);
                            int ss=temp_.next;
                            while(ss!=-1){
                                if(strcmp(temp_.name,name)==0){
                                    int espacio=0;
                                    struct ebr temp_2;
                                    fseek(fich,temp_.next,SEEK_SET);
                                    fread(&temp_2,sizeof(temp_2),1,fich);

                                    if(temp_2.status!=-1){
                                        espacio=temp_2.start;
                                    }
                                    else{
                                        espacio=mabore.partition[j].start+mabore.partition[j].size;
                                    }
                                    if(temp_.size+(add*temp)<espacio){
                                        temp_.size+=(add*temp);
                                        fseek(fich,temp_.start,SEEK_SET);
                                        fwrite(&temp_,sizeof(temp_),1,fich);
                                        fclose(fich);
                                        return 0;
                                    }
                                    else errR("fdisk","cant remove that amount of memory");
                                }
                                else{
                                    fseek(fich,ss,SEEK_SET);
                                    fread(&temp_,sizeof(temp_),1,fich);
                                    ss=temp_.next;
                                }
                            }
                        }
                    }
                    return errR("fdisk","no such partition exists");
                }
            }
        }
    }
    return 0;
}
int del_part(char del[4], char path[256], char name[256]){
    FILE *fich;
    struct mbr mabore;
    for(unsigned int i=1;i<strlen(path);i++)
        if(path[i]==' ') path[i]='_';
    if((fich = fopen(path,"rb+"))==NULL){
        if(fich !=NULL) fclose(fich);
        return errR("fdisk","cant open disk");
    }
    fread(&mabore,sizeof(mabore),1,fich);
    if(strcmp(del,"full")==0){
        for(int i=0;i<4;i++){
            if(strcmp(mabore.partition[i].name,name)==0){
                int init=mabore.partition[i].start;
                int tam=mabore.partition[i].size;

                rewind(fich);
                fseek(fich,init,SEEK_CUR);
                for(int i=0;i<tam;i++){
                    fputc(0,fich);
                    fseek(fich,1,SEEK_CUR);
                }

                mabore.partition[i].status=0;
                mabore.partition[i].start=0;
                mabore.partition[i].fit=0;
                mabore.partition[i].size=0;
                mabore.partition[i].type=0;
                for(int j=0;j<16;j++)
                    mabore.partition[i].name[j]=0;
                struct Part k;
                for(int i=1;i<5;i++){
                    for(int j=0;j<4;j++){
                        if((mabore.partition[j].start==0 && mabore.partition[j+1].start!=0)||mabore.partition[j].start>mabore.partition[j+1].start){
                            if(mabore.partition[j+1].start!=0){
                                k=mabore.partition[j+1]; mabore.partition[j+1]=mabore.partition[j]; mabore.partition[j]=k;
                            }
                        }
                    }
                }
                break;
            }
            if(i==3){
                for(int j=0;j<4;j++){
                    if(mabore.partition[j].type=='E'){
                        struct ebr temp;
                        struct ebr primero;
                        struct ebr segundo;
                        fseek(fich,mabore.partition[j].start,SEEK_SET);
                        fread(&temp,sizeof(temp),1,fich);
                        segundo=temp;
                        primero.status=3;
                        int ss=temp.next;
                        while(ss!=-1){
                            if(strcmp(temp.name,name)==0){



                                temp.fit=0;
                                strcpy(temp.name,"LIBRE");
                                temp.status=0;

                                fseek(fich,temp.start,SEEK_SET);
                                fwrite(&temp,sizeof(temp),1,fich);


                                while(segundo.next!=-1){
                                    if(primero.status==0 && segundo.status==0){
                                        primero.next=segundo.next;
                                        primero.size+=segundo.size;
                                        fseek(fich,primero.start,SEEK_SET);
                                        fwrite(&primero,sizeof(primero),1,fich);
                                    }
                                    primero=segundo;
                                    fseek(fich,segundo.next,SEEK_SET);
                                    fread(&segundo,sizeof(segundo),1,fich);
                                }
                                fclose(fich);
                                return 0;
                            }
                            else{
                                fseek(fich,ss,SEEK_SET);
                                fread(&temp,sizeof(temp),1,fich);
                                ss=temp.next;
                            }
                        }
                    }
                }
                return errR("fdisk","no such partition exists");
            }
        }
    }
    else{
        for(int ii=0;ii<4;ii++){
            if(strcmp(mabore.partition[ii].name,name)==0){
                mabore.partition[ii].status=0;
                mabore.partition[ii].start=0;
                mabore.partition[ii].fit=0;
                mabore.partition[ii].size=0;
                mabore.partition[ii].type=0;
                for(int j=0;j<16;j++)
                    mabore.partition[ii].name[j]=0;
                struct Part k;
                for(int i=1;i<4;i++){
                    for(int j=0;j<3;j++){
                        if((mabore.partition[j].start==0 && mabore.partition[j+1].start!=0)||mabore.partition[j].start>mabore.partition[j+1].start){
                            if(mabore.partition[j+1].start!=0){
                                k=mabore.partition[j+1]; mabore.partition[j+1]=mabore.partition[j]; mabore.partition[j]=k;
                            }
                        }
                    }
                }
                break;
            }
            if(ii==3){
                for(int j=0;j<4;j++){
                    if(mabore.partition[j].type=='E'){
                        struct ebr temp;
                        struct ebr primero;
                        struct ebr segundo;
                        fseek(fich,mabore.partition[j].start,SEEK_SET);
                        fread(&temp,sizeof(temp),1,fich);
                        segundo=temp;
                        primero.status=3;
                        int ss=temp.next;
                        while(ss!=-1){
                            if(strcmp(temp.name,name)==0){

                                temp.fit=0;
                                strcpy(temp.name,"LIBRE");
                                temp.status=0;

                                fseek(fich,temp.start,SEEK_SET);
                                fwrite(&temp,sizeof(temp),1,fich);

                                while(segundo.next!=-1){
                                    if(primero.status==0 && segundo.status==0){
                                        primero.next=segundo.next;
                                        primero.size+=segundo.size;
                                        fseek(fich,primero.start,SEEK_SET);
                                        fwrite(&primero,sizeof(primero),1,fich);
                                    }
                                    primero=segundo;
                                    fseek(fich,segundo.next,SEEK_SET);
                                    fread(&segundo,sizeof(segundo),1,fich);
                                }

                                fclose(fich);

                                return 0;
                            }
                            else{
                                fseek(fich,ss,SEEK_SET);
                                fread(&temp,sizeof(temp),1,fich);
                                ss=temp.next;
                            }
                        }
                    }
                }
                return errR("fdisk","no such partition exists");
            }

        }
    }
    escribir_mabore(&mabore,fich);
    fclose(fich);
    return 0;
}
void escribir_mabore(struct mbr *mabore,FILE *fichero){
    rewind(fichero);
    fwrite(mabore,sizeof(*mabore),1,fichero);
}
void do_mbr_graph(struct mbr *mabore,FILE *rep,char text[256],char png[256], char path[256]){
    char *graph="";
    char tempo[255];
    graph=concat(graph,"digraph html{\n");
    graph=concat(graph,"abc [shape=none, margin=0, label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");

    graph=concat(graph,"<TR><td>Nombre</td><td>Valor</td></TR>\n");

    graph=concat(graph,"<TR>\n");
    graph=concat(graph,"<Td>mbr_tamaño</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%d</Td>",mabore->size);graph=concat(graph,tempo);
    graph=concat(graph,"</TR>\n");

    graph=concat(graph,"<TR>\n");
    graph=concat(graph,"<Td>mbr_fecha_creacion</Td>\n<td>");
    graph=concat(graph,mabore->time);
    graph=concat(graph,"</td>\n</TR>\n");

    graph=concat(graph,"<TR>\n");
    graph=concat(graph,"<Td>mbr_disk_signature</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%d</Td>",mabore->id);graph=concat(graph,tempo);
    graph=concat(graph,"</TR>\n");

    graph=concat(graph,"<TR>\n");
    graph=concat(graph,"<Td>Disk_fit</Td>\n<td>");
    cl(tempo);tempo[0]=mabore->fit;graph=concat(graph,tempo);
    graph=concat(graph,"</td>\n</TR>\n");

    for(int i=0;i<4;i++){
        if(mabore->partition[i].name[0]!=0){
            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_status_");cl(tempo);sprintf(tempo,"%d</Td>\n",i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",mabore->partition[i].status);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_type_");cl(tempo);sprintf(tempo,"%d</Td>\n",i);graph=concat(graph,tempo);
            cl(tempo);strcpy(tempo,"<td>");tempo[4]=mabore->partition[i].type;graph=concat(graph,tempo);
            graph=concat(graph,"</td>\n</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_fit_");cl(tempo);sprintf(tempo,"%d</Td>\n",i);graph=concat(graph,tempo);
            cl(tempo);strcpy(tempo,"<td>");tempo[4]=mabore->partition[i].fit;graph=concat(graph,tempo);
            graph=concat(graph,"</td>\n</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_start_");cl(tempo);sprintf(tempo,"%d</Td>\n",i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",mabore->partition[i].start);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_size_");cl(tempo);sprintf(tempo,"%d</Td>\n",i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",mabore->partition[i].size);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_name_");cl(tempo);sprintf(tempo,"%d</Td>\n<td>",i);graph=concat(graph,tempo);
            graph=concat(graph,mabore->partition[i].name);
            graph=concat(graph,"</td>\n</TR>\n");
        }
    }
    graph=concat(graph,"</TABLE>\n>];\n\n");
    for(int i=0;i<4;i++){
        if(mabore->partition[i].type=='E'){
            FILE *fich;
            struct ebr exbore={0,0,0,0,0,""};
            for(unsigned int i=1;i<strlen(path);i++)
                if(path[i]==' ') path[i]='_';
            if((fich = fopen(path,"rb+"))==NULL){
                if(fich !=NULL) fclose(fich);
            }
            fseek(fich,mabore->partition[i].start,SEEK_SET);
            fread(&exbore,sizeof(exbore),1,fich);


            int ii=1;
            while(exbore.next!=-1){
                graph=concat(graph,"ebr_");cl(tempo);sprintf(tempo,"%d",ii);graph=concat(graph,tempo);
                graph=concat(graph," [shape=none, margin=0, label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");
                graph=concat(graph,"<TR><td>Nombre</td><td>Valor</td></TR>\n");
                graph=concat(graph,"<TR>\n");
                graph=concat(graph,"<Td>part_status_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.status);graph=concat(graph,tempo);
                graph=concat(graph,"</TR>\n");

                graph=concat(graph,"<TR>\n");
                graph=concat(graph,"<Td>part_fit_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
                cl(tempo);strcpy(tempo,"<td>");tempo[4]=exbore.fit;graph=concat(graph,tempo);
                graph=concat(graph,"</td>\n</TR>\n");

                graph=concat(graph,"<TR>\n");
                graph=concat(graph,"<Td>part_start_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.start);graph=concat(graph,tempo);
                graph=concat(graph,"</TR>\n");

                graph=concat(graph,"<TR>\n");
                graph=concat(graph,"<Td>part_size_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.size);graph=concat(graph,tempo);
                graph=concat(graph,"</TR>\n");

                graph=concat(graph,"<TR>\n");
                graph=concat(graph,"<Td>part_next_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.next);graph=concat(graph,tempo);
                graph=concat(graph,"</TR>\n");

                graph=concat(graph,"<TR>\n");
                graph=concat(graph,"<Td>part_name_");cl(tempo);sprintf(tempo,"%d</Td>\n<td>",ii);graph=concat(graph,tempo);
                graph=concat(graph,exbore.name);
                graph=concat(graph,"</td>\n</TR>\n");

                graph=concat(graph,"</TABLE>\n>];\n");

                fseek(fich,exbore.next,SEEK_SET);
                fread(&exbore,sizeof(exbore),1,fich);
                ii++;
            }
            graph=concat(graph,"ebr_");cl(tempo);sprintf(tempo,"%d",ii);graph=concat(graph,tempo);
            graph=concat(graph," [shape=none, margin=0, label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");
            graph=concat(graph,"<TR><td>Nombre</td><td>Valor</td></TR>\n");
            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_status_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.status);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_fit_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
            cl(tempo);strcpy(tempo,"<td>");tempo[4]=exbore.fit;graph=concat(graph,tempo);
            graph=concat(graph,"</td>\n</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_start_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.start);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_size_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.size);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_next_");cl(tempo);sprintf(tempo,"%d</Td>\n",ii);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<Td>%d</Td>",exbore.next);graph=concat(graph,tempo);
            graph=concat(graph,"</TR>\n");

            graph=concat(graph,"<TR>\n");
            graph=concat(graph,"<Td>part_name_");cl(tempo);sprintf(tempo,"%d</Td>\n<td>",ii);graph=concat(graph,tempo);
            graph=concat(graph,exbore.name);
            graph=concat(graph,"</td>\n</TR>\n");

            graph=concat(graph,"</TABLE>\n>];\n");

            fclose(fich);
            break;
        }
    }
    graph=concat(graph,"}");

    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    char temp[256];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
void do_disk_graph(struct mbr *mabore,FILE *rep,char text[256],char png[256],char path[256]){
    char *graph="";
    char tempo[255];
    const float total=mabore->size;
    float percent;
    graph=concat(graph,"digraph html{\n");
    graph=concat(graph,"abc [shape=none, margin=0, label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");
    graph=concat(graph,"<tr>\n");

    graph=concat(graph,"<td rowspan=\"2\">MBR<br/>");
    float b=sizeof(*mabore);
    percent=(b/total)*100.0;
    cl(tempo);sprintf(tempo,"%f%% del disco",percent);graph=concat(graph,tempo);
    graph=concat(graph,"</td>\n");
    for(int i=0;i<4;i++){
        if(mabore->partition[i].status==1){
            if(b<mabore->partition[i].start){
                graph=concat(graph,"<td rowspan=\"2\">LIBRE<br/>");
                float te=mabore->partition[i].start-b;
                percent=(te/total)*100.0;
                cl(tempo);sprintf(tempo,"%f%% del disco",percent);graph=concat(graph,tempo);
                b=mabore->partition[i].start;
                i--;
            }
            else{
                if(mabore->partition[i].type=='P'){
                    graph=concat(graph,"<td rowspan=\"2\" BGCOLOR=\"#ffe4c4\">PRIMARIO<br/>");
                }
                else if(mabore->partition[i].type=='E'){
                    cl(tempo);sprintf(tempo,"<td colspan=\"%i\"",mabore->partition[i].tt);graph=concat(graph,tempo);
                    graph=concat(graph," BGCOLOR=\"#6699cc\"><br/>EXTENDIDA<br/>");
                }
                float te=mabore->partition[i].size;
                percent=(te/total)*100.0;
                cl(tempo);sprintf(tempo,"%f%% del disco",percent);graph=concat(graph,tempo);
                b=mabore->partition[i].size+mabore->partition[i].start;
            }
            graph=concat(graph,"</td>\n");
        }
    }

    if(b<total){
        graph=concat(graph,"<td rowspan=\"2\">LIBRE<br/>");
        float te=total-b;
        percent=(te/total)*100.0;
        cl(tempo);sprintf(tempo,"%f%% del disco",percent);graph=concat(graph,tempo);
        graph=concat(graph,"</td>\n");
    }

    graph=concat(graph,"</tr>\n");

    for(int i=0;i<4;i++){
        if(mabore->partition[i].type=='E'){
            FILE *fich;
            struct ebr exbore={0,0,0,0,0,""};
            for(unsigned int i=1;i<strlen(path);i++)
                if(path[i]==' ') path[i]='_';
            if((fich = fopen(path,"rb+"))==NULL){
                if(fich !=NULL) fclose(fich);
            }
            fseek(fich,mabore->partition[i].start,SEEK_SET);
            fread(&exbore,sizeof(exbore),1,fich);

            graph=concat(graph,"<tr>\n");

            while(exbore.next!=-1){
                if(exbore.status!=0){
                    graph=concat(graph,"<td BGCOLOR=\"#87a96b\"><br/>EBR");
                    b=sizeof(exbore);
                    percent=(b/total)*100.0;
                    cl(tempo);sprintf(tempo,"<br/>%f%% del disco",percent);graph=concat(graph,tempo);
                    graph=concat(graph,"</td>\n");

                    graph=concat(graph,"<td BGCOLOR=\"#a3c1ad\"><br/>LOGICA_");
                    graph=concat(graph,exbore.name);
                    b=exbore.size-b;
                    percent=(b/total)*100.0;
                    cl(tempo);sprintf(tempo,"<br/>%f%% del disco",percent);graph=concat(graph,tempo);
                    graph=concat(graph,"</td>\n");
                }
                else{
                    graph=concat(graph,"<td><br/>ESPACIO_LIBRE");
                    b=exbore.size;
                    percent=(b/total)*100.0;
                    cl(tempo);sprintf(tempo,"<br/>%f%% del disco",percent);graph=concat(graph,tempo);
                    graph=concat(graph,"</td>\n");
                }

                fseek(fich,exbore.next,SEEK_SET);
                fread(&exbore,sizeof(exbore),1,fich);
            }

            graph=concat(graph,"<td BGCOLOR=\"#87a96b\"><br/>EBR");
            b=sizeof(exbore);
            percent=(b/total)*100.0;
            cl(tempo);sprintf(tempo,"<br/>%f%% del disco",percent);graph=concat(graph,tempo);
            graph=concat(graph,"</td>\n");

            graph=concat(graph,"<td><br/>ESPACIO_LIBRE");
            b=exbore.size;
            percent=(b/total)*100.0;
            cl(tempo);sprintf(tempo,"<br/>%f%% del disco",percent);graph=concat(graph,tempo);
            graph=concat(graph,"</td>\n");

            fclose(fich);
            graph=concat(graph,"</tr>\n");
            break;
        }
    }

    graph=concat(graph,"</TABLE>\n>];\n}");
    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    char temp[256];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
void do_inode_graph(struct Super_Block *sb,FILE *rep,char text[256],char png[256], FILE *fich){
    char *graph="";
    char tempo[255]="";
    graph=concat(graph,"digraph inodos{\nnode [shape=plaintext]\n");

    char c=0;
    for(int i=0;i<sb->n;i++){
        fseek(fich,sb->bm_inode_start+i,SEEK_SET);
        c=getc(fich);
        if(c!=0){
            struct Inode inodo;
            fseek(fich,sb->inode_start+i*sizeof(inodo),SEEK_SET);
            fread(&inodo,sizeof(inodo),1,fich);
            cl(tempo);sprintf(tempo,"inode%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td colspan=\"2\">inode%i</td>\n</tr>",i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>uid</td>\n<td>%i</td></tr>",inodo.uid);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>gid</td>\n<td>%i</td></tr>",inodo.gid);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>size</td>\n<td>%i</td></tr>",inodo.size_file);graph=concat(graph,tempo);
            char * atime = ctime(&inodo.r_time);atime[strlen(atime)-1] = '\0';
            cl(tempo);sprintf(tempo,"<tr>\n<td>atime</td>\n<td>%s</td></tr>",atime);graph=concat(graph,tempo);
            char * cctime = ctime(&inodo.c_time);cctime[strlen(cctime)-1] = '\0';
            cl(tempo);sprintf(tempo,"<tr>\n<td>ctime</td>\n<td>%s</td></tr>",cctime);graph=concat(graph,tempo);
            char * wtime = ctime(&inodo.w_time);wtime[strlen(wtime)-1] = '\0';
            cl(tempo);sprintf(tempo,"<tr>\n<td>wtime</td>\n<td>%s</td></tr>",wtime);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>type</td>\n<td>%i</td></tr>",inodo.type);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>perm</td>\n<td>%o</td></tr>",inodo.perm);graph=concat(graph,tempo);
            for(int j=0;j<15;j++){
                cl(tempo);sprintf(tempo,"<tr>\n<td>block[%i]</td>\n<td>%i</td></tr>",j,inodo.block[j]);graph=concat(graph,tempo);
            }
            graph=concat(graph,"</TABLE>>];\n");
        }
    }



    graph=concat(graph,"}");

    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    fclose(fich);
    char temp[256];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
void do_block_graph(struct Super_Block *sb,FILE *rep,char text[256],char png[256], FILE *fich){
    char *graph="";
    char tempo[255]="";
    graph=concat(graph,"digraph blocks{\nnode [shape=plaintext]\n");

    char c=0;
    for(int i=0;i<3*sb->n;i++){
        fseek(fich,sb->bm_block_start+i,SEEK_SET);
        c=getc(fich);

        struct Block_file fil;
        struct Block_dir dir;
        struct Block_point point;
        switch(c){
            case 1://file
                fseek(fich,sb->block_start+i*sizeof(fil),SEEK_SET);
                fread(&fil,sizeof(fil),1,fich);
                cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td>file_block%i</td>\n</tr>",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td>%s</td>\n</tr>",fil.cont);graph=concat(graph,tempo);
                graph=concat(graph,"</TABLE>>];\n");
                break;
            case 2://dir
                fseek(fich,sb->block_start+i*sizeof(dir),SEEK_SET);
                fread(&dir,sizeof(dir),1,fich);
                cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td colspan=\"2\">directory_block%i</td>\n</tr>",i);graph=concat(graph,tempo);
                graph=concat(graph,"<tr><td>b_name</td><td>b_inode</td></tr>\n");
                for(int j=0;j<4;j++){
                    cl(tempo);sprintf(tempo,"<tr>\n<td>%s</td>\n<td>%i</td></tr>",dir.cont[j].name,dir.cont[j].inode);graph=concat(graph,tempo);
                }
                graph=concat(graph,"</TABLE>>];\n");
                break;
            case 3://pointers
                fseek(fich,sb->block_start+i*sizeof(point),SEEK_SET);
                fread(&point,sizeof(point),1,fich);
                cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td>pointers_block%i</td>\n</tr>",i);graph=concat(graph,tempo);
                graph=concat(graph,"<tr><td>\n");
                for(int j=0;j<16;j++){
                    cl(tempo);sprintf(tempo,"%i, ",point.pointer[j]);graph=concat(graph,tempo);
                    if((j+1)%4==0) graph=concat(graph,"<br/>");
                }
                graph=concat(graph,"</td></tr>\n");
                graph=concat(graph,"</TABLE>>];\n");
                break;
        }
    }
    graph=concat(graph,"}");

    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    fclose(fich);
    char temp[256];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
void do_bmi_graph(struct Super_Block *sb,FILE *rep, FILE *fich){
    char c=0;
    int max=sb->n;
    rewind(rep);
    for(int i=0;i<max;i++){
        fseek(fich,sb->bm_inode_start+i,SEEK_SET);
        c=getc(fich);
        if(c!=0){
            putc('1',rep);
            putc(' ',rep);
        }
        else{
            putc('0',rep);
            putc(' ',rep);
        }
        if((i+1)%20==0){
            putc('\n',rep);
        }
    }

    fclose(rep);
    fclose(fich);
}
void do_bmb_graph(struct Super_Block *sb,FILE *rep, FILE *fich){
    char c=0;
    int max=3*sb->n;
    rewind(rep);
    for(int i=0;i<max;i++){
        fseek(fich,sb->bm_block_start+i,SEEK_SET);
        c=getc(fich);
        if(c!=0){
            putc('1',rep);
            putc(' ',rep);
        }
        else{
            putc('0',rep);
            putc(' ',rep);
        }
        if((i+1)%20==0){
            putc('\n',rep);
        }
    }
    fclose(rep);
    fclose(fich);
}
void do_sb_graph(struct Super_Block *sb,FILE *rep,char text[256],char png[256], FILE *fich){
    char *graph="";
    char tempo[255];
    graph=concat(graph,"digraph html{\n");
    graph=concat(graph,"abc [shape=none, margin=0, label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");

    graph=concat(graph,"<TR><td>Nombre</td><td>Valor</td></TR>\n");

    graph=concat(graph,"<TR>\n<Td>inodes_count</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->inode_count);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>block_count</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->block_count);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>free_inodes_count</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->free_inode_count);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>free_block_count</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->free_block_count);graph=concat(graph,tempo);

    char * mtime = ctime(&sb->time_m);mtime[strlen(mtime)-1] = '\0';
    cl(tempo);sprintf(tempo,"<tr>\n<td>mtime</td>\n<td>%s</td></tr>",mtime);graph=concat(graph,tempo);
    char * umtime = ctime(&sb->time_um);umtime[strlen(umtime)-1] = '\0';
    cl(tempo);sprintf(tempo,"<tr>\n<td>umtime</td>\n<td>%s</td></tr>",umtime);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>mnt_count</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->m_count);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>magic</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>0x%04X</Td></TR>\n",sb->magic);graph=concat(graph,tempo);

    graph=concat(graph,"<TR>\n<Td>inodes_size</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->inode_size);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>block_size</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->block_size);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>first_inode</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->first_inode);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>first_block</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->first_block);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>bm_inode_start</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->bm_inode_start);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>bm_block_start</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->bm_block_start);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>inode_start</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->inode_start);graph=concat(graph,tempo);
    graph=concat(graph,"<TR>\n<Td>block_start</Td>\n");
    cl(tempo);sprintf(tempo,"<Td>%i</Td></TR>\n",sb->block_start);graph=concat(graph,tempo);


    graph=concat(graph,"</TABLE>\n>];\n}\n");

    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    fclose(fich);
    char temp[255];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
void do_tree_graph(struct Super_Block *sb,FILE *rep,char text[256],char png[256], FILE *fich){
    char *graph="";
    char tempo[255]="";
    graph=concat(graph,"digraph inodos{\nnode [shape=plaintext];rankdir=\"LR\";\n");

    char c=0;
    for(int i=0;i<sb->n;i++){
        fseek(fich,sb->bm_inode_start+i,SEEK_SET);
        c=getc(fich);
        if(c!=0){
            struct Inode inodo;
            char nombre[10]="";
            sprintf(nombre,"inode%i",i);
            fseek(fich,sb->inode_start+i*sizeof(inodo),SEEK_SET);
            fread(&inodo,sizeof(inodo),1,fich);
            cl(tempo);sprintf(tempo,"inode%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td port=\"i%i\" colspan=\"2\">inode%i</td>\n</tr>",i,i);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>uid</td>\n<td>%i</td></tr>",inodo.uid);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>gid</td>\n<td>%i</td></tr>",inodo.gid);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>size</td>\n<td>%i</td></tr>",inodo.size_file);graph=concat(graph,tempo);
            char * atime = ctime(&inodo.r_time);atime[strlen(atime)-1] = '\0';
            cl(tempo);sprintf(tempo,"<tr>\n<td>atime</td>\n<td>%s</td></tr>",atime);graph=concat(graph,tempo);
            char * cctime = ctime(&inodo.c_time);cctime[strlen(cctime)-1] = '\0';
            cl(tempo);sprintf(tempo,"<tr>\n<td>ctime</td>\n<td>%s</td></tr>",cctime);graph=concat(graph,tempo);
            char * wtime = ctime(&inodo.w_time);wtime[strlen(wtime)-1] = '\0';
            cl(tempo);sprintf(tempo,"<tr>\n<td>wtime</td>\n<td>%s</td></tr>",wtime);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>type</td>\n<td>%i</td></tr>",inodo.type);graph=concat(graph,tempo);
            cl(tempo);sprintf(tempo,"<tr>\n<td>perm</td>\n<td>%o</td></tr>",inodo.perm);graph=concat(graph,tempo);
            for(int j=0;j<15;j++){
                cl(tempo);sprintf(tempo,"<tr>\n<td>block[%i]</td>\n<td port=\"f%i\">%i</td></tr>",j,j,inodo.block[j]);graph=concat(graph,tempo);
            }
            graph=concat(graph,"</TABLE>>];\n");

            for(int j=0;j<15;j++){
                if(inodo.block[j]!=-1){
                    cl(tempo);sprintf(tempo,"%s:f%i -> block%i:b%i;\n",nombre,j,inodo.block[j],inodo.block[j]);graph=concat(graph,tempo);
                }
            }
        }
    }
    for(int i=0;i<3*sb->n;i++){
        fseek(fich,sb->bm_block_start+i,SEEK_SET);
        c=getc(fich);

        struct Block_file fil;
        struct Block_dir dir;
        struct Block_point point;
        char nombre[10]="";
        sprintf(nombre,"block%i",i);
        switch(c){

            case 1://file
                fseek(fich,sb->block_start+i*sizeof(fil),SEEK_SET);
                fread(&fil,sizeof(fil),1,fich);
                cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td port=\"b%i\">file_block%i</td>\n</tr>",i,i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td>%s</td>\n</tr>",fil.cont);graph=concat(graph,tempo);
                graph=concat(graph,"</TABLE>>];\n");
                break;
            case 2://dir
                fseek(fich,sb->block_start+i*sizeof(dir),SEEK_SET);
                fread(&dir,sizeof(dir),1,fich);
                cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td port=\"b%i\" colspan=\"2\">directory_block%i</td>\n</tr>",i,i);graph=concat(graph,tempo);
                graph=concat(graph,"<tr><td>b_name</td><td>b_inode</td></tr>\n");
                for(int j=0;j<4;j++){
                    cl(tempo);sprintf(tempo,"<tr>\n<td>%s</td>\n<td port=\"f%i\">%i</td></tr>",dir.cont[j].name,j,dir.cont[j].inode);graph=concat(graph,tempo);
                }
                graph=concat(graph,"</TABLE>>];\n");
                for(int j=0;j<4;j++){
                    if(dir.cont[j].inode!=-1 && dir.cont[j].name[0]!='.'){
                        cl(tempo);sprintf(tempo,"%s:f%i -> inode%i:i%i;\n",nombre,j,dir.cont[j].inode,dir.cont[j].inode);graph=concat(graph,tempo);
                    }
                }
                break;
            case 3://pointers
                fseek(fich,sb->block_start+i*sizeof(point),SEEK_SET);
                fread(&point,sizeof(point),1,fich);
                cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
                cl(tempo);sprintf(tempo,"<tr>\n<td port=\"b%i\">pointers_block%i</td>\n</tr>",i,i);graph=concat(graph,tempo);
                for(int j=0;j<16;j++){
                    cl(tempo);sprintf(tempo,"<tr><td port=\"f%i\">%i</td></tr>",j,point.pointer[j]);graph=concat(graph,tempo);
                }
                graph=concat(graph,"</TABLE>>];\n");
                for(int j=0;j<16;j++){
                    if(point.pointer[j]!=-1){
                        cl(tempo);sprintf(tempo,"%s:f%i -> block%i:b%i;\n",nombre,j,point.pointer[j],point.pointer[j]);graph=concat(graph,tempo);
                    }
                }
                break;
        }
    }


    graph=concat(graph,"}");

    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    fclose(fich);
    char temp[256];
    strcpy(temp,"dot -Tsvg ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    //remove(text);
}
void do_file_graph(struct Super_Block *sb,FILE *rep,FILE *fich,char path[256]){
    char *texto="";

    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);
    texto=read_file_i(&inodo,path,sb,fich);
    if(texto[0]==2){
        printf("    error en comando: 'rep', motivo: 'doesnt have reading permissions'.\n");
        fclose(rep);
        fclose(fich);
        return;
    }
    rewind(rep);
    fwrite(texto,strlen(texto),1,rep);
    fclose(rep);
    fclose(fich);
}
void do_journ_graph(struct Super_Block *sb,FILE *rep,char text[256],char png[256], FILE *fich){
    char *graph="";
    char tempo[255]="";
    graph=concat(graph,"digraph inodos{\nnode [shape=plaintext]\n");
    for(int i=0;i<counter;i++){
        struct Journaling journal={0,0,"",0,0,0,0,0};
        fseek(fich,sb->start+80+i*sizeof(journal),SEEK_SET);
        fread(&journal,sizeof(journal),1,fich);
        cl(tempo);sprintf(tempo,"block%i [shape=none, margin=0, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">",i);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>journal_block%i</td>\n</tr>",i);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%i</td>\n</tr>",journal.operation);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%i</td>\n</tr>",journal.tipo);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%s</td>\n</tr>",journal.name);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%i</td>\n</tr>",journal.content);graph=concat(graph,tempo);
        char * mtime = ctime(&journal.time);mtime[strlen(mtime)-1] = '\0';
        cl(tempo);sprintf(tempo,"<tr>\n<td>mtime</td>\n<td>%s</td></tr>",mtime);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%i</td>\n</tr>",journal.uid);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%i</td>\n</tr>",journal.gid);graph=concat(graph,tempo);
        cl(tempo);sprintf(tempo,"<tr>\n<td>Operacion</td><td>%i</td>\n</tr>",journal.permisos);graph=concat(graph,tempo);
        graph=concat(graph,"</TABLE>>];\n");
    }

    graph=concat(graph,"}");
    fwrite(graph,1,strlen(graph),rep);
    fclose(rep);
    fclose(fich);
    char temp[255];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
void do_ls_graph(struct Super_Block *sb,FILE *rep,char text[256],char png[256], FILE *fich,char path[256]){
    ls="";
    ls=concat(ls,"digraph html{\n");
    ls=concat(ls,"abc [shape=none, margin=0, label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");
    ls=concat(ls,"<tr><td>Permisos</td><td>Owner</td><td>Group</td><td>Size</td><td>Date</td><td>Type</td><td>Name</td></tr>");

    list(path,sb,fich);

    ls=concat(ls,"</TABLE>\n>];\n}\n");

    fwrite(ls,1,strlen(ls),rep);
    fclose(rep);
    fclose(fich);
    char temp[255];
    strcpy(temp,"dot -Tpng ");
    strcat(temp,text);
    strcat(temp," > ");
    strcat(temp,png);
    system(temp);
    remove(text);
}
char* concat(const char *s1, const char *s2){
    int a=strlen(s1);
    int b=strlen(s2);
    char *result = malloc(a + b + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
void cl(char temp[255]){
    for(int i=0;i<20;i++)
        temp[i]=0x0;
}
int put_ebr(struct ebr *exbore, FILE *fich,struct Part *particion){
    struct ebr temp;
    fseek(fich,particion->start,SEEK_SET);
    fread(&temp,sizeof(temp),1,fich);
    int ss=temp.next;

    if(particion->fit=='F'){
        while(ss!=-1){
            if(temp.status==0 && temp.size>exbore->size){
                exbore->start=temp.start;
                exbore->next=ss;
                fseek(fich,exbore->start,SEEK_SET);
                fwrite(exbore,sizeof(*exbore),1,fich);
                return 0;
            }
            else{
                fseek(fich,ss,SEEK_SET);
                fread(&temp,sizeof(temp),1,fich);
                ss=temp.next;
            }
        }
    }
    else if(particion->fit=='W'){
        int size_ant=0;
        int space=0;
        int start_space=particion->start;
        int temp_next=0;
        bool eq=false;
        struct ebr ebr_temp;
        while(ss!=-1){
            size_ant=temp.size;
            if(temp.status==0 && temp.size>=exbore->size && space<size_ant){
                space=size_ant;
                start_space=temp.start;
                temp_next=temp.next;
                if(temp.size==exbore->size)eq=true;
                else ebr_temp=temp;
            }
            fseek(fich,ss,SEEK_SET);
            fread(&temp,sizeof(temp),1,fich);
            ss=temp.next;
        }
        if(space!=0){
            if(eq){
                exbore->next=temp_next;
            }
            else{
                ebr_temp.size-=exbore->size;
                ebr_temp.start+=exbore->size;
                fseek(fich,ebr_temp.start,SEEK_SET);
                fwrite(&ebr_temp,sizeof(ebr_temp),1,fich);
                exbore->next=ebr_temp.start;
            }
            exbore->start=start_space;

            fseek(fich,exbore->start,SEEK_SET);
            fwrite(exbore,sizeof(*exbore),1,fich);
            return 0;
        }
    }
    else{
        int size_ant=0;
        int space=0x7fffffff;
        int start_space=particion->start;
        int temp_next=0;
        bool eq=false;
        struct ebr ebr_temp;
        while(ss!=-1){
            size_ant=temp.size;
            if(temp.status==0 && temp.size>=exbore->size && space>size_ant){
                space=size_ant;
                start_space=temp.start;
                temp_next=temp.next;
                if(temp.size==exbore->size)eq=true;
                else ebr_temp=temp;
            }
            fseek(fich,ss,SEEK_SET);
            fread(&temp,sizeof(temp),1,fich);
            ss=temp.next;
        }
        if(space!=0x7fffffff){
            if(eq){
                exbore->next=temp_next;
            }
            else{
                ebr_temp.size-=exbore->size;
                ebr_temp.start+=exbore->size;
                fseek(fich,ebr_temp.start,SEEK_SET);
                fwrite(&ebr_temp,sizeof(ebr_temp),1,fich);
                exbore->next=ebr_temp.start;
            }
            exbore->start=start_space;

            fseek(fich,exbore->start,SEEK_SET);
            fwrite(exbore,sizeof(*exbore),1,fich);
            return 0;
        }
    }

    if(temp.size>exbore->size){
        exbore->start=temp.start;
        exbore->next=temp.start+temp.size;
        struct ebr siguiente={0,0,exbore->next,temp.size-exbore->size,-1,"LIBRE"};
        fseek(fich,exbore->start,SEEK_SET);
        fwrite(exbore,sizeof(*exbore),1,fich);
        fseek(fich,siguiente.start,SEEK_SET);
        fwrite(&siguiente,sizeof(siguiente),1,fich);
    }
    else return errR("fdisk","doesnt have enough space for logic partition");
    return 0;
}

int mk_format(int start, int size, FILE *fich, bool type,bool t){
    struct Super_Block sb={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if(t){
        struct Journaling journal={1,0,"mkfs",0,time(NULL),start,size,0};
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);
    }
    usuario.UID=1;
    int progres_bar[9]={size/10, size/5, 3*size/10, 2*size/5, size/2, 3*size/5, 7*size/10, 4*size/5, 9*size/10};
    int pr=0;
    printf("    Formating... please wait...[");
    fflush(stdout);
    if(type){
        fseek(fich,start,SEEK_SET);
        for(int i=0;i<size;i++){
            fputc(0,fich);
            if(i==progres_bar[pr]){
                printf("#");
                fflush(stdout);
                pr++;
            }
        }
        printf("#]\n");
        fflush(stdout);
    }
    else{
        printf("##########]\n");
    }
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';


    struct Journaling jo={0,0,"",0,0,0,0,0};
    struct Inode in={0,0,0,0,0,0,{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0};
    struct Block_file bf={""};
    int s=sizeof(sb);
    int j=sizeof(jo);

    int n=(size-sizeof(sb))/(sizeof(jo)+sizeof(in)+3*sizeof(bf)+4);

    sb.inode_count=n;
    sb.block_count=3*n;
    sb.free_inode_count=n;
    sb.free_block_count=3*n;
    sb.m_count=1;
    sb.time_m=time(NULL);
    sb.time_um=time(NULL);
    sb.magic=0xEF53;
    sb.inode_size=sizeof(in);
    sb.block_size=sizeof(bf);
    sb.first_inode=0;
    sb.first_block=0;
    sb.bm_inode_start=start+sizeof(sb)+sizeof(jo)*n;
    sb.bm_block_start=sb.bm_inode_start+n;
    sb.inode_start=sb.bm_block_start+3*n;
    sb.block_start=sb.inode_start+n*sizeof(in);
    sb.n=n;
    sb.start=start;

    fseek(fich,start,SEEK_SET);
    fwrite(&sb,sizeof(sb),1,fich);

    in.uid=0;
    in.gid=0;
    in.size_file=0;
    in.r_time=time(NULL);
    in.c_time=time(NULL);
    in.w_time=time(NULL);
    in.type=0;
    in.perm=0664;
    fseek(fich,sb.bm_inode_start,SEEK_SET);
    putc(1,fich);
    sb.first_inode=find_inode(&sb,fich);
    struct Block_dir dir={{{".",0},{"..",0},{"",-1},{"",-1}}};
    
    in.block[0]=add_block_dir(&dir,&sb,fich);

    char *cadena="1,G,root\n1,U,root,root,123\n";
    crear_archivo_i(&in,1,1,"users.txt","/",&sb,fich,cadena);

    fseek(fich,sb.inode_start,SEEK_SET);
    fwrite(&in,sizeof(in),1,fich);
    fseek(fich,start,SEEK_SET);
    fwrite(&sb,sizeof(sb),1,fich);

    usuario.UID=0;
    return 0;
}
int find_inode(struct Super_Block *sb, FILE *fich){
    char c=1;
    int i=-1;
    while(c!=0x00){
        i++;
        fseek(fich,sb->bm_inode_start+i,SEEK_SET);
        c=getc(fich);
    }
    return i;
}
int add_inode(struct Inode *in, struct Super_Block *sb, FILE *fich){
    int num=sb->first_inode;
    int mb_inode_dir=sb->bm_inode_start+num;
    int inode_dir=sb->inode_start+num*sb->inode_size;

    fseek(fich,mb_inode_dir,SEEK_SET);
    putc(1,fich);
    fseek(fich,inode_dir,SEEK_SET);
    fwrite(in,sizeof(*in),1,fich);
    sb->first_inode=find_inode(sb,fich);
    fseek(fich,sb->start,SEEK_SET);
    fwrite(sb,sizeof(*sb),1,fich);
    return num;
}
int find_block(struct Super_Block *sb, FILE *fich){
    char c=1;
    int i=-1;
    while(c!=0x00){
        i++;
        fseek(fich,sb->bm_block_start+i,SEEK_SET);
        c=getc(fich);
    }
    return i;
}
int add_block_dir(struct Block_dir *dir, struct Super_Block *sb, FILE *fich){
    int num = sb->first_block;
    int mb_block_dir=sb->bm_block_start+num;
    int block_dir=sb->block_start+num*sb->block_size;

    fseek(fich,mb_block_dir,SEEK_SET);
    fputc(2,fich);
    fseek(fich,block_dir,SEEK_SET);
    fwrite(dir,sizeof(*dir),1,fich);
    sb->first_block=find_block(sb,fich);
    return num;
}
int add_block_file(struct Block_file *file, struct Super_Block *sb, FILE *fich){
    int num=sb->first_block;
    int mb_block_file=sb->bm_block_start+num;
    int block_file=sb->block_start+num*sb->block_size;

    fseek(fich,mb_block_file,SEEK_SET);
    fputc(1,fich);
    fseek(fich,block_file,SEEK_SET);
    fwrite(file,sizeof(*file),1,fich);
    sb->first_block=find_block(sb,fich);
    fseek(fich,sb->start,SEEK_SET);
    fwrite(sb,sizeof(*sb),1,fich);
    return num;
}
int add_block_pointer(struct Block_point *pointer, struct Super_Block *sb, FILE *fich){
    int num =sb->first_block;
    int mb_block_dir=sb->bm_block_start+num;
    int block_dir=sb->block_start+num*sb->block_size;

    fseek(fich,mb_block_dir,SEEK_SET);
    fputc(3,fich);
    fseek(fich,block_dir,SEEK_SET);
    fwrite(pointer,sizeof(*pointer),1,fich);
    sb->first_block=find_block(sb,fich);
    fseek(fich,sb->start,SEEK_SET);
    fwrite(sb,sizeof(*sb),1,fich);
    return num;
}
int crear_carpeta_i(struct Inode *inodo, bool p, int uid, int gid, char name[12], char *path,struct Super_Block *sb, FILE *fich){
    char path_[255]="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(i<12){
            if(inodo->block[i]!=-1){
                int dir=inodo->block[i];
                int pun=0;
                int creado=crear_carpeta(p,uid,gid,name,str,&pun,&dirs,dir,sb,fich,0,permiso(inodo->uid,inodo->gid,inodo->perm,3));
                if(creado==0){ //todo correcto
                    fseek(fich,sb->inode_start,SEEK_SET);
                    fwrite(inodo,sizeof(*inodo),1,fich);
                    return 0;
                }
                else if(creado==1){
                    return errR("mkdir","dir not found");
                }
                else if(creado==2 && inodo->block[i+1]==-1){//lleno && siguiente dir vacio
                    if(i<11){
                        struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                        inodo->block[i+1]=add_block_dir(&vacio,sb,fich);
                    }
                    else{
                        struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                        struct Block_point punteros={{add_block_dir(&vacio,sb,fich),-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                        inodo->block[i+1]=add_block_pointer(&punteros,sb,fich);
                        fseek(fich,sb->inode_start,SEEK_SET);
                        fwrite(inodo,sizeof(*inodo),1,fich);
                    }
                }
                else if(creado==4)
                    return errR("mkdir","doesnt have the necessary permissions");
            }
        }
        else if(i==12){
            struct Block_point punteros;
            fseek(fich,sb->block_start+sizeof(punteros)*inodo->block[12],SEEK_SET);
            fread(&punteros,sizeof(punteros),1,fich);
            for(int ii=0;ii<16;ii++){
                if(punteros.pointer[ii]!=-1){
                    int dir=punteros.pointer[ii];
                    int pun=0;
                    int creado=crear_carpeta(p,uid,gid,name,str,&pun,&dirs,dir,sb,fich,0,permiso(inodo->uid,inodo->gid,inodo->perm,3));
                    if(creado==0){ //todo correcto
                        fseek(fich,sb->block_start+sizeof(punteros)*inodo->block[12],SEEK_SET);
                        fwrite(&punteros,sizeof(punteros),1,fich);
                        return 0;
                    }
                    else if(creado==1){
                        return errR("mkdir","dir not found");
                    }
                    else if(creado==2 && punteros.pointer[ii+1]==-1){
                        if(ii<15){
                            struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                            punteros.pointer[ii+1]=add_block_dir(&vacio,sb,fich);
                        }
                        else{

                        }
                    }
                    else if(creado==4)
                        return errR("mkdir","doesnt have the necessary permissions");
                }

            }
        }
    }
    return 0;
}
int crear_carpeta(bool p,int uid, int gid, char name[12], char str[40][12],int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich,int padre, bool let){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int ii=0;ii<15;ii++){
                    if(ii<12){
                        if(inodo.block[ii]!=-1){
                            int dir=inodo.block[ii];
                            int creado = crear_carpeta(p,uid,gid,name,str,puntero,dirs,dir,sb,fich,carpeta.cont[i].inode,permiso(inodo.uid,inodo.gid,inodo.perm,3));
                            if(creado == 0){
                                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                                fwrite(&inodo,sizeof(inodo),1,fich);
                                return 0;//todo good
                            }
                            else if(creado==1){
                                return errR("mkdir","dir not found");
                            }
                            else if(creado==2 && inodo.block[ii+1]==-1){
                                if(ii<11){
                                    struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                                    inodo.block[ii+1]=add_block_dir(&vacio,sb,fich);
                                    fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                                    fwrite(&inodo,sizeof(inodo),1,fich);
                                }
                                else{
                                    struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                                    struct Block_point punteros={{add_block_dir(&vacio,sb,fich),-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                                    inodo.block[ii+1]=add_block_pointer(&punteros,sb,fich);
                                    fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                                    fwrite(&inodo,sizeof(inodo),1,fich);
                                }
                            }
                        }
                    }
                    else if(ii==12){
                        struct Block_point punteros;
                        fseek(fich,sb->block_start+sizeof(punteros)*inodo.block[12],SEEK_SET);
                        fread(&punteros,sizeof(punteros),1,fich);
                        for(int iii=0;iii<16;iii++){
                            if(punteros.pointer[iii]!=-1){
                                int dir=punteros.pointer[iii];
                                int creado=crear_carpeta(p,uid,gid,name,str,puntero,dirs,dir,sb,fich,0,permiso(inodo.uid,inodo.gid,inodo.perm,3));
                                if(creado==0){ //todo correcto
                                    fseek(fich,sb->block_start+sizeof(punteros)*inodo.block[12],SEEK_SET);
                                    fwrite(&punteros,sizeof(punteros),1,fich);
                                    return 0;
                                }
                                else if(creado==1){
                                    return errR("mkdir","dir not found");
                                }
                                else if(creado==2 && punteros.pointer[iii+1]==-1){
                                    struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                                    punteros.pointer[iii+1]=add_block_dir(&vacio,sb,fich);
                                }
                            }

                        }
                    }
                }
                return 2;//inode full
            }
            if(carpeta.cont[i].inode==-1){
                if(p){
                    if(!let)return 4;
                    struct Inode in={uid,gid,0,time(NULL),time(NULL),time(NULL),{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0664};
                    strcpy(carpeta.cont[i].name,str[*puntero]);
                    struct Block_dir vacio={{{".",0},{"..",padre},{"",-1},{"",-1}}};
                    in.block[0]=add_block_dir(&vacio,sb,fich);
                    carpeta.cont[i].inode=add_inode(&in,sb,fich);
                    vacio.cont[0].inode=carpeta.cont[i].inode;
                    update_dir(&carpeta,directorio,sb,fich);
                    update_dir(&vacio,in.block[0],sb,fich);
                    *puntero+=1;
                    int dir=in.block[0];
                    int creado = crear_carpeta(p,uid,gid,name,str,puntero,dirs,dir,sb,fich,carpeta.cont[i].inode,permiso(in.uid,in.gid,in.perm,3));
                    if(creado == 0){
                        fseek(fich,sb->inode_start+sizeof(in)*carpeta.cont[i].inode,SEEK_SET);
                        fwrite(&in,sizeof(in),1,fich);
                        return 0;//todo good
                    }
                    return 1;
                }
                else{
                    bool counter=true;
                    for(int j=0;j<4;j++){
                        if(carpeta.cont[j].inode!=-1){
                            counter= false;
                            break;
                        }
                    }
                    if(counter){
                        fseek(fich,sb->bm_block_start+directorio,SEEK_SET);
                        putc(0,fich);
                    }
                    return 1;//folder not found
                }

            }
        }
        return 2;//no enough space
    }
    else{
        for(int ii=0;ii<4;ii++){
            if(carpeta.cont[ii].inode==-1){
                if(!let)return 4;
                struct Inode in={uid,gid,0,time(NULL),time(NULL),time(NULL),{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0664};
                strcpy(carpeta.cont[ii].name,name);
                struct Block_dir vacio={{{".",0},{"..",padre},{"",-1},{"",-1}}};
                in.block[0]=add_block_dir(&vacio,sb,fich);
                carpeta.cont[ii].inode=add_inode(&in,sb,fich);
                vacio.cont[0].inode=carpeta.cont[ii].inode;
                update_dir(&carpeta,directorio,sb,fich);
                update_dir(&vacio,in.block[0],sb,fich);
                return 0;//todo good
            }
        }
        return 2;//block dir full
    }
}
void update_dir(struct Block_dir *dir, int num,struct Super_Block *sb,FILE *fich){
    fseek(fich,sb->block_start+num*sizeof(*dir),SEEK_SET);
    fwrite(dir,sizeof(*dir),1,fich);
}
bool permiso(int owner, int group,int permiso,int get){
    bool dejar=true;
    if(usuario.GID>1){
        int perm=permiso;
        int u=perm/0100;
        perm=perm%0100;
        int g=perm/010;
        perm=perm%010;
        int o=perm;
        switch(get){
            case 1://escritura
                if(owner==usuario.UID){
                    if(u<2 || u==4 || u==5)dejar=false;
                }
                else if(group==usuario.GID){
                    if(g<2 || g==4 || g==5)dejar=false;
                }
                else{
                    if(o<2 || o==4 || o==5)dejar=false;
                }
                break;
            case 2://lectura
                if(owner==usuario.UID){
                    if(u<4)dejar=false;
                }
                else if(group==usuario.GID){
                    if(g<4)dejar=false;
                }
                else{
                    if(o<4)dejar=false;
                }
                break;
            case 3://lectura y escritura
                if(owner==usuario.UID){
                    if(u<6)dejar=false;
                }
                else if(group==usuario.GID){
                    if(g<6)dejar=false;
                }
                else{
                    if(o<6)dejar=false;
                }
                break;
        }

    }
    return dejar;
}
int crear_archivo_i(struct Inode *inodo, int uid, int gid, char name[12], char *path,struct Super_Block *sb, FILE *fich,char *texto){
    char path_[255]="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            int creado=crear_archivo(uid,gid,name,str,&pun,&dirs,dir,sb,fich,texto,permiso(inodo->uid,inodo->gid,inodo->perm,3));
            if(creado==0) //todo correcto
                break;
            else if(creado==1 && inodo->block[i+1]==-1){//lleno && siguiente dir vacio
                //struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                //inodo->block[i+1]=add_block_dir(&vacio,sb,fich);
                return errR("mkdir","dir doesnt exist");
            }
            else if(creado==2)
                return errR("mkdir","doesnt have the necessary permissions");
        }
    }
    return 0;
}
int crear_archivo(int uid, int gid, char name[12], char str[40][12],int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich,char *text, bool let){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int ii=0;ii<15;ii++){
                    if(inodo.block[ii]!=-1){
                        int dir=inodo.block[ii];
                        int creado = crear_archivo(uid,gid,name,str,puntero,dirs,dir,sb,fich,text,permiso(inodo.uid,inodo.gid,inodo.perm,3));
                        if(creado == 0){
                            fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                            fwrite(&inodo,sizeof(inodo),1,fich);
                            return 0;
                        }
                        else if(creado==1 && inodo.block[ii+1]==-1 && *puntero==*dirs){
                            struct Block_dir vacio={{{"",-1},{"",-1},{"",-1},{"",-1}}};
                            inodo.block[ii+1]=add_block_dir(&vacio,sb,fich);
                            fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                            fwrite(&inodo,sizeof(inodo),1,fich);
                        }
                        else if(creado==2)
                            return 2;
                    }
                }
                return 1;
            }
        }
        return 1;//dir or file not found
    }
    else{
        return archivo(uid,gid,name,&carpeta,directorio,sb,fich,text,let);
    }
}
int archivo(int uid, int gid, char name[12], struct Block_dir *carpeta,int directorio, struct Super_Block *sb, FILE *fich,char *text,bool let){
    for(int i=0;i<4;i++){
        if(carpeta->cont[i].inode==-1){
            if(!let)return 2;
            strcpy(carpeta->cont[i].name,name);
            struct Inode inodo={uid,gid,strlen(text),time(NULL),time(NULL),time(NULL),{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},1,0664};

            unsigned int length=0;
            for(int ii=0;ii<15 && length<strlen(text);ii++){
                if(ii<12){
                    inodo.block[ii]=addtext(sb,fich,text,&length);
                }
                else if(ii==12){
                    struct Block_point punteros={{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                    for(int j=0;j<16 && length<strlen(text);j++){
                        punteros.pointer[j]=addtext(sb,fich,text,&length);
                    }
                    inodo.block[ii]=add_block_pointer(&punteros,sb,fich);
                }
                else if(ii==13){
                    struct Block_point punteros={{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                    for(int j=0;j<16 && length<strlen(text);j++){
                        struct Block_point punteros1={{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                        for(int k=0;k<16 && length<strlen(text);k++){
                            punteros1.pointer[k]=addtext(sb,fich,text,&length);
                        }
                        punteros.pointer[j]=add_block_pointer(&punteros1,sb,fich);
                    }
                    inodo.block[ii]=add_block_pointer(&punteros,sb,fich);
                }
                else if(ii==14){
                    struct Block_point punteros={{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                    for(int j=0;j<16 && length<strlen(text);j++){
                        struct Block_point punteros1={{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                        for(int k=0;k<16 && length<strlen(text);k++){
                            struct Block_point punteros2={{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
                            for(int l=0;l<16 && length<strlen(text);l++){
                                punteros2.pointer[l]=addtext(sb,fich,text,&length);
                            }
                            punteros1.pointer[k]=add_block_pointer(&punteros2,sb,fich);
                        }
                        punteros.pointer[j]=add_block_pointer(&punteros1,sb,fich);
                    }
                    inodo.block[ii]=add_block_pointer(&punteros,sb,fich);
                }
            }
            carpeta->cont[i].inode=add_inode(&inodo,sb,fich);
            update_dir(carpeta,directorio,sb,fich);
            return 0;
        }
    }
    return 1;
}
int addtext(struct Super_Block *sb,FILE *fich,char *text,unsigned int *length){
    struct Block_file file={""};
    for(unsigned int i=*length,j=0;j<63 && *length<strlen(text);i++,j++,*length+=1){
        file.cont[j]=text[i];
    }
    return add_block_file(&file,sb,fich);
}
char* read_file_i(struct Inode *inodo, char *path,struct Super_Block *sb, FILE *fich){
    char path_[255]="";
    char name_[15]="";
    char *null="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    strcpy(name_,ptr);
    ptr=strtok(NULL,delim);
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            char *texto=read_file(str,name_,&pun,&dirs,dir,sb,fich,permiso(inodo->uid,inodo->gid,inodo->perm,2));
            if(texto[0]==0) continue;
            else return texto;
        }
    }
    return null;
}
char* read_file(char str[40][12],char *name,int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich, bool let){
    char *null="";
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int i=0;i<15;i++){
                    if(inodo.block[i]!=-1){
                        int dir=inodo.block[i];
                        char *texto = read_file(str,name,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,2));
                        if(texto[0]==0) continue;
                        else return texto;
                    }
                }
                return null;
            }
        }
        return null;
    }
    else{
        return leer(&carpeta,name,directorio,sb,fich,let);
    }
}
char* leer(struct Block_dir *carpeta,char *name,int directorio, struct Super_Block *sb, FILE *fich, bool let){
    char null[1]="";
    for(int i=0;i<4;i++){
        if(strcmp(carpeta->cont[i].name,name)==0){
            if(!let){null[0]=2;return null;}
            struct Inode inodo;
            fseek(fich,sb->inode_start+sizeof(inodo)*carpeta->cont[i].inode,SEEK_SET);
            fread(&inodo,sizeof(inodo),1,fich);

            char *texto="";
            for(int ii=0;ii<15;ii++){
                if(inodo.block[ii]!=-1){
                    struct Block_file file;
                    fseek(fich,sb->block_start+sizeof(file)*inodo.block[ii],SEEK_SET);
                    fread(&file,sizeof(file),1,fich);
                    texto=concat(texto,file.cont);
                }
                else break;
            }
            return texto;
        }
    }
    return null;
}
int logear(char user[11], char pass[11],char path[255],char part[5],struct Super_Block *sb, FILE *fich){
    char UsGr[25][5][10]={{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}}};
    char UG[25][60]={"","","","","","","","","","","","","","","","","","","","","","","","",""};
    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);

    char *texto=read_file_i(&inodo,"users.txt",sb,fich);
    char delim[]="\n";
    char *ptr=strtok(texto,delim);
    int temp=1;
    strcpy(UG[0],ptr);
    ptr=strtok(NULL,delim);
    for(int j=1;ptr!=NULL;j++,temp++){
        strcpy(UG[j],ptr);
        ptr=strtok(NULL,delim);
    }
    delim[0]=',';
    for(int j=0;j<temp;j++){
        ptr=strtok(UG[j],delim);
        strcpy(UsGr[j][0],ptr);
        ptr=strtok(NULL,delim);
        for(int k=1;ptr!=NULL;k++){
            strcpy(UsGr[j][k],ptr);
            ptr=strtok(NULL,delim);
        }
    }
    for(int i=0;i<temp;i++){
        if(UsGr[i][1][0]=='U' && strcmp(UsGr[i][3],user)==0 && strcmp(UsGr[i][4],pass)==0){
            usuario.UID=UsGr[i][0][0]-48;
            char grupo[11]="";
            strcpy(grupo,UsGr[i][2]);
            for(int j=0;j<temp;j++){
                if(UsGr[j][1][0]=='G' && strcmp(grupo,UsGr[j][2])==0){
                    usuario.GID=UsGr[j][0][0]-48;
                    break;
                }
            }
        }
    }
    if(usuario.UID==0 || usuario.GID==0){
        usuario.UID=0;
        usuario.GID=0;
        return errR("login","cant log in");
    }
    else{
        strcpy(usuario.path,path);
        strcpy(usuario.part,part);
        printf("    login correct!\n");
    }
    return 0;
}
int make_group(char name[11], struct Super_Block *sb, FILE *fich){
    char UsGr[25][5][10]={{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}}};
    char UG[25][60]={"","","","","","","","","","","","","","","","","","","","","","","","",""};
    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);

    char *text=read_file_i(&inodo,"users.txt",sb,fich);
    char *texto="";
    texto=concat(texto,text);
    char delim[]="\n";
    char *ptr=strtok(texto,delim);
    int temp=1;
    strcpy(UG[0],ptr);
    ptr=strtok(NULL,delim);
    for(int j=1;ptr!=NULL;j++,temp++){
        strcpy(UG[j],ptr);
        ptr=strtok(NULL,delim);
    }
    delim[0]=',';
    for(int j=0;j<temp;j++){
        ptr=strtok(UG[j],delim);
        strcpy(UsGr[j][0],ptr);
        ptr=strtok(NULL,delim);
        for(int k=1;ptr!=NULL;k++){
            strcpy(UsGr[j][k],ptr);
            ptr=strtok(NULL,delim);
        }
    }
    int grupo=0;
    for(int i=0;i<temp;i++){
        if(UsGr[i][1][0]=='G'){
           int gr=UsGr[i][0][0]-48;
           if(grupo<gr)grupo=gr;
           if(strcmp(UsGr[i][2],name)==0)
               return errR("mkgrp","group already created with that name");
        }
    }
    char cadena[30]="";
    sprintf(cadena,"%i,G,%s\n",++grupo,name);
    text=concat(text,cadena);
    return escribir_file_i(&inodo,"/users.txt",sb,fich,text);
}
int escribir_file_i(struct Inode *inodo, char *path,struct Super_Block *sb, FILE *fich,char *text){
    char path_[255]="";
    char name_[15]="";
    char *null="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    strcpy(name_,ptr);
    ptr=strtok(NULL,delim);
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            int creado = escribir_file(str,name_,&pun,&dirs,dir,sb,fich,text);
            if(creado == 0){
                return 0;
            }
        }
    }
    return 1;
}
int escribir_file(char str[40][12],char *name,int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich,char *text){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int i=0;i<15;i++){
                    if(inodo.block[i]!=-1){
                        int dir=inodo.block[i];
                        int creado = escribir_file(str,name,puntero,dirs,dir,sb,fich,text);
                        if(creado == 0){
                            return 0;
                        }

                    }
                }
                return 1;
            }
        }
        return 1;
    }
    else{
        return actualizar_file(&carpeta,name,sb,fich,text);
    }
}
int actualizar_file(struct Block_dir *carpeta,char *name, struct Super_Block *sb, FILE *fich,char *text){
    for(int i=0;i<4;i++){
        if(strcmp(carpeta->cont[i].name,name)==0){
            struct Inode inodo;
            fseek(fich,sb->inode_start+sizeof(inodo)*carpeta->cont[i].inode,SEEK_SET);
            fread(&inodo,sizeof(inodo),1,fich);
            inodo.size_file=strlen(text);
            unsigned int length=0;
            for(int ii=0;ii<15 && length<strlen(text);ii++){
                if(inodo.block[ii]!=-1){
                    fseek(fich,sb->bm_block_start+inodo.block[ii],SEEK_SET);
                    putc(0,fich);
                    sb->first_block=find_block(sb,fich);
                }
                inodo.block[ii]=addtext(sb,fich,text,&length);
            }
            fseek(fich,sb->inode_start+sizeof(inodo)*carpeta->cont[i].inode,SEEK_SET);
            fwrite(&inodo,sizeof(inodo),1,fich);
            fseek(fich,sb->start,SEEK_SET);
            fwrite(sb,sizeof(*sb),1,fich);
            return 0;
        }
    }
    return 1;
}
int make_user(char group[11],char user[11],char pass[11], struct Super_Block *sb, FILE *fich){
    char UsGr[25][5][10]={{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}},{{""},{""},{""},{""},{""}}};
    char UG[25][60]={"","","","","","","","","","","","","","","","","","","","","","","","",""};
    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);


    char *text=read_file_i(&inodo,"users.txt",sb,fich);
    char *texto="";
    texto=concat(texto,text);
    char delim[]="\n";
    char *ptr=strtok(texto,delim);
    int temp=1;
    strcpy(UG[0],ptr);
    ptr=strtok(NULL,delim);
    for(int j=1;ptr!=NULL;j++,temp++){
        strcpy(UG[j],ptr);
        ptr=strtok(NULL,delim);
    }
    delim[0]=',';
    for(int j=0;j<temp;j++){
        ptr=strtok(UG[j],delim);
        strcpy(UsGr[j][0],ptr);
        ptr=strtok(NULL,delim);
        for(int k=1;ptr!=NULL;k++){
            strcpy(UsGr[j][k],ptr);
            ptr=strtok(NULL,delim);
        }
    }
    int usuari=0;
    bool exist=false;
    for(int i=0;i<temp;i++){
        if(UsGr[i][1][0]=='U'){
           int us=UsGr[i][0][0]-48;
           if(usuari<us)usuari=us;
           if(strcmp(UsGr[i][3],user)==0)
               return errR("mkusr","user already created with that name");
        }
        else{
            if(strcmp(UsGr[i][2],group)==0)
                exist=true;
        }
    }
    if(!exist)return errR("mkusr","that group doesn't exist");
    char cadena[60]="";
    sprintf(cadena,"%i,U,%s,%s,%s\n",++usuari,group,user,pass);
    text=concat(text,cadena);
    return escribir_file_i(&inodo,"/users.txt",sb,fich,text);
}
int make_directory(char path[255],bool p, struct Super_Block *sb, FILE *fich){
    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);

    char path_[255]="";
    char name_[15]="";
    char only_path[255]="/";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    strcpy(name_,ptr);
    ptr=strtok(NULL,delim);
    for(int j=0;ptr!=NULL;j++){
        strcat(only_path,"/");
        strcat(only_path,name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    char *nombre=&name_;
    char *pathe=&only_path;
    return crear_carpeta_i(&inodo,p,usuario.UID,usuario.GID,nombre,pathe,sb,fich);
}
int make_file(char path[255], bool p, int size, char cont[255], bool tam, struct Super_Block *sb,FILE* fich){
    char *texto="";

    char path_[255]="";
    char name_[15]="";
    char only_path[255]="/";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    strcpy(name_,ptr);
    ptr=strtok(NULL,delim);
    for(int j=0;ptr!=NULL;j++){
        strcat(only_path,"/");
        strcat(only_path,name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    char *nombre=&name_;
    char *pathe=&only_path;

    if(tam){
        for(int i=0, num=0;i<size;i++,num++){
            char temp[2]="";
            if(i%10==0)num=0;
            temp[0]=num+48;
            texto=concat(texto,temp);
        }
    }
    else{
        FILE *entrada;
        if((entrada=fopen(cont,"rt"))!=NULL){
            char comando[256];
            while(!feof(entrada)){
                limpiar(comando);
                fgets(comando,256,entrada);
                texto=concat(texto,comando);
            }
            fclose(entrada);
        }
    }
    if(p) make_directory(pathe,true,sb,fich);
    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);

    return crear_archivo_i(&inodo,usuario.UID,usuario.GID,nombre,pathe,sb,fich,texto);
}
int remover(char path[255], struct Super_Block *sb,FILE* fich){

    char path_[255]="";
    char name_[15]="";
    char only_path[255]="/";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    strcpy(name_,ptr);
    ptr=strtok(NULL,delim);
    for(int j=0;ptr!=NULL;j++){
        strcat(only_path,"/");
        strcat(only_path,name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    char *nombre=&name_;
    char *pathe=&only_path;

    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);
    int ret = remover_i(&inodo,nombre,pathe,sb,fich);
    if(ret==1)return 0;
    else return errR("rem","cant remove file or directory");
}
int remover_i(struct Inode *inodo, char name[15], char path[255], struct Super_Block *sb,FILE* fich){
    char path_[255]="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            int removido=remover_cosa(usuario.UID,usuario.GID,name,str,&pun,&dirs,dir,sb,fich,permiso(inodo->uid,inodo->gid,inodo->perm,3));
            if(removido==1) //todo correcto
                return 1;
        }
    }
    return 0;
}
int remover_cosa(int uid, int gid, char name[12], char str[40][12],int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich, bool let){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int ii=0;ii<15;ii++){
                    if(ii<12){
                        if(inodo.block[ii]!=-1){
                            int dir=inodo.block[ii];
                            int removido = remover_cosa(uid,gid,name,str,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,3));
                            if(removido==1) //todo correcto
                                return 1;
                        }
                    }
                    else if(ii==12){
                        struct Block_point punteros;
                        fseek(fich,sb->block_start+sizeof(punteros)*inodo.block[12],SEEK_SET);
                        fread(&punteros,sizeof(punteros),1,fich);
                        for(int iii=0;iii<16;iii++){
                            if(punteros.pointer[iii]!=-1){
                                int dir=punteros.pointer[iii];
                                int removido = remover_cosa(uid,gid,name,str,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,3));
                                if(removido==1) //todo correcto
                                    return 1;
                            }
                        }
                    }
                }
                return 0;//inode full
            }
        }
        return 0;//no enough space
    }
    else{
        for(int ii=0;ii<4;ii++){
            if(strcmp(carpeta.cont[ii].name,name)==0){
                //printf("lo encontré :3");
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[ii].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                int eliminado = quitar_todo(&inodo,carpeta.cont[ii].inode,sb,fich);

                if(eliminado==1){
                    strcpy(carpeta.cont[ii].name,"");
                    carpeta.cont[ii].inode=-1;
                    update_dir(&carpeta,directorio,sb,fich);
                    return 1;
                }

            }
        }
        return 0;//block dir full
    }
}
int quitar_todo(struct Inode *inodo, int bm_in, struct Super_Block *sb,FILE *fich){
    if(permiso(inodo->uid,inodo->gid,inodo->perm,3)){
        for(int i=0;i<15;i++){
            if(inodo->block[i]!=-1){
                struct Block_dir dir;
                fseek(fich,sb->block_start+sizeof(dir)*inodo->block[i],SEEK_SET);
                fread(&dir,sizeof(dir),1,fich);
                for(int j=0;j<4;j++){
                    if(dir.cont[j].name[0]!='.' && dir.cont[j].inode!=-1){
                        struct Inode in;
                        fseek(fich,sb->inode_start+sizeof(in)*dir.cont[j].inode,SEEK_SET);
                        fread(&in,sizeof(in),1,fich);
                        int eliminado = quitar_todo(&in,dir.cont[j].inode,sb,fich);
                        if(eliminado==0)
                            return 0;
                    }
                }
                fseek(fich,sb->bm_block_start+inodo->block[i],SEEK_SET);
                putc(0,fich);
            }
        }
        fseek(fich,sb->bm_inode_start+bm_in,SEEK_SET);
        putc(0,fich);
        return 1;
    }
    else
        return 0;
}
int mover(char path[255],char dest[255], struct Super_Block *sb,FILE* fich){
    char path_[255]="";
    char name_[15]="";
    char only_path[255]="/";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    strcpy(name_,ptr);
    ptr=strtok(NULL,delim);
    for(int j=0;ptr!=NULL;j++){
        strcat(only_path,"/");
        strcat(only_path,name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    char *nombre=&name_;
    char *pathe=&only_path;

    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);
    int ret = mover_i(&inodo,nombre,pathe,sb,fich,dest);
    if(ret==0)return 0;
    else return errR("rem","cant move file or directory");
}
int mover_i(struct Inode *inodo, char name[15], char path[255], struct Super_Block *sb,FILE* fich,char dest[255]){
    char path_[255]="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            int removido=mover_cosa(usuario.UID,usuario.GID,name,str,&pun,&dirs,dir,sb,fich,permiso(inodo->uid,inodo->gid,inodo->perm,3),dest);
            if(removido==0) //todo correcto
                return 0;
        }
    }
    return 1;
}
int mover_cosa(int uid, int gid, char name[12], char str[40][12],int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich, bool let,char dest[255]){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int ii=0;ii<15;ii++){
                    if(ii<12){
                        if(inodo.block[ii]!=-1){
                            int dir=inodo.block[ii];
                            int removido=mover_cosa(usuario.UID,usuario.GID,name,str,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,3),dest);
                            if(removido==0) //todo correcto
                                return 0;
                        }
                    }
                    else if(ii==12){
                        struct Block_point punteros;
                        fseek(fich,sb->block_start+sizeof(punteros)*inodo.block[12],SEEK_SET);
                        fread(&punteros,sizeof(punteros),1,fich);
                        for(int iii=0;iii<16;iii++){
                            if(punteros.pointer[iii]!=-1){
                                int dir=punteros.pointer[iii];
                                int removido=mover_cosa(usuario.UID,usuario.GID,name,str,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,3),dest);
                                if(removido==0) //todo correcto
                                    return 0;
                            }
                        }
                    }
                }
                return 1;//inode full
            }
        }
        return 1;//no enough space
    }
    else{
        for(int ii=0;ii<4;ii++){
            if(strcmp(carpeta.cont[ii].name,name)==0){
                //printf("lo encontré :3");
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[ii].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                int eliminado = mover_todo(dest,carpeta.cont[ii].inode,carpeta.cont[ii].name,sb,fich);

                if(eliminado==0){
                    strcpy(carpeta.cont[ii].name,"");
                    carpeta.cont[ii].inode=-1;
                    update_dir(&carpeta,directorio,sb,fich);
                    return 0;
                }

            }
        }
        return 1;//block dir full
    }
}
int mover_todo(char path[255],int ino, char nombr[12], struct Super_Block *sb,FILE* fich){

    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);
    return mover_todo_i(&inodo,path,sb,fich,ino, nombr);
}
int mover_todo_i(struct Inode *inodo, char path[255], struct Super_Block *sb,FILE* fich,int ino, char nombr[12]){
    char path_[255]="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            int removido=mover_todo_ica(usuario.UID,usuario.GID,str,&pun,&dirs,dir,sb,fich,permiso(inodo->uid,inodo->gid,inodo->perm,3),ino,nombr);
            if(removido==0) //todo correcto
                return 0;
        }
    }
    return 1;
}
int mover_todo_ica(int uid, int gid, char str[40][12],int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich, bool let,int ino, char nombr[12]){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int ii=0;ii<15;ii++){
                    if(ii<12){
                        if(inodo.block[ii]!=-1){
                            int dir=inodo.block[ii];
                            int removido = mover_todo_ica(uid,gid,str,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,3),ino,nombr);
                            if(removido==0) //todo correcto
                                return 0;
                        }
                    }
                    else if(ii==12){
                        struct Block_point punteros;
                        fseek(fich,sb->block_start+sizeof(punteros)*inodo.block[12],SEEK_SET);
                        fread(&punteros,sizeof(punteros),1,fich);
                        for(int iii=0;iii<16;iii++){
                            if(punteros.pointer[iii]!=-1){
                                int dir=punteros.pointer[iii];
                                int removido = mover_todo_ica(uid,gid,str,puntero,dirs,dir,sb,fich,permiso(inodo.uid,inodo.gid,inodo.perm,3),ino,nombr);
                                if(removido==0) //todo correcto
                                    return 0;
                            }
                        }
                    }
                }
                return 1;//inode full
            }
        }
        return 1;//no enough space
    }
    else{
        for(int ii=0;ii<4;ii++){
            if(carpeta.cont[ii].inode==-1){
                carpeta.cont[ii].inode=ino;
                strcpy(carpeta.cont[ii].name,nombr);
                update_dir(&carpeta,directorio,sb,fich);
                return 0;
            }
        }
        return 1;//block dir full
    }
}
void tronar(struct Super_Block *sb, FILE* fich){
    fseek(fich,sb->bm_inode_start,SEEK_SET);
    int size=(sb->block_start+3*64*sb->n)-sb->bm_inode_start;
    int progres_bar[9]={size/10, size/5, 3*size/10, 2*size/5, size/2, 3*size/5, 7*size/10, 4*size/5, 9*size/10};
    int pr=0;
    printf("    DELETING...[");
    fflush(stdout);
    for(int i=0;i<size;i++){
        fputc(0,fich);
        if(i==progres_bar[pr]){
            printf("#");
            fflush(stdout);
            pr++;
        }
    }
    printf("#]\n");
    fflush(stdout);
}
void recuperar(struct Super_Block *sb, FILE* fich){
    for(int i=0;i<counter;i++){
        struct Journaling journal={0,0,"",0,0,0,0,0};
        fseek(fich,sb->start+80+i*sizeof(journal),SEEK_SET);
        fread(&journal,sizeof(journal),1,fich);
        switch (journal.operation) {
            case 1:{
                int start=journal.uid;
                int size=journal.gid;
                mk_format(start,size,fich,0,false);
                break;
            }
            case 2:{
                char aux[150];
                strcpy(aux,journal.name);
                make_group(aux,sb,fich);
                break;
            }
            case 3:{
                char *delim=",";
                char str[150];
                strcpy(str,journal.name);
                char *ptr=strtok(str,delim);
                char grp[12]="";
                strcpy(grp,ptr);
                ptr=strtok(NULL,delim);
                char usr[12]="";
                strcpy(usr,ptr);
                ptr=strtok(NULL,delim);
                char psw[12]="";
                strcpy(psw,ptr);
                make_user(grp,usr,psw,sb,fich);
                break;
            }
            case 4:{
                char path[150]="";
                strcpy(path,journal.name);
                bool p=journal.tipo;
                usuario.UID=journal.uid;
                usuario.GID=journal.gid;
                make_directory(path,p,sb,fich);
                break;
            }
            case 5:{
                struct Journaling journal2={0,0,"",0,0,0,0,0};
                fseek(fich,sb->start+80+(++i)*sizeof(journal2),SEEK_SET);
                fread(&journal2,sizeof(journal2),1,fich);

                char path[150];
                strcpy(path,journal.name);
                char path2[150];
                strcpy(path2,journal2.name);
                bool p=journal.tipo;
                int size=journal.uid;
                int tam=journal2.tipo;
                usuario.UID=journal.uid;
                usuario.GID=journal.gid;
                make_file(path,p,size,path2,tam,sb,fich);
                break;
            }
            case 6:{
                char aux[150];
                strcpy(aux,journal.name);
                remover(aux,sb,fich);
                break;
            }
            case 7:{
                struct Journaling journal2={0,0,"",0,0,0,0,0};
                fseek(fich,sb->start+80+(++i)*sizeof(journal2),SEEK_SET);
                fread(&journal2,sizeof(journal2),1,fich);

                char path[150];
                strcpy(path,journal.name);
                char path2[150];
                strcpy(path2,journal2.name);

                mover(path,path2,sb,fich);
                break;
            }
            default:
                break;
        }
    }
}
void list(char path[256],struct Super_Block *sb, FILE* fich){

    char path_[255]="";
    char name_[15]="";
    char only_path[255]="/";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    if(ptr!=NULL){
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    for(int j=0;ptr!=NULL;j++){
        strcat(only_path,"/");
        strcat(only_path,name_);
        strcpy(name_,ptr);
        ptr=strtok(NULL,delim);
    }
    char *nombre=&name_;
    char *pathe=&only_path;

    struct Inode inodo;
    fseek(fich,sb->inode_start,SEEK_SET);
    fread(&inodo,sizeof(inodo),1,fich);
    list_i(&inodo,nombre,pathe,sb,fich);
}
int list_i(struct Inode *inodo, char name[15], char path[255], struct Super_Block *sb,FILE* fich){
    char path_[255]="";
    strcpy(path_,path);
    char delim[]="/";
    char *ptr=strtok(path_,delim);
    char str[40][12];
    int dirs=0;
    for(int i=0;i<40;i++)
        for(int k=0;k<12;k++)
            str[i][k]=0;
    for(int j=0;ptr!=NULL;j++,dirs++){
        strcpy(str[j],ptr);
        ptr=strtok(NULL,delim);
    }
    for(int i=0;i<15;i++){
        if(inodo->block[i]!=-1){
            int dir=inodo->block[i];
            int pun=0;
            int removido=lists(usuario.UID,usuario.GID,name,str,&pun,&dirs,dir,sb,fich);
            if(removido==1) //todo correcto
                return 1;
        }
    }
    return 0;
}
int lists(int uid, int gid, char name[12], char str[40][12],int *puntero, int *dirs,int directorio, struct Super_Block *sb, FILE *fich){
    struct Block_dir carpeta;
    int address=sb->block_start+sizeof(carpeta)*directorio;
    fseek(fich,address,SEEK_SET);
    fread(&carpeta,sizeof(carpeta),1,fich);
    if(*puntero<*dirs){
        for(int i=0;i<4;i++){
            if(strcmp(carpeta.cont[i].name,str[*puntero])==0){
                *puntero+=1;
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[i].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);
                for(int ii=0;ii<15;ii++){
                    if(ii<12){
                        if(inodo.block[ii]!=-1){
                            int dir=inodo.block[ii];
                            int removido = lists(uid,gid,name,str,puntero,dirs,dir,sb,fich);
                            if(removido==1) //todo correcto
                                return 1;
                        }
                    }
                    else if(ii==12){
                        struct Block_point punteros;
                        fseek(fich,sb->block_start+sizeof(punteros)*inodo.block[12],SEEK_SET);
                        fread(&punteros,sizeof(punteros),1,fich);
                        for(int iii=0;iii<16;iii++){
                            if(punteros.pointer[iii]!=-1){
                                int dir=punteros.pointer[iii];
                                int removido = lists(uid,gid,name,str,puntero,dirs,dir,sb,fich);
                                if(removido==1) //todo correcto
                                    return 1;
                            }
                        }
                    }
                }
                return 0;//inode full
            }
        }
        return 0;//no enough space
    }
    else{
        if(strcmp(name,"")==0){
            for(int k=0;k<4;k++){
                if(carpeta.cont[k].name[0]!='.' && carpeta.cont[k].inode!=-1){
                    char tempo[255]="";
                    ls=concat(ls,"<tr>");
                    struct Inode in;
                    fseek(fich,sb->inode_start+sizeof(in)*carpeta.cont[k].inode,SEEK_SET);
                    fread(&in,sizeof(in),1,fich);
                    char temp[11]="";
                    getperm(temp, in);
                    cl(tempo);sprintf(tempo,"<td>%s</td>",temp);ls=concat(ls,tempo);
                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.uid);ls=concat(ls,tempo);
                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.gid);ls=concat(ls,tempo);
                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.size_file);ls=concat(ls,tempo);
                    char * mtime = ctime(&in.c_time);mtime[strlen(mtime)-1] = '\0';
                    cl(tempo);sprintf(tempo,"<td>%s</td>",mtime);ls=concat(ls,tempo);
                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.type);ls=concat(ls,tempo);
                    cl(tempo);sprintf(tempo,"<td>%s</td>",carpeta.cont[k].name);ls=concat(ls,tempo);

                    ls=concat(ls,"</tr>");
                }
            }
        }
        for(int ii=0;ii<4;ii++){
            if(strcmp(carpeta.cont[ii].name,name)==0){
                //printf("lo encontré :3");
                struct Inode inodo;
                fseek(fich,sb->inode_start+sizeof(inodo)*carpeta.cont[ii].inode,SEEK_SET);
                fread(&inodo,sizeof(inodo),1,fich);

                for(int j=0;j<15;j++){
                    if(j<12){
                        if(inodo.block[j]!=-1){
                            struct Block_dir dir;
                            fseek(fich,sb->block_start+sizeof(dir)*inodo.block[j],SEEK_SET);
                            fread(&dir,sizeof(dir),1,fich);
                            for(int k=0;k<4;k++){
                                if(dir.cont[k].name[0]!='.' &&dir.cont[k].inode!=-1){
                                    char tempo[255]="";
                                    ls=concat(ls,"<tr>");
                                    struct Inode in;
                                    fseek(fich,sb->inode_start+sizeof(in)*carpeta.cont[k].inode,SEEK_SET);
                                    fread(&in,sizeof(in),1,fich);
                                    char temp[11]="";
                                    getperm(temp, in);
                                    cl(tempo);sprintf(tempo,"<td>%s</td>",temp);ls=concat(ls,tempo);
                                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.uid);ls=concat(ls,tempo);
                                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.gid);ls=concat(ls,tempo);
                                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.size_file);ls=concat(ls,tempo);
                                    char * mtime = ctime(&in.c_time);mtime[strlen(mtime)-1] = '\0';
                                    cl(tempo);sprintf(tempo,"<td>%s</td>",mtime);ls=concat(ls,tempo);
                                    cl(tempo);sprintf(tempo,"<td>%i</td>",in.type);ls=concat(ls,tempo);
                                    cl(tempo);sprintf(tempo,"<td>%s</td>",dir.cont[k].name);ls=concat(ls,tempo);

                                    ls=concat(ls,"</tr>");
                                }
                            }
                        }
                    }
                    else if (j==12){

                    }
                }
            }
        }
        return 0;//block dir full
    }
}
void getperm(char str[11],struct Inode in){
    if(in.type==0) str[0]='d';
    else str[0]='-';
    int perm=in.perm;
    int u=perm/0100;
    perm=perm%0100;
    int g=perm/010;
    perm=perm%010;
    int o=perm;
    char temp[4]="";
    getp(temp,u);
    strcat(str,temp);
    getp(temp,g);
    strcat(str,temp);
    getp(temp,o);
    strcat(str,temp);
}
void getp(char str[4],int i){
    if(i==0)strcpy(str,"---");
    else if(i==1)strcpy(str,"--x");
    else if(i==2)strcpy(str,"-w-");
    else if(i==3)strcpy(str,"-wx");
    else if(i==4)strcpy(str,"r--");
    else if(i==5)strcpy(str,"r-x");
    else if(i==6)strcpy(str,"rw-");
    else if(i==7)strcpy(str,"rwx");
}
/***********************************************/

int MKDISK(char strs[20][256], int j){
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
        else
            return errR(strs[0],"parameter not found");
    }
    if(size==0) return errR(strs[0],"size not valid");
    if(path[0]==0) return errR(strs[0],"path not valid");
    char aux[256];
    quitar(aux, path);
    //printf("%i|%c|%i|%s\n",size,fit,unit,aux);
    return crear_disco(size,fit,unit,aux);
}
int RMDISK(char strs[20][256]){
    char g;
    printf("Are you sure to remove disk? (y/n)");
    fgets(cmd,256,stdin);
    g=cmd[0];
    if(g=='y' || g=='Y'){
        if(strcmp(strs[1],"-path")==0){
            char dir[256];
            quitar(dir, strs[2]);
            int status;
            for(unsigned int i=1;i<strlen(dir);i++)
                if(dir[i]==' ') dir[i]='_';
            status=remove(dir);
            if(status==0){
                printf("****************************** DISCO ELIMINADO ******************************\n\n");
            }
            else return errR(strs[0],"path of file not found");
        }
        else return errR(strs[0],"parameter not found");
    }
    else{
        printf("****************************** DISCO NO ELIMINADO ******************************\n\n");
    }
    return 0;
}
int FDISK(char strs[20][256], int j){
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
            if(exec=='c')exec='d';
            if(strcmp(strs[2*i],"fast")==0) strcpy(del,"fast");
            else if(strcmp(strs[2*i],"full")==0) strcpy(del,"full");
            else return errR(strs[0],"fit not valid");
        }
        else if(strcmp(strs[2*i-1],"-name")==0){
            strcpy(name, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-add")==0){
            if(exec=='c')exec='a';
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
        //printf("%i|%c|%i|%c|%s|%s\n",size, fit, unit, type, aux_p, aux_n);
        return crear_part(size, fit, unit, type, aux_p, aux_n);
    }
    else if(exec=='a'){
        //printf("%i|%i|%s|%s\n",add, unit, aux_p, aux_n);
        return add_part(add, unit, aux_p, aux_n);
    }
    else if(exec=='d'){
        //printf("%s|%s|%s\n",del, aux_p, aux_n);
        return del_part(del, aux_p, aux_n);
    }
    else return errR(strs[0],"command is not valid");
}
int MOUNT(char strs[20][256], int j){
    char path[256];
    char name[256];
    for(int i=1;i<j+1;i++){
        if(strcmp(strs[2*i-1],"-path")==0){
            strcpy(path, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-name")==0){
            strcpy(name, strs[2*i]);
        }
    }
    for(mon_num=0;mon_num<200;mon_num++){
        if(strcmp(mon[mon_num].path,path)==0){
            for(int i=0;i<4;i++){
                if(mon[mon_num].part[i][0]==0){
                    FILE *fich;
                    struct mbr mabore;
                    for(unsigned int ii=1;ii<strlen(path);ii++)
                        if(path[ii]==' ') path[ii]='_';
                    if((fich = fopen(path,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("fdisk","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    for(int jj=0;jj<4;jj++){
                        if(strcmp(mabore.partition[jj].name,name)==0){
                            mabore.partition[jj].mounted=true;
                            break;
                        }
                        if(jj==3)
                            return errR("fdisk","cant find partition in disk");
                    }
                    strcpy(mon[mon_num].part[i],name);
                    mon[mon_num].id[i][0]='v';
                    mon[mon_num].id[i][1]='d';
                    mon[mon_num].id[i][2]=97+mon_num;
                    mon[mon_num].id[i][3]=49+i;
                    escribir_mabore(&mabore,fich);
                    fclose(fich);
                    return 0;
                }
            }
        }
        else if(mon[mon_num].path[0]==0){

            for(int i=0;i<4;i++){
                if(mon[mon_num].part[i][0]==0){
                    FILE *fich;
                    struct mbr mabore;
                    for(unsigned int ii=1;ii<strlen(path);ii++)
                        if(path[ii]==' ') path[ii]='_';
                    if((fich = fopen(path,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("fdisk","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    for(int jj=0;jj<4;jj++){
                        if(strcmp(mabore.partition[jj].name,name)==0){
                            mabore.partition[jj].mounted=true;
                            break;
                        }
                        if(jj==3)
                            return errR("fdisk","cant find partition in disk");
                    }
                    strcpy(mon[mon_num].path,path);
                    strcpy(mon[mon_num].part[i],name);
                    mon[mon_num].id[i][0]='v';
                    mon[mon_num].id[i][1]='d';
                    mon[mon_num].id[i][2]=97+mon_num;
                    mon[mon_num].id[i][3]=49+i;
                    escribir_mabore(&mabore,fich);
                    fclose(fich);
                    return 0;
                }
            }
        }
    }
    return 0;
}
int UNMOUNT(char strs[20][256]){
    char id[256];
    if(strcmp(strs[1],"-id")==0){
        strcpy(id, strs[2]);
    }
    else
        return errR(strs[0],"parameter not found");
    bool temp=false;
    for(mon_num=0;mon_num<200;mon_num++){
        for(int i=0;i<4;i++){
            if(strcmp(mon[mon_num].id[i],id)==0){

                FILE *fich;
                struct mbr mabore;
                char path[256];
                strcpy(path,mon[mon_num].path);
                for(unsigned int ii=1;ii<strlen(path);ii++)
                    if(path[ii]==' ') path[ii]='_';
                if((fich = fopen(path,"rb+"))==NULL){
                    if(fich !=NULL) fclose(fich);
                    return errR("fdisk","cant open disk");
                }
                fread(&mabore,sizeof(mabore),1,fich);
                for(int jj=0;jj<4;jj++){
                    if(strcmp(mabore.partition[jj].name,mon[mon_num].part[i])==0){
                        mabore.partition[jj].mounted=false;
                    }
                }
                escribir_mabore(&mabore,fich);
                fclose(fich);

                for(int j=0;j<4;j++)
                    mon[mon_num].id[i][j]=0;
                limpiar(mon[mon_num].part[i]);
                temp=true;
                break;
            }
            else if(mon[mon_num].path[0]==0){
                return errR(strs[0],"cant find mounted partition");
            }
        }
        if(temp) break;
    }
    return 0;
}
int PAUSE(){
    printf("Press ENTER key to Continue\n");
    fgets(cmd,256,stdin);
    return 0;
}
int REP(char strs[20][256], int j){
    char id[256];
    char path_rep[256];
    char name[12];
    char ruta[256];
    for(int i=1;i<j+1;i++){
        if(strcmp(strs[2*i-1],"-id")==0){
            strcpy(id, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-path")==0){
            strcpy(path_rep, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-name")==0){
            strcpy(name, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-ruta")==0){
            char ruta_[256];
            strcpy(ruta_, strs[2*i]);
            quitar(ruta, ruta_);
        }
        else
            return errR(strs[0],"parameter not found");
    }
    if(strcmp(name,"mbr")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        char path_[256];
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){

                    FILE *fich;

                    strcpy(path_,mon[mon_num].path);
                    for(unsigned int ii=1;ii<strlen(path_);ii++)
                        if(path_[ii]==' ') path_[ii]='_';
                    if((fich = fopen(path_,"rb"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("fdisk","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    fclose(fich);
                }
            }
        }

        do_mbr_graph(&mabore,rep,path,aux,path_);
    }
    else if(strcmp(name,"disk")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        char path_[256];
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){

                    FILE *fich;

                    strcpy(path_,mon[mon_num].path);
                    for(unsigned int ii=1;ii<strlen(path_);ii++)
                        if(path_[ii]==' ') path_[ii]='_';
                    if((fich = fopen(path_,"rb"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("fdisk","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    fclose(fich);
                }
            }
        }

        do_disk_graph(&mabore,rep,path,aux,path_);
    }
    else if(strcmp(name,"inode")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            goto endi;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        endi:
        do_inode_graph(&sb,rep,path,aux,fich);
    }
    else if(strcmp(name,"block")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            goto endb;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        endb:
        do_block_graph(&sb,rep,path,aux,fich);
    }
    else if(strcmp(name,"bm_inode")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char path[256];
        quitar(path,path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            break;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        do_bmi_graph(&sb,rep,fich);
    }
    else if(strcmp(name,"bm_block")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char path[256];
        quitar(path,path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            break;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        do_bmb_graph(&sb,rep,fich);
    }
    else if(strcmp(name,"sb")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            break;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        do_sb_graph(&sb,rep,path,aux,fich);
    }
    else if(strcmp(name,"tree")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            goto endt;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        endt:
        do_tree_graph(&sb,rep,path,aux,fich);
    }
    else if(strcmp(name,"file")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        FILE *rep;
        char path[256];
        quitar(path,path_rep);
        struct mbr mabore;
        struct Super_Block sb;
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("rep","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);

                    for(unsigned int i=1;i<strlen(ruta);i++)
                        if(disk[i]==' ') ruta[i]='_';
                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("rep","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            break;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        do_file_graph(&sb,rep,fich,ruta);
    }
    else if(strcmp(name,"journaling")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            goto endj;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        endj:
        do_journ_graph(&sb,rep,path,aux,fich);
    }

    else if(strcmp(name,"ls")==0){
        if(path_rep[0]==0) return errR(strs[0],"path not valid");
        char aux[256];
        quitar(aux, path_rep);
        FILE *rep;
        struct mbr mabore;
        struct Super_Block sb;
        char path[256];
        strcpy(path,aux);
        strcat(path,".txt");
        if((rep=fopen_dir_txt(path))==NULL){
            if(rep !=NULL) fclose(rep);
            return errR("mkdisk","cant create rep");
        }
        FILE *fich;
        for(mon_num=0;mon_num<200;mon_num++){
            for(int i=0;i<4;i++){
                if(strcmp(mon[mon_num].id[i],id)==0){
                    char disk[256];
                    char part[256];
                    strcpy(disk,mon[mon_num].path);
                    strcpy(part,mon[mon_num].part[i]);


                    for(unsigned int i=1;i<strlen(disk);i++)
                        if(disk[i]==' ') disk[i]='_';
                    if((fich = fopen(disk,"rb+"))==NULL){
                        if(fich !=NULL) fclose(fich);
                        return errR("mkfs","cant open disk");
                    }
                    fread(&mabore,sizeof(mabore),1,fich);
                    int start=0;
                    for(int j=0;j<4;j++){
                        if(strcmp(mabore.partition[j].name,part)==0){
                            start=mabore.partition[j].start;
                            fseek(fich,start,SEEK_SET);
                            fread(&sb,sizeof(sb),1,fich);
                            break;
                        }
                        if(j==3)errR("mkfs","partition not found");
                    }
                    break;
                }
            }
            if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
        }
        do_ls_graph(&sb,rep,path,aux,fich,ruta);
    }
    else return errR(strs[0],"parameter of rep not found");
    return 0;
}
int EXEC(char strs[20][256]){
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
                    printf("%s",comando);
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
    return 0;
}
int MKFS(char strs[20][256], int j){
    char id[256];
    limpiar(id);
    bool type=true;
    for(int i=1;i<j+1;i++){
        if(strcmp(strs[2*i-1],"-id")==0){
            strcpy(id, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-fs")==0){
            continue;
        }
        else if(strcmp(strs[2*i-1],"-type")==0){
            if(strcmp(strs[2*i],"fast")==0) type=false;
            else if(strcmp(strs[2*i],"full")==0) type=true;
            else return errR(strs[0],"type not valid");
        }
        else return errR(strs[0],"parameter not found");
    }
    for(mon_num=0;mon_num<200;mon_num++){
        for(int i=0;i<4;i++){
            if(strcmp(mon[mon_num].id[i],id)==0){
                char disk[256];
                char part[256];
                strcpy(disk,mon[mon_num].path);
                strcpy(part,mon[mon_num].part[i]);

                FILE *fich;
                struct mbr mabore;
                for(unsigned int i=1;i<strlen(disk);i++)
                    if(disk[i]==' ') disk[i]='_';
                if((fich = fopen(disk,"rb+"))==NULL){
                    if(fich !=NULL) fclose(fich);
                    return errR("mkfs","cant open disk");
                }
                fread(&mabore,sizeof(mabore),1,fich);
                int start=0;
                int size=0;
                for(int j=0;j<4;j++){
                    if(strcmp(mabore.partition[j].name,part)==0){
                        start=mabore.partition[j].start;
                        size=mabore.partition[j].size;
                        break;
                    }
                    if(j==3)errR("mkfs","partition not found");
                }

                int ret= mk_format(start,size,fich,type,true);
                fclose(fich);
                return ret;
            }
        }
        if(mon[mon_num].path[0]==0)errR("mkfs","mount not found");
    }
    return 0;
}
int LOGIN(char strs[20][256], int j){
    if(usuario.UID!=0) return errR(strs[0],"already logged in");
    char id[255]="";
    char usr[11]="";
    char pwd[11]="";
    for(int i=1;i<j+1;i++){
        if(strcmp(strs[2*i-1],"-usr")==0){
            strcpy(usr, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-pwd")==0){
            strcpy(pwd, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-id")==0){
            strcpy(id, strs[2*i]);
        }
        else return errR(strs[0],"parameter not found");
    }
    for(mon_num=0;mon_num<200;mon_num++){
        for(int i=0;i<4;i++){
            if(strcmp(mon[mon_num].id[i],id)==0){
                char disk[256];
                char part[256];
                strcpy(disk,mon[mon_num].path);
                strcpy(part,mon[mon_num].part[i]);

                FILE *fich;
                struct mbr mabore;
                for(unsigned int i=1;i<strlen(disk);i++)
                    if(disk[i]==' ') disk[i]='_';
                if((fich = fopen(disk,"rb+"))==NULL){
                    if(fich !=NULL) fclose(fich);
                    return errR("login","cant open disk");
                }
                fread(&mabore,sizeof(mabore),1,fich);
                int start=0;
                struct Super_Block sb;
                for(int j=0;j<4;j++){
                    if(strcmp(mabore.partition[j].name,part)==0){
                        start=mabore.partition[j].start;
                        fseek(fich,start,SEEK_SET);
                        fread(&sb,sizeof(sb),1,fich);
                        break;
                    }
                    if(j==3)errR("mkfs","partition not found");
                }

                char aux[256];
                quitar(aux, usr);
                char aux2[256];
                quitar(aux2, pwd);
                minusculas(aux);
                minusculas(aux2);
                int ret = logear(aux,aux2,disk,part,&sb,fich);
                fclose(fich);
                return ret;
            }
        }
        if(mon[mon_num].path[0]==0)errR("login","mount not found");
    }
    return 0;

}
int LOGOUT(){
    if(usuario.UID==0)
        return errR("logout","no user logged in");
    else{
        usuario.UID=0;
        usuario.GID=0;
        strcpy(usuario.part,"");
        strcpy(usuario.path,"");
    }
    return 0;
}
int MKGRP(char strs[20][256], int j){
    char name[11]="";
    if(strcmp(strs[1],"-name")==0){
        strcpy(name, strs[2]);
    }
    else return errR(strs[0],"parameter not found");
    if(usuario.UID!=0){
        FILE *fich;
        struct mbr mabore;
        if((fich = fopen(usuario.path,"rb+"))==NULL){
            if(fich !=NULL) fclose(fich);
            return errR("login","cant open disk");
        }
        fread(&mabore,sizeof(mabore),1,fich);
        int start=0;
        struct Super_Block sb;
        for(int k=0;k<4;k++){
            if(strcmp(mabore.partition[k].name,usuario.part)==0){
                start=mabore.partition[k].start;
                fseek(fich,start,SEEK_SET);
                fread(&sb,sizeof(sb),1,fich);
                break;
            }
            if(k==3)errR("mkgrp","partition not found");
        }

        char aux[256];
        quitar(aux, name);
        minusculas(aux);
        struct Journaling journal={2,0,"aux",0,time(NULL),0,0,0};
        strcpy(journal.name,aux);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);
        int ret = make_group(aux,&sb,fich);
        fclose(fich);
        return ret;
    }
    else return errR("mkgrp","no user has logged in");

    return 0;
}
int MKUSR(char strs[20][256], int j){
    char grp[11]="";
    char usr[11]="";
    char pwd[11]="";
    for(int i=1;i<j+1;i++){
        if(strcmp(strs[2*i-1],"-usr")==0){
            strcpy(usr, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-pwd")==0){
            strcpy(pwd, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-grp")==0){
            strcpy(grp, strs[2*i]);
        }
        else return errR(strs[0],"parameter not found");
    }

    if(usuario.UID!=0){
        FILE *fich;
        struct mbr mabore;
        if((fich = fopen(usuario.path,"rb+"))==NULL){
            if(fich !=NULL) fclose(fich);
            return errR("login","cant open disk");
        }
        fread(&mabore,sizeof(mabore),1,fich);
        int start=0;
        struct Super_Block sb;
        for(int j=0;j<4;j++){
            if(strcmp(mabore.partition[j].name,usuario.part)==0){
                start=mabore.partition[j].start;
                fseek(fich,start,SEEK_SET);
                fread(&sb,sizeof(sb),1,fich);
                break;
            }
            if(j==3)errR("mkgrp","partition not found");
        }

        char aux[15];
        quitar(aux, grp);
        char aux2[15];
        quitar(aux2, usr);
        char aux3[15];
        quitar(aux3, pwd);
        minusculas(aux);
        minusculas(aux2);
        minusculas(aux3);
        char tempo[255]="";
        strcat(tempo,aux);
        strcat(tempo,",");
        strcat(tempo,aux2);
        strcat(tempo,",");
        strcat(tempo,aux3);

        struct Journaling journal={3,0,"tempo",0,time(NULL),0,0,0};
        strcpy(journal.name,tempo);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);
        int ret=make_user(aux,aux2,aux3,&sb,fich);
        fclose(fich);
        return ret;

    }
    else return errR("mkgrp","no user has logged in");

    return 0;
}
int MKDIR(char strs[20][256], int j){
    char path[255]="";
    bool p=false;
    int lim=0;
    for(int i=0;i<20 && strs[i][0]!=0;i++, lim++);
    for(int i=1;i<lim;i++){
        if(strs[i][0]=='-'){
            if(strcmp(strs[i],"-path")==0){
                strcpy(path, strs[i+1]);
            }
            else if(strcmp(strs[i],"-p")==0){
                p=true;
            }
            else return errR(strs[0],"parameter not found");
        }
    }
    if(usuario.UID!=0){
        FILE *fich;
        struct mbr mabore;
        if((fich = fopen(usuario.path,"rb+"))==NULL){
            if(fich !=NULL) fclose(fich);
            return errR("login","cant open disk");
        }
        fread(&mabore,sizeof(mabore),1,fich);
        int start=0;
        struct Super_Block sb;
        for(int j=0;j<4;j++){
            if(strcmp(mabore.partition[j].name,usuario.part)==0){
                start=mabore.partition[j].start;
                fseek(fich,start,SEEK_SET);
                fread(&sb,sizeof(sb),1,fich);
                break;
            }
            if(j==3)errR("mkgrp","partition not found");
        }

        char aux[256];
        quitar(aux, path);
        repla2(aux);

        struct Journaling journal={4,p,"aux",0,time(NULL),usuario.UID,usuario.GID,0664};
        strcpy(journal.name,aux);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);

        int ret=make_directory(aux, p ,&sb,fich);
        fclose(fich);
        return ret;

    }
    else return errR("mkgrp","no user has logged in");

    return 0;
    printf("%s\n",path);
}
int MKFILE(char strs[20][256], int j){
    char path[255]="";
    int size=0;
    char cont[255]="";
    bool p=false;
    bool tam=true;
    int lim=0;
    for(int i=0;i<20 && strs[i][0]!=0;i++, lim++);
    for(int i=1;i<lim;i++){
        if(strs[i][0]=='-'){
            if(strcmp(strs[i],"-path")==0){
                strcpy(path, strs[i+1]);
            }
            else if(strcmp(strs[i],"-p")==0){
                p=true;
            }
            else if(strcmp(strs[i],"-size")==0){
                size=strtol(strs[i+1], NULL,10);
                if(size<=0) return errR(strs[0],"size not valid");
            }
            else if(strcmp(strs[i],"-cont")==0){
                strcpy(cont, strs[i+1]);
                tam=false;
            }
            else return errR(strs[0],"parameter not found");
        }
    }
    if(usuario.UID!=0){
        FILE *fich;
        struct mbr mabore;
        if((fich = fopen(usuario.path,"rb+"))==NULL){
            if(fich !=NULL) fclose(fich);
            return errR("login","cant open disk");
        }
        fread(&mabore,sizeof(mabore),1,fich);
        int start=0;
        struct Super_Block sb;
        for(int j=0;j<4;j++){
            if(strcmp(mabore.partition[j].name,usuario.part)==0){
                start=mabore.partition[j].start;
                fseek(fich,start,SEEK_SET);
                fread(&sb,sizeof(sb),1,fich);
                break;
            }
            if(j==3)errR("mkgrp","partition not found");
        }

        char aux[256];
        quitar(aux, path);
        repla2(aux);
        char aux2[256];
        quitar(aux2, cont);

        struct Journaling journal={5,p,"aux",0,time(NULL),size,0,0};
        strcpy(journal.name,aux);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);

        struct Journaling journal2={5,tam,"aux2",0,time(NULL),usuario.UID,usuario.GID,0664};
        strcpy(journal.name,aux2);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal2,sizeof(journal2),1,fich);


        int ret=make_file(aux, p, size, aux2, tam, &sb, fich);
        fclose(fich);
        return ret;

    }
    else return errR("mkgrp","no user has logged in");

    return 0;
}
int REM(char strs[20][256], int j){
    char path[256];
    if(strcmp(strs[1],"-path")==0){
        strcpy(path, strs[2]);
    }
    else
        return errR(strs[0],"parameter not found");
    if(usuario.UID!=0){
        FILE *fich;
        struct mbr mabore;
        if((fich = fopen(usuario.path,"rb+"))==NULL){
            if(fich !=NULL) fclose(fich);
            return errR("login","cant open disk");
        }
        fread(&mabore,sizeof(mabore),1,fich);
        int start=0;
        struct Super_Block sb;
        for(int j=0;j<4;j++){
            if(strcmp(mabore.partition[j].name,usuario.part)==0){
                start=mabore.partition[j].start;
                fseek(fich,start,SEEK_SET);
                fread(&sb,sizeof(sb),1,fich);
                break;
            }
            if(j==3)errR("mkgrp","partition not found");
        }

        char aux[256];
        quitar(aux, path);

        struct Journaling journal={6,0,"aux",0,time(NULL),0,0,0};
        strcpy(journal.name,aux);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);

        int ret=remover(aux, &sb, fich);
        fclose(fich);
        return ret;

    }
    else return errR("mkgrp","no user has logged in");

    return 0;
}
int MV(char strs[20][256], int j){
    char path[255]="";
    char dest[255]="";
    for(int i=1;i<j+1;i++){
        if(strcmp(strs[2*i-1],"-path")==0){
            strcpy(path, strs[2*i]);
        }
        else if(strcmp(strs[2*i-1],"-dest")==0){
            strcpy(dest, strs[2*i]);
        }
        else return errR(strs[0],"parameter not found");
    }
    if(usuario.UID!=0){
        FILE *fich;
        struct mbr mabore;
        if((fich = fopen(usuario.path,"rb+"))==NULL){
            if(fich !=NULL) fclose(fich);
            return errR("login","cant open disk");
        }
        fread(&mabore,sizeof(mabore),1,fich);
        int start=0;
        struct Super_Block sb;
        for(int j=0;j<4;j++){
            if(strcmp(mabore.partition[j].name,usuario.part)==0){
                start=mabore.partition[j].start;
                fseek(fich,start,SEEK_SET);
                fread(&sb,sizeof(sb),1,fich);
                break;
            }
            if(j==3)errR("mkgrp","partition not found");
        }

        char aux[256];
        quitar(aux, path);
        repla2(aux);
        char aux2[256];
        quitar(aux2, dest);
        repla2(aux2);

        struct Journaling journal={7,0,"aux",0,time(NULL),0,0,0};
        strcpy(journal.name,aux);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal,sizeof(journal),1,fich);

        struct Journaling journal2={7,0,"aux2",0,time(NULL),0,0,0};
        strcpy(journal.name,aux2);
        fseek(fich,start+80+(counter++)*sizeof(journal),SEEK_SET);
        fwrite(&journal2,sizeof(journal2),1,fich);

        int ret=mover(aux, aux2, &sb, fich);
        fclose(fich);
        return ret;

    }
    else return errR("mkgrp","no user has logged in");

    return 0;
}
int LOSS(char strs[20][256], int j){
    char id[256];
    if(strcmp(strs[1],"-id")==0){
        strcpy(id, strs[2]);
    }
    else
        return errR(strs[0],"parameter not found");
    for(mon_num=0;mon_num<200;mon_num++){
        for(int i=0;i<4;i++){
            if(strcmp(mon[mon_num].id[i],id)==0){
                char disk[256];
                char part[256];
                strcpy(disk,mon[mon_num].path);
                strcpy(part,mon[mon_num].part[i]);

                FILE *fich;
                struct mbr mabore;
                for(unsigned int i=1;i<strlen(disk);i++)
                    if(disk[i]==' ') disk[i]='_';
                if((fich = fopen(disk,"rb+"))==NULL){
                    if(fich !=NULL) fclose(fich);
                    return errR("login","cant open disk");
                }
                fread(&mabore,sizeof(mabore),1,fich);
                int start=0;
                struct Super_Block sb;
                for(int j=0;j<4;j++){
                    if(strcmp(mabore.partition[j].name,part)==0){
                        start=mabore.partition[j].start;
                        fseek(fich,start,SEEK_SET);
                        fread(&sb,sizeof(sb),1,fich);
                        break;
                    }
                    if(j==3)errR("mkfs","partition not found");
                }
                tronar(&sb,fich);
                fclose(fich);
            }
        }
        if(mon[mon_num].path[0]==0)errR("login","mount not found");
    }
    return 0;
}
int RECOVERY(char strs[20][256], int j){
    char id[256];
    if(strcmp(strs[1],"-id")==0){
        strcpy(id, strs[2]);
    }
    else
        return errR(strs[0],"parameter not found");
    for(mon_num=0;mon_num<200;mon_num++){
        for(int i=0;i<4;i++){
            if(strcmp(mon[mon_num].id[i],id)==0){
                char disk[256];
                char part[256];
                strcpy(disk,mon[mon_num].path);
                strcpy(part,mon[mon_num].part[i]);

                FILE *fich;
                struct mbr mabore;
                for(unsigned int i=1;i<strlen(disk);i++)
                    if(disk[i]==' ') disk[i]='_';
                if((fich = fopen(disk,"rb+"))==NULL){
                    if(fich !=NULL) fclose(fich);
                    return errR("login","cant open disk");
                }
                fread(&mabore,sizeof(mabore),1,fich);
                int start=0;
                struct Super_Block sb;
                for(int j=0;j<4;j++){
                    if(strcmp(mabore.partition[j].name,part)==0){
                        start=mabore.partition[j].start;
                        fseek(fich,start,SEEK_SET);
                        fread(&sb,sizeof(sb),1,fich);
                        break;
                    }
                    if(j==3)errR("mkfs","partition not found");
                }
                recuperar(&sb,fich);
                fclose(fich);
            }
        }
        if(mon[mon_num].path[0]==0)errR("login","mount not found");
    }
    return 0;
}















