#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_N 500
#define MAX_K 128
#define RUNS 30
#define ITERS 20
#define CAPACITY 20
#define RMAX 500.0
#define RMIN 0.5
#define HMIN 30.0
#define HMAX 150.0
#define PI 3.14159265358979323846
#define PSO_MAX_PARTICLES 12

static int g_capacity=CAPACITY;
static double g_rmin=RMIN,g_hmin=HMIN,g_hmax=HMAX,g_lambda_k=100.0,g_eta_nlos=20.0;
static void reset_parameters(void){g_capacity=CAPACITY;g_rmin=RMIN;g_hmin=HMIN;g_hmax=HMAX;g_lambda_k=100.0;g_eta_nlos=20.0;}

typedef struct { double x,y; } Point;
typedef struct {
  int k, served, assign[MAX_N], load[MAX_K];
  Point p[MAX_K]; double h[MAX_K], rate[MAX_N];
  double throughput, utility;
} Sol;

static double u01(void){ return rand()/(RAND_MAX+1.0); }
static int normal_ready=0; static double normal_spare;
static double n01(void){
  if(normal_ready){normal_ready=0; return normal_spare;}
  double u=u01(); if(u<1e-12)u=1e-12; double v=u01(),r=sqrt(-2*log(u));
  normal_spare=r*sin(2*PI*v); normal_ready=1; return r*cos(2*PI*v);
}
static double clip(double x){return x<0?0:(x>1000?1000:x);}
static double d2(Point a,Point b){double x=a.x-b.x,y=a.y-b.y;return sqrt(x*x+y*y);}

static void users(int scenario,int n,Point *q){
  static const Point c[4]={{250,250},{750,250},{250,750},{750,750}};
  for(int i=0;i<n;i++){
    if(scenario==0){int z=rand()%4;q[i].x=clip(c[z].x+120*n01());q[i].y=clip(c[z].y+120*n01());}
    else if(scenario==1){q[i].x=1000*u01();q[i].y=1000*u01();}
    else if(scenario==2){
      if(u01()<0.75){q[i].x=clip(680+80*n01());q[i].y=clip(340+80*n01());}
      else {q[i].x=1000*u01();q[i].y=1000*u01();}
    } else {q[i].x=clip(100+800*u01());q[i].y=clip(180+0.55*q[i].x+45*n01());}
  }
}

