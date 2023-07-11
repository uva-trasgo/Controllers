#include <stdio.h>
#include <string.h>

void main(int argc, char * argv[]) {
        FILE * f = fopen(argv[1], "r");

        //static const char file_contents[2000];
        char file_contents[2000];
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);

        fread(file_contents, size, 1, f);
        char * v = file_contents;
        char * m, * p, * p2;

        while(strstr(v, "CTRL_KERNEL_PROTO(") != NULL){
            m = strstr(v, "CTRL_KERNEL_PROTO(");
            p = strchr(m, ' '); // Start
                                                                                                                                                              
            // Skip extra spaces                                                                                                                              
            while(*p == ' ') p++;                                                                                                                             
            p--;                                                                                                                                              
                                                                                                                                                              
            if (!p) {                                                                                                                                         
                v += 2;                                                                                                                                       
                continue;                                                                                                                                     
            }                                                                                                                                                 
            p++;                                                                                                                                              
            p2 = strchr(p , ','); // End                                                                                                                      
            if (!p2) {                                                                                                                                        
                v = p;                                                                                                                                        
                continue;                                                                                                                                     
            }                                                                                                                                                 
            *p2 = '\0';                                                                                                                                       
            printf("%s\n", p);                                                                                                                                
            v = p2+1;                                                                                                                                         
        }                                                                                                                                                     
}   
