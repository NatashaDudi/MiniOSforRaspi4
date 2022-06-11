// inspired by https://www.javatpoint.com/random-function-in-c

#include <stdio.h>  
#include <stdlib.h>     
int main()  
{  

    int number;  

    printf ("Get 100 random numbers from 1 to 100 \n");  
    for (int i = 0; i < 100; i++) {  
        number = rand() % 100 + 1; 
        printf ("%d,", number);  
    } 
    
    printf ("\n\n");  
    printf ("Get 100 random numbers from 0 to 3 \n");  
    for (int i = 0; i < 100; i++) {  
        number = rand() % 4; 
        printf ("%d,", number);  
    }  

} 
