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

bool flag=true;
char cmd[256];
char err[256];
char dat[256];
int mon_num=0;
FILE* fopen_dir(char path[256]);
FILE* fopen_dir_txt(char path[256]);
void strip(char *str, int size );
int analize(char *str);
void limpiar(char temp[256]);
int crear_disco(int size, char fit, int unit, char path[256]);
int crear_part(int size, char fit, int unit, char type, char path[256], char name[256]);
char* concat(const char *s1, const char *s2);
int put_ebr(struct ebr *exbore, FILE *fich,struct Part *particion);
void do_disk_graph(struct mbr *mabore,FILE *rep,char text[256],char png[256],char path[256]);

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
    else if(strcmp(strs[0],"rmdisk")==0){//eliminar disco
        char g;
        printf("Are you sure to remove disk? (y/n)");
        fgets(cmd,256,stdin);
        g=cmd[0];
        if(g=='y' || g=='Y'){
            if(strcmp(strs[1],"-path")==0){
                char dir[256];
                quitar(dir, strs[2]);
                int status;
                for(int i=1;i<strlen(dir);i++)
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
    }
    else if(strcmp(strs[0],"mount")==0){//montar disco
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
                        for(int ii=1;ii<strlen(path);ii++)
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
                        for(int ii=1;ii<strlen(path);ii++)
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
    }
    else if(strcmp(strs[0],"unmount")==0){//desmontar disco
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
                    for(int ii=1;ii<strlen(path);ii++)
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
    }
    else if(strcmp(strs[0],"pause")==0){//pequeña pausa
        printf("Press ENTER key to Continue\n");
        fgets(cmd,256,stdin);
    }
    else if(strcmp(strs[0],"rep")==0){//reporte de discos
        char id[256];
        char path_rep[256];
        char name[10];
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
                        for(int ii=1;ii<strlen(path_);ii++)
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
                        for(int ii=1;ii<strlen(path_);ii++)
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
        else
            return errR(strs[0],"parameter of rep not found");
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
FILE* fopen_dir_txt(char path[256]){
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
    fputc('/0',disco);

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
    for(int i=1;i<strlen(path);i++)
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
    for(int i=1;i<strlen(path);i++)
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
                /*
                rewind(fich);
                fseek(fich,init,SEEK_CUR);
                for(int i=0;i<tam;i++){
                    fputc('/0',fich);
                    fseek(fich,1,SEEK_CUR);
                }
                */
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
                        struct ebr tem;
                        fseek(fich,mabore.partition[j].start,SEEK_SET);
                        fread(&temp,sizeof(temp),1,fich);
                        int ss=temp.next;
                        while(ss!=-1){
                            if(strcmp(temp.name,name)==0){

                                /*
                                rewind(fich);
                                int init=temp.start+sizeof(temp);
                                int tam=temp.size-sizeof(temp);
                                fseek(fich,init,SEEK_CUR);
                                for(int i=0;i<tam;i++){
                                    fputc('/0',fich);
                                    fseek(fich,1,SEEK_CUR);
                                }
                                */
                                temp.fit='/0';
                                strcpy(temp.name,"LIBRE");
                                temp.status=0;

                                struct ebr temp_2;
                                fseek(fich,temp.next,SEEK_SET);
                                fread(&temp_2,sizeof(temp_2),1,fich);

                                if(temp_2.status==0){
                                    temp.next=temp_2.next;
                                    temp.size+=temp_2.size;
                                }
                                else if(tem.status==0){
                                    tem.next=temp.next;
                                    tem.size+=temp.size;
                                    fseek(fich,tem.start,SEEK_SET);
                                    fwrite(&tem,sizeof(tem),1,fich);
                                    fclose(fich);
                                    return 0;
                                }

                                fseek(fich,temp.start,SEEK_SET);
                                fwrite(&temp,sizeof(temp),1,fich);
                                fclose(fich);
                                return 0;
                            }
                            else{
                                tem=temp;
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
                        struct ebr tem;
                        fseek(fich,mabore.partition[j].start,SEEK_SET);
                        fread(&temp,sizeof(temp),1,fich);
                        int ss=temp.next;
                        while(ss!=-1){
                            if(strcmp(temp.name,name)==0){

                                temp.fit='/0';
                                strcpy(temp.name,"LIBRE");
                                temp.status=0;

                                struct ebr temp_2;
                                fseek(fich,temp.next,SEEK_SET);
                                fread(&temp_2,sizeof(temp_2),1,fich);

                                if(temp_2.status==0){
                                    temp.next=temp_2.next;
                                    temp.size+=temp_2.size;
                                }
                                else if(tem.status==0){
                                    tem.next=temp.next;
                                    tem.size+=temp.size;
                                    fseek(fich,tem.start,SEEK_SET);
                                    fwrite(&tem,sizeof(tem),1,fich);
                                    fclose(fich);
                                    return 0;
                                }

                                fseek(fich,temp.start,SEEK_SET);
                                fwrite(&temp,sizeof(temp),1,fich);
                                fclose(fich);
                                return 0;
                            }
                            else{
                                tem=temp;
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
    char tempo[20];
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
            struct ebr exbore={0,'/0',0,0,0,""};
            for(int i=1;i<strlen(path);i++)
                if(path[i]==' ') path[i]='_';
            if((fich = fopen(path,"rb+"))==NULL){
                if(fich !=NULL) fclose(fich);
                return errR("rep","cant open disk");
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
char* concat(const char *s1, const char *s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
void cl(char temp[20]){
    for(int i=0;i<20;i++)
        temp[i]=0x0;
}
void do_disk_graph(struct mbr *mabore,FILE *rep,char text[256],char png[256],char path[256]){
    char *graph="";
    char tempo[20];
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
            struct ebr exbore={0,'/0',0,0,0,""};
            for(int i=1;i<strlen(path);i++)
                if(path[i]==' ') path[i]='_';
            if((fich = fopen(path,"rb+"))==NULL){
                if(fich !=NULL) fclose(fich);
                return errR("rep","cant open disk");
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
int put_ebr(struct ebr *exbore, FILE *fich,struct Part *particion){
    /* exbore -> nuevo
     * mabore -> mbr
     * fich -> fichero
     * start -> comienza extendida
     * temp -> ebr existente if(-1)no hay siguiente
     * ss -> siguiente ebr
     */
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
        int size_ant=particion->start;
        int space=0;
        int start_space=size_ant;
        while(ss!=-1){
            if(temp.status==0 && temp.size>exbore->size && space<temp.start-size_ant){
                space=temp.start-size_ant;
                start_space=size_ant;
            }
            size_ant=temp.start+temp.size;
            fseek(fich,ss,SEEK_SET);
            fread(&temp,sizeof(temp),1,fich);
            ss=temp.next;
        }
        if(space!=0){
            exbore->start=temp.start;
            exbore->next=ss;
            return 0;
        }
    }
    else{
        int size_ant=particion->start;
        int space=0x7fffffff;
        int start_space=size_ant;
        while(ss!=-1){
            if(temp.status==0 && temp.size>exbore->size && space>temp.start-size_ant){
                space=temp.start-size_ant;
                start_space=size_ant;
            }
            size_ant=temp.start+temp.size;
            fseek(fich,ss,SEEK_SET);
            fread(&temp,sizeof(temp),1,fich);
            ss=temp.next;
        }
        if(space!=0x7fffffff){
            exbore->start=temp.start;
            exbore->next=ss;
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
