#include <stdio.h>
int main(){
    int count = 0;
#pragma omp parallel reduction (+:count)
    { 
      	count++; 
	 printf("Current value %d\n",count); 
    }
    printf("It were %d threads\n",count); 
}