static double pathloss(double horizontal,double h){
  const double fc=3.5e9,c=3e8,A=9.61,B=.16,C=2,elos=1;
  double d=sqrt(horizontal*horizontal+h*h),theta=atan2(h,horizontal+1e-9)*180/PI;
  double plos=1/(1+A*exp(-B*(theta-C)));
  return 20*log10(d)+20*log10(fc)+20*log10(4*PI/c)+plos*elos+(1-plos)*g_eta_nlos;
}
static double received(Point a,Point b,double h){
  if(d2(a,b)>RMAX)return 0;
  return pow(10,33.0/10.0)*pow(10,-pathloss(d2(a,b),h)/10.0);
}
static void evaluate(Sol *s,int n,Point *q){
  const double noise=pow(10,-100.0/10.0),W=20e6;
  memset(s->load,0,sizeof(s->load));s->served=0;s->throughput=0;
  for(int i=0;i<n;i++){
    double sig[MAX_K],total=0;for(int j=0;j<s->k;j++){sig[j]=received(q[i],s->p[j],s->h[j]);total+=sig[j];}
    int bj=-1;double br=0;
    for(int j=0;j<s->k;j++)if(s->load[j]<g_capacity&&sig[j]>0){
      double r=W*log(1+sig[j]/(noise+total-sig[j]))/log(2.0)/1e6;
      if(r>=g_rmin&&r>br){br=r;bj=j;}
    }
    s->assign[i]=bj;s->rate[i]=br;if(bj>=0){s->load[bj]++;s->served++;s->throughput+=br;}
  }
  s->utility=100000.0*s->served+s->throughput-g_lambda_k*s->k;
}
static double best_h(Point p,int n,Point *q,const int *member,int label){
  double bh=g_hmin,bs=-1;
  for(int z=0;z<=8;z++){double h=g_hmin+(g_hmax-g_hmin)*z/8.0,score=0;
    for(int i=0;i<n;i++)if(!member||member[i]==label)score+=received(q[i],p,h);
    if(score>bs){bs=score;bh=h;}
  } return bh;
}
static int cmp_x(const void*a,const void*b){const Point*p=a,*q=b;return(p->x>q->x)-(p->x<q->x);}
static int cmp_y(const void*a,const void*b){const Point*p=a,*q=b;return(p->y>q->y)-(p->y<q->y);}
static void gno(Sol*s,int n,Point*q){
  Point tmp[MAX_N];memcpy(tmp,q,n*sizeof(Point));int cell=g_capacity<10?g_capacity:10;s->k=(n+cell-1)/cell;
  /* Contiguous blocks after horizontal sorting; within-block ordering does not change the centroid. */
  qsort(tmp,n,sizeof(Point),cmp_x);
  for(int j=0;j<s->k;j++){int a=j*n/s->k,b=(j+1)*n/s->k;if(j&1)qsort(tmp+a,b-a,sizeof(Point),cmp_y);
    s->p[j]=(Point){0,0};for(int i=a;i<b;i++){s->p[j].x+=tmp[i].x;s->p[j].y+=tmp[i].y;}
    s->p[j].x/=(b-a);s->p[j].y/=(b-a);s->h[j]=best_h(s->p[j],b-a,tmp+a,NULL,0);
  } evaluate(s,n,q);
}
static void kmeans(Sol*s,int n,Point*q){
  s->k=(n+g_capacity-1)/g_capacity;int lab[MAX_N];
  for(int j=0;j<s->k;j++)s->p[j]=q[(j*n/s->k+n/2)%n];
  for(int it=0;it<25;it++){
    double sx[MAX_K]={0},sy[MAX_K]={0};int cnt[MAX_K]={0};
    for(int i=0;i<n;i++){int b=0;for(int j=1;j<s->k;j++)if(d2(q[i],s->p[j])<d2(q[i],s->p[b]))b=j;lab[i]=b;sx[b]+=q[i].x;sy[b]+=q[i].y;cnt[b]++;}
    for(int j=0;j<s->k;j++)if(cnt[j]){s->p[j].x=sx[j]/cnt[j];s->p[j].y=sy[j]/cnt[j];}
  }
  for(int j=0;j<s->k;j++)s->h[j]=best_h(s->p[j],n,q,lab,j);
  evaluate(s,n,q);
}
static void refine(Sol*s,int n,Point*q){
  Sol old=*s;
  for(int j=0;j<s->k;j++){double x=0,y=0;int c=0;for(int i=0;i<n;i++)if(s->assign[i]==j){x+=q[i].x;y+=q[i].y;c++;}
    if(c){Point target={x/c,y/c};double d=d2(target,s->p[j]);if(d>80){target.x=s->p[j].x+80*(target.x-s->p[j].x)/d;target.y=s->p[j].y+80*(target.y-s->p[j].y)/d;}s->p[j]=target;s->h[j]=best_h(target,n,q,s->assign,j);}
  } evaluate(s,n,q);if(s->utility<old.utility)*s=old;
}
static void remove_one(Sol*s,int victim){for(int j=victim;j<s->k-1;j++){s->p[j]=s->p[j+1];s->h[j]=s->h[j+1];}s->k--;}
static void alns(Sol*s,int n,Point*q,int do_refine){
  for(int it=0;it<ITERS;it++){Sol cand=*s;if(cand.k<=1)break;int v=rand()%cand.k;
    if(!(it&1)){v=0;for(int j=1;j<cand.k;j++)if(cand.load[j]<cand.load[v])v=j;}
    remove_one(&cand,v);evaluate(&cand,n,q);if(do_refine&&!(it&1))refine(&cand,n,q);
    if(cand.served>=s->served&&cand.utility>=s->utility)*s=cand;
  }
}
/* Continuous PSO baseline at a controlled fleet size. The three particle/
   generation budgets are reported explicitly and are not presented as a
   tuned state-of-the-art implementation. */
