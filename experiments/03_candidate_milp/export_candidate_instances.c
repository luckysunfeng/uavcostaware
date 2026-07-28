#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define PI 3.14159265358979323846
static int ready=0;static double spare;
static double u01(void){return rand()/(RAND_MAX+1.0);}
static double n01(void){if(ready){ready=0;return spare;}double u=u01();if(u<1e-12)u=1e-12;double v=u01(),r=sqrt(-2*log(u));spare=r*sin(2*PI*v);ready=1;return r*cos(2*PI*v);}
static double clip(double x){return x<0?0:(x>1000?1000:x);}
int main(void){const int ns[2]={100,200};const double c[4][2]={{250,250},{750,250},{250,750},{750,750}};FILE*f=fopen("results/reproduced/generated_candidate_instances.csv","w");if(!f)return 1;fprintf(f,"N,seed,user,x,y\n");
 for(int a=0;a<2;a++)for(int run=0;run<30;run++){int n=ns[a],seed=10000+a*100+run;srand(seed);ready=0;for(int i=0;i<n;i++){int z=rand()%4;double x=clip(c[z][0]+120*n01()),y=clip(c[z][1]+120*n01());fprintf(f,"%d,%d,%d,%.10f,%.10f\n",n,seed,i,x,y);}}
 fclose(f);puts("saved results/reproduced/generated_candidate_instances.csv");return 0;}
