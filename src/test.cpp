#include <string>
#include <stdio.h>
#include <string.h>

using namespace std;

void getAllObjectsInFile1(const char* filename){

    FILE *f = fopen(filename, "r");

    char* aux=0;
    char obj_names[20][50]={};

    int i = 0;

    while(fscanf(f, "%s ", aux) == 1){
        if(strcmp(aux, "o") == 0){
            fscanf(f, "%s", obj_names[i]);
            printf("%s", obj_names[i]);
            i++;
        }
    }

    fclose(f);
}

int main(){

    getAllObjectsInFile1("../../data/castle.obj");

    return 0;
}
