#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 1000
#define MAX_DIRECTORYNAME_LENGTH 1000
#define MAX_COMMIT_MESSAGE_LENGTH 72
#define MAX_LINE_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1000
#define MAX_MODIFIED_DATE_LENGTH 30

bool global = false;


int run_init(int argc , char* argv[]);
int get_config(int argc , char* argv[]);
int create_config(FILE* file);
//
int run_add(int argc , char* argv[]);
int add_to_staging(char *filepath , char* modified_date);
//
int check_for_existing(char* argv[]);
//
int run_commit(int argc , char*argv[]); //OK
int inc_last_commit_ID();//ok
int commit_staged_file(int commit_ID , char* filepath);//ok
int track_file(char* filepath);//ok
bool is_tracked(char* filepath);//ok
int create_commit_file(int commit_ID , char* message);//OK
int find_file_last_commit(char* filepath);//ok
int find_file_last_change_before_commit(char* filepath , int commit_ID);//not sure
//
int run_reset(int argc , char* argv[]);
int remove_from_staging(char* filepath);
//
int run_checkout(int argc , char* argv[]);
int checkout_file(char *filepath , int commit_ID);



int main(int argc , char* argv[]){
    if(argc < 2){
        perror("enter a valid command!");
    }
    if(strcmp(argv[1] , "config") == 0){
        get_config(argc , argv);
    } else if(strcmp(argv[1] , "init") == 0){
        if(!global){
            perror("first set a username and email!");
        }
        run_init(argc , argv);
    } else if(strcmp(argv[1] , "add") == 0){
        run_add(argc , argv);
    } else if(strcmp(argv[1] , "reset") == 0){
        run_reset(argc,argv);
    } else if(strcmp(argv[1] , "commit") == 0){
        run_commit(argc,argv);
    }
    return 0;
}









int get_config(int argc , char* argv[]){
    
    if(argc<4){
        perror("enter a valid command!");
        return 1;
    } 
    if(argc == 4){

        FILE *file ;
        if(strstr( argv[3] , "name") != NULL){
            file = fopen("set_configs.txt" , "w");
            char username[100];
            
    
            sscanf(argv[4], "\"%s[^\"]\"", username);
            if (file != NULL) {
                fprintf(file, "%s\n", username);
                fclose(file);
            }
        } else if(strstr( argv[3] , "email") != NULL){
            file = fopen("set_configs.txt" , "a");
            char email[100];
    
            sscanf(argv[4], "\"%s[^\"]\"", email);
            if (file != NULL) {
                fprintf(file, "%s\n", email);
                fclose(file);
            }
        }
    } else if(argc == 5){
        FILE *file ;
        if(strstr(argv[3] , "name") != NULL){
            file = fopen("set_configs.txt" , "w");
            char username[100];
            
    
            sscanf(argv[4], "\"%s[^\"]\"", username);
            if (file != NULL) {
                fprintf(file, "%s\n", username);
                fclose(file);
            }
        } else if(strstr(argv[3] , "email") != NULL){
            file = fopen("set_configs.txt" , "a");
            char email[100];
    
            sscanf(argv[4], "\"%s[^\"]\"", email);
            if (file != NULL) {
                fprintf(file, "%s\n", email);
                fclose(file);
            }
        }
        global = true;

    }
    return 0;
}

int run_init(int argc , char* argv[]){
    if(argc < 3){
        perror("enter a valid command!");
    }
    char cwd[MAX_FILENAME_LENGTH];
    if(getcwd(cwd , sizeof(cwd)) == NULL) return 1;
    char tmp_dir[MAX_FILENAME_LENGTH];
    bool exist = false;
    do{
        DIR *dir = opendir(".");
        if(dir == NULL) return 1;
        struct dirent *entry;

        while(entry = readdir(dir) != NULL){
            if(entry ->d_type == DT_DIR && strcmp(entry->d_name , ".neogit") == 0){
                exist = true;
            }
        }
        closedir(dir);
        if(getcwd(tmp_dir , sizeof(tmp_dir) == NULL)) return 1;
        if(strcmp(tmp_dir , "/") != 0){
            if(chdir("..") != 0) return 1;
        }
    } while(strcmp(tmp_dir , "/") != 0);
    if(chdir(cwd) != 0 ) return 1;
    if(exist){
        perror("neogit repo already exists!");
    } else{
        if(mkdir(".neogit" , 0755) != 0) return 1;
        create_config("set_configs.txt");
        FILE* file1 = fopen(".neogit/staging" , "w");
    }
    return 0;
}
int create_config(FILE* file){
    char username[100] , email[100];
    file = fopen("set_configs.txt" , "r");
    if(fgets(username , sizeof(username) , file) == NULL) return 1;
    if(fgets(email , sizeof(email) , file) == NULL) return 1;
    FILE* output = fopen(".neogit/configs" , "w");
    fprintf(output , "username : %s\n" , username);
    fprintf(output , "email : %s\n" , email);
    fprintf(output,"branch : master\n");
    fclose(output);
}