static void pso_controlled(Sol *out,int n,Point*q,int k,int particles,int iterations){
  Sol particle[PSO_MAX_PARTICLES],pbest[PSO_MAX_PARTICLES],gbest;double vx[PSO_MAX_PARTICLES][MAX_K],vy[PSO_MAX_PARTICLES][MAX_K],vh[PSO_MAX_PARTICLES][MAX_K];
  memset(&gbest,0,sizeof(gbest));gbest.utility=-1e300;
  for(int a=0;a<particles;a++){
    particle[a].k=k;
    for(int j=0;j<k;j++){particle[a].p[j].x=1000*u01();particle[a].p[j].y=1000*u01();particle[a].h[j]=g_hmin+(g_hmax-g_hmin)*u01();vx[a][j]=vy[a][j]=vh[a][j]=0;}
    evaluate(&particle[a],n,q);pbest[a]=particle[a];if(pbest[a].utility>gbest.utility)gbest=pbest[a];
  }
  for(int it=0;it<iterations;it++)for(int a=0;a<particles;a++){
    for(int j=0;j<k;j++){
      vx[a][j]=.65*vx[a][j]+1.4*u01()*(pbest[a].p[j].x-particle[a].p[j].x)+1.4*u01()*(gbest.p[j].x-particle[a].p[j].x);
      vy[a][j]=.65*vy[a][j]+1.4*u01()*(pbest[a].p[j].y-particle[a].p[j].y)+1.4*u01()*(gbest.p[j].y-particle[a].p[j].y);
      vh[a][j]=.65*vh[a][j]+1.4*u01()*(pbest[a].h[j]-particle[a].h[j])+1.4*u01()*(gbest.h[j]-particle[a].h[j]);
      particle[a].p[j].x=clip(particle[a].p[j].x+vx[a][j]);particle[a].p[j].y=clip(particle[a].p[j].y+vy[a][j]);
      particle[a].h[j]+=vh[a][j];if(particle[a].h[j]<g_hmin)particle[a].h[j]=g_hmin;if(particle[a].h[j]>g_hmax)particle[a].h[j]=g_hmax;
    }
    evaluate(&particle[a],n,q);if(particle[a].utility>pbest[a].utility)pbest[a]=particle[a];if(pbest[a].utility>gbest.utility)gbest=pbest[a];
  }
  *out=gbest;
}
static int cmp_d(const void*a,const void*b){double x=*(const double*)a,y=*(const double*)b;return(x>y)-(x<y);}
static void metrics(FILE*f,const char*scenario,int n,int seed,const char*method,Sol*s,double sec){
  double r[MAX_N],sum2=0;for(int i=0;i<n;i++){r[i]=s->rate[i];sum2+=r[i]*r[i];}qsort(r,n,sizeof(double),cmp_d);
  double mean=s->throughput/n,p5=r[(int)floor(.05*(n-1))],jain=sum2>0?s->throughput*s->throughput/(n*sum2):0;
  fprintf(f,"%s,%d,%d,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n",scenario,n,seed,method,(double)s->served/n,s->k,s->throughput,mean,p5,1.0-(double)s->served/n,jain,sec);
}
static void sensitivity_row(FILE*f,const char*factor,double level,int seed,Sol*s,int n,double sec){
  double r[MAX_N],sum2=0;for(int i=0;i<n;i++){r[i]=s->rate[i];sum2+=r[i]*r[i];}qsort(r,n,sizeof(double),cmp_d);
  double jain=sum2>0?s->throughput*s->throughput/(n*sum2):0;
  fprintf(f,"%s,%.8g,%d,%.8f,%d,%.8f,%.8f,%.8f,%.8f\n",factor,level,seed,(double)s->served/n,s->k,s->throughput,r[(int)floor(.05*(n-1))],jain,sec);
}
static void run_sensitivity(void){
  const char*factor[5]={"qos_rmin_mbps","capacity_users","fleet_cost_lambda_k","altitude_hmax_m","nlos_loss_db"};
  const double level[5][3]={{0.5,2,5},{10,20,30},{25,100,400},{80,150,250},{10,20,30}};
  FILE*f=fopen("results/reproduced/parameter_sensitivity_runs.csv","w");if(!f)return;
  fprintf(f,"factor,level,seed,coverage,K,throughput_Mbps,p5_rate_Mbps,jain_fairness,time_s\n");
  for(int a=0;a<5;a++)for(int b=0;b<3;b++)for(int run=0;run<RUNS;run++){
    reset_parameters();
    if(a==0)g_rmin=level[a][b];else if(a==1)g_capacity=(int)level[a][b];else if(a==2)g_lambda_k=level[a][b];else if(a==3)g_hmax=level[a][b];else g_eta_nlos=level[a][b];
    /* Common random numbers: every factor and level uses the same 30 instances. */
    int seed=20000+run,n=400;Point q[MAX_N];Sol s;srand(seed);normal_ready=0;users(0,n,q);gno(&s,n,q);
    clock_t t=clock();alns(&s,n,q,1);sensitivity_row(f,factor[a],level[a][b],seed,&s,n,(double)(clock()-t)/CLOCKS_PER_SEC);
  }
  fclose(f);reset_parameters();
}
int main(int argc,char **argv){
  if(argc>1&&strcmp(argv[1],"--sensitivity-only")==0){run_sensitivity();puts("saved results/reproduced/parameter_sensitivity_runs.csv");return 0;}
  const int ns[]={100,200,300,400};const char*sc[]={"four_cluster","uniform","hotspot","corridor"};
  FILE*f=fopen("results/reproduced/spatial_robustness_ablation_pso_runs.csv","w");if(!f)return 1;
  fprintf(f,"scenario,N,seed,method,coverage,K,throughput_Mbps,mean_user_rate_Mbps,p5_rate_Mbps,qos_violation,jain_fairness,time_s\n");
  for(int z=0;z<4;z++)for(int a=0;a<4;a++)for(int run=0;run<RUNS;run++){
    int n=ns[a],seed=10000+z*1000+a*100+run;Point q[MAX_N];srand(seed);normal_ready=0;users(z,n,q);
    Sol base,sol;clock_t t;
    t=clock();kmeans(&sol,n,q);metrics(f,sc[z],n,seed,"kmeans_grid",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);
    t=clock();gno(&base,n,q);metrics(f,sc[z],n,seed,"gno",&base,(double)(clock()-t)/CLOCKS_PER_SEC);
    sol=base;t=clock();alns(&sol,n,q,0);metrics(f,sc[z],n,seed,"alns_no_refine",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);
    sol=base;t=clock();for(int it=0;it<10;it++)refine(&sol,n,q);metrics(f,sc[z],n,seed,"fixed_gno_refine",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);
    sol=base;t=clock();alns(&sol,n,q,1);metrics(f,sc[z],n,seed,"alns_sca_lite",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);
    {int controlled_k=sol.k;t=clock();pso_controlled(&sol,n,q,controlled_k,3,4);metrics(f,sc[z],n,seed,"pso_low_3x4",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);
      t=clock();pso_controlled(&sol,n,q,controlled_k,6,8);metrics(f,sc[z],n,seed,"pso_mid_6x8",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);
      t=clock();pso_controlled(&sol,n,q,controlled_k,12,16);metrics(f,sc[z],n,seed,"pso_high_12x16",&sol,(double)(clock()-t)/CLOCKS_PER_SEC);}
  }
  fclose(f);run_sensitivity();puts("saved results/reproduced/spatial_robustness_ablation_pso_runs.csv and parameter_sensitivity_runs.csv");return 0;
}
