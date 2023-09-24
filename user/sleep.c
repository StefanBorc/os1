#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc,char *argv[]){
  if(argc>0){
    int n;
    n = 0;                         
    while('0' <= *argv[0] && * argv[0]<= '9') {
    n = n*10 + *argv[0]++ - '0';         
    }  
    sleep(n);
    exit(0);
    }
    else{
      exit(1);
    }                                
  

}