int run_add(int argc , char* argv[]){
    if(argc < 3){
        perror("enter a valid command!");
        return 1;
    } else if(argc == 3){
        int length = strlen(argv[2]);
        int flag = 1;
        if(strstr(argv[2] , "*") != NULL) flag = 0;
        if(flag){
            if(check_for_existing(argv[2])){
                perror("directory doesn't exists!");
            }
            FILE* for_undo = fopen(".neogit/undo_operation.txt","w");
            char *filepath = argv[2];
            struct stat filepath_stat;
            if(stat(filepath , &filepath) != 0 ) return 1;
            if(S_ISDIR(filepath_stat.st_mode)){
                DIR *dir = opendir(filepath);
                if(dir == NULL) return 1;
                struct dirent *entry;

                while(entry = readdir(dir) != NULL){
                    if(entry ->d_type != DT_DIR){
                        fprintf(for_undo , "%s\n" , entry);
                        struct tm dt;
                        dt = *(gmtime(&filepath_stat.st_mtime));
                        char modified_date[MAX_MODIFIED_DATE_LENGTH];
                        sprintf(modified_date , "%d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);
                        add_to_staging(entry , modified_date);
                    }
                }
                fclose(for_undo);
                closedir(dir);
            } else{
                FILE* for_undo = fopen(".neogit/undo_operation.txt","w");
                fprintf(for_undo , "%s\n" , filepath);
                fclose(for_undo);
                struct tm dt;
                dt = *(gmtime(&filepath_stat.st_mtime));
                char modified_date[MAX_MODIFIED_DATE_LENGTH];
                sprintf(modified_date , "%d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);
                add_to_staging(filepath , modified_date);
            }
            
            
            
        } else {
            //TODO FOR WILDCARTS
        }
        
    } else if(argc == 4){
        //TODO FOR DEPTH
    } else{
        for(int i = 3 ; i < argc ; i++){
            if(check_for_existing(argv[2])){
                perror("directory doesn't exists!");
            }
            char *filepath = argv[2];
            struct stat filepath_stat;
            if(stat(filepath , &filepath) != 0 ) return 1;
            if(S_ISDIR(filepath_stat.st_mode)){
                DIR *dir = opendir(filepath);
                if(dir == NULL) return 1;
                struct dirent *entry;
                FILE* for_undo = fopen(".neogit/undo_operation.txt","w");
                while(entry = readdir(dir) != NULL){
                    if(entry ->d_type != DT_DIR){
                        fprintf(for_undo , "%s\n" , filepath);
                        struct tm dt;
                        dt = *(gmtime(&filepath_stat.st_mtime));
                        char modified_date[MAX_MODIFIED_DATE_LENGTH];
                        sprintf(modified_date , "%d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);
                        add_to_staging(entry , modified_date);
                    }
                }
                fclose(for_undo);
                closedir(dir);
            } else{
                FILE* for_undo = fopen(".neogit/undo_operation.txt","w");
                fprintf(for_undo , "%s\n" , filepath);
                fclose(for_undo);
                struct tm dt;
                dt = *(gmtime(&filepath_stat.st_mtime));
                char modified_date[MAX_MODIFIED_DATE_LENGTH];
                sprintf(modified_date , "%d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);
                add_to_staging(filepath , modified_date);
            }
        }
    }
    return 0;
}

int add_to_staging(char *filepath , char* modified_date) {
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(filepath, line) == 0) return 0;
    }
    fclose(file);
    file = fopen(".neogit/staging","r");
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        char *path = strtok(line, " ");
        if (strcmp(path, filepath) == 0) {
            found = 1;
            fseek(file, -strlen(line), SEEK_CUR);
            fprintf(file, "%s modified on : %s\n", filepath, modified_date);
            break;
        }
    }

    if (!found) {
        fclose(file);
        file = fopen(".neogit/staging", "a");
        if (file == NULL) {
            perror("fopen");
            return 1;
        }
        fprintf(file, "%s modified on : %s\n", filepath, modified_date);
    }

    fclose(file);
    
    
    fclose(file);

    return 0;
}

int check_for_existing(char* argv[]){
    char file_name[MAX_FILENAME_LENGTH];
    if(strcmp(argv[2] , ".") != 0 ){
        strcpy(file_name , argv[2]);
        char cwd[MAX_FILENAME_LENGTH];
        if(getcwd(cwd , sizeof(cwd)) == NULL) return 1;
        char tmp_dir[MAX_FILENAME_LENGTH];
        bool exist = false;
        do{
            DIR *dir = opendir(".");
            if(dir == NULL) return 1;
            struct dirent *entry;

            while(entry = readdir(dir) != NULL){
                if(strcmp(entry->d_name , file_name) == 0){
                    exist = true;
                }
            }
            closedir(dir);
            if(getcwd(tmp_dir , sizeof(tmp_dir) == NULL)) return 1;
            if(strcmp(tmp_dir , "/") != 0){
                if(chdir("..") != 0) return 1;
            }
        } while(strcmp(tmp_dir , "/") != 0);
        if(chdir(cwd) != 0 ) return 1;
        if(exist){
            return 0;
        } else{
            return 1;
        }
    }
}
int run_reset(int argc , char* argv[]){
    if(argc < 3){
        perror("enter a valid command!");
    }
    if(check_for_existing(argv[2])){
        perror("directory doesn't exists!");
    }
    if(strstr(argv[2] , "-") != NULL){
        char line[MAX_LINE_LENGTH];
        FILE* fp = fopen(".neogit/undo_operation.txt" , "r");
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line , "\n")] = 0;
            remove_from_staging(line);
        }

        fclose(fp);
    }else{
        char *filepath = argv[2];
        struct stat filepath_stat;
        if(stat(filepath , &filepath) != 0 ) return 1;
        if(S_ISDIR(filepath_stat.st_mode)){
            DIR *dir = opendir(filepath);
            if(dir == NULL) return 1;
            struct dirent *entry;

            while(entry = readdir(dir) != NULL){
                if(entry ->d_type != DT_DIR){
                    remove_from_staging(entry);
                }
            }
            closedir(dir);
        }else{
            remove_from_staging(filepath);
        }
    }
    
    return 0;
}
int remove_from_staging(char* filepath){
    FILE *fp, *temp;
    char filename[] = ".neogit/staging.txt";
    char tempfilename[] = ".neogit/temp.txt";
    char line[100];

    fp = fopen(filename, "r");
    temp = fopen(tempfilename, "w");

    if (fp == NULL || temp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, filepath) == NULL) {
            fputs(line, temp);
        }
    }
    fclose(fp);
    fclose(temp);

    remove(filename);
    rename(tempfilename, filename);
    return 0;
}
int run_commit(int argc , char*argv[]){
    if(argc < 4){
        perror("use the correct format please!");
        return 1;
    }
    char message[MAX_COMMIT_MESSAGE_LENGTH];
    strcpy(message,argv[3]);
    int commit_ID = inc_last_commit_ID();
    if(commit_ID == -1) return 1;
    FILE* file = fopen(".neogit/staging.txt" , "r");
    if(file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while(fgets(line , sizeof(line) , file) != NULL){
        int length = strlen(line);
        if(line[length-1] == '\n'){
            line[length-1] = '\0';
        }
        if(!check_for_existing(line)){
            char dir_path[MAX_FILENAME_LENGTH];
            strcpy(dir_path , ".neogit/files/");
            strcat(dir_path , line);
            if(mkdir(dir_path , 0755) != 0) return 1;
        }
        commit_staged_file(commit_ID,line);
        track_file(line);
    }
    fclose(file);
    file = fopen(".neogit/staging.txt" , "w");
    if(file == NULL) return 1;
    fclose(file);
    create_commit_file(commit_ID , message);
    fprintf(stdout , "commit successfuly with commit ID %d" , commit_ID);
    return 0;
}
int create_commit_file(int commit_ID , char* message){
    char commit_filepath[MAX_FILENAME_LENGTH];
    strcpy(commit_filepath , ".neogit/commits/");
    char temp[10];
    sprintf(temp,"%d" , commit_ID);
    strcat(commit_filepath , temp);
    FILE *file = fopen(commit_filepath , "w");
    if(file == NULL) return 1;
    fprintf(file , "message: %s\n" , message);
    fprintf(file , "files:\n");
    DIR *dir = opendir(".");
    struct dirent *entry;
    if(dir == NULL){
        perror("Eroor opening current directory");
        return 1;
    }
    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_REG && is_tracked(entry->d_name)){
            int file_last_commit_ID = find_file_last_commit(entry->d_name);
            fprintf(file , "%s %s\n" , entry->d_name , file_last_commit_ID);
        }
    }
    closedir(dir);
    fclose(file);
    return 0;
}
int find_file_last_commit(char* filepath){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir , ".neogit/files/file/");
    strcat(filepath_dir , filepath);
    int max = -1;
    DIR* dir = opendir(filepath_dir);
    struct dirent *entry;
    if(dir == NULL) return 1;
    while((entry = readdir(dir)) == NULL){
        if(entry->d_type == DT_REG){
            int tmp = atoi(entry->d_name);
            max = max>tmp ? max : tmp;
        }
    }
    closedir(dir);
    return max;
}
int inc_last_commit_ID(){
    FILE* file = fopen(".neogit/configs" , "w");
    if(file == NULL) return -1;

    FILE *tmp_file = fopen(".neogit/tmp_configs" , "w");
    if(tmp_file == NULL) return -1;

    int last_commit_ID;
    char line[MAX_LINE_LENGTH];
    while(fgets(line , sizeof(line) , file) != NULL){
        if(strncmp(line , "last_commit_ID" , 14) == 0){
            sscanf(line , "last_commit_ID: %d\n" , &last_commit_ID);
            last_commit_ID++;
            fprintf(tmp_file , "last_commit_ID: %d\n" , last_commit_ID);
        } else fprintf(tmp_file , "%s" , line);
    }
    fclose(file);
    fclose(tmp_file);
    remove(".neogit/configs");
    rename(".neogit/tmp_configs" ,".neogit/configs" );
    return last_commit_ID;
}
int find_file_last_change_before_commit(char* filepath , int commit_ID){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/files/");
    strcat(filepath_dir , filepath);
    int max = -1;
    DIR* dir = opendir(filepath_dir);
    if(dir == NULL) return 1;
    struct dirent *entry;
    while((entry = readdir(dir)) == NULL){
        if(entry->d_type == DT_REG){
            int tmp = atoi(entry->d_name);
            max = max>tmp ? max : tmp;
        }
    }
    closedir(dir);
    return max;

}
bool is_tracked(char* filepath){
    FILE* file = fopen(".neogit/tracks" , "r");
    if(file == NULL) return false;
    char line[MAX_LINE_LENGTH];
    while(fgets(line , sizeof(line) , file) != NULL){
        int length = strlen(line);
        if(line[length-1] == '\n'){
            line[length-1] == '\0';
        }
        if(strcmp(line , filepath) == 0) return true;
        break;

    }
    fclose(file);
    return false;
}
int track_file(char* filepath){
    if(is_tracked(filepath)) return 0;
    FILE* file = fopen(".neogit/tracks" , "a");
    if(file == NULL) return 1;
    fprintf(file , "%s\n" , filepath);
    return 0;
}
int commit_staged_file(int commit_ID , char* filepath){
    FILE *read_file,*write_file;
    char read_path[MAX_FILENAME_LENGTH];
    strcpy(read_path , filepath);
    char write_path[MAX_FILENAME_LENGTH];
    strcpy(write_path , ".neogit/files/");
    strcat(write_path , filepath);
    strcat(write_path , "/");
    char tmp[10];
    sprintf(tmp , "%d" , commit_ID);
    strcat(write_path , tmp);

    read_file = fopen(read_path , "r");
    if(read_file == NULL) return 1;
    write_file = fopen(write_path , "w");
    if(write_file == NULL) return 1;
    char buffer = fgetc(read_file);
    while(buffer != EOF){
        fputs(buffer , write_file);
        buffer = fgetc(read_file);
    }
    fclose(read_file);
    fclose(write_file);
    return 0;
}
