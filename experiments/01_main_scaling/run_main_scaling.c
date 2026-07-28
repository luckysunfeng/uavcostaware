#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FC 3.5e9
#define LIGHT_SPEED 3e8
#define PT_DBM 33.0
#define NOISE_DBM -100.0
#define ETA_LOS 1.0
#define ETA_NLOS 20.0
#define A_PARAM 9.61
#define B_PARAM 0.16
#define C_PARAM 2.0
#define W_HZ 20e6
#define R_MAX 500.0
#define R_MIN 0.5
#define N_PRB 20
#define H_MIN 30.0
#define H_MAX 150.0
#define TAU 10
#define MAX_USERS 500
#define MAX_UAVS 128
#define RUNS 30
#define ALNS_ITERS 20

static double PT_MW, NOISE_MW;

typedef struct {
    int K;
    double xy[MAX_UAVS][2];
    double h[MAX_UAVS];
    int assignment[MAX_USERS];
    double rate[MAX_USERS];
    int load[MAX_UAVS];
    int served;
    double throughput;
    double utility;
} Solution;

typedef struct { double sum, sum_sq; } Stats;

static double uniform01(void) { return rand() / (RAND_MAX + 1.0); }
static double normal01(void) {
    static int spare_ready = 0; static double spare;
    if (spare_ready) { spare_ready = 0; return spare; }
    double u = uniform01(); if (u < 1e-12) u = 1e-12;
    double v = uniform01();
    double r = sqrt(-2.0 * log(u));
    spare = r * sin(2.0 * M_PI * v); spare_ready = 1;
    return r * cos(2.0 * M_PI * v);
}
static double dist(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2, dy = y1 - y2; return sqrt(dx * dx + dy * dy);
}
static double pl_db(double d, double h) {
    double d3 = sqrt(d*d + h*h);
    double theta = atan2(h, d + 1e-9) * 180.0 / M_PI;
    double p_los = 1.0 / (1.0 + A_PARAM * exp(-B_PARAM * (theta - C_PARAM)));
    double common = 20.0 * log10(d3) + 20.0 * log10(FC) + 20.0 * log10(4.0 * M_PI / LIGHT_SPEED);
    return common + p_los * ETA_LOS + (1.0 - p_los) * ETA_NLOS;
}
static double rate_mbps(double d, double h, double interference) {
    if (d > R_MAX) return 0.0;
    double signal = PT_MW * pow(10.0, -pl_db(d, h) / 10.0);
    return W_HZ * log2(1.0 + signal / (NOISE_MW + interference)) / 1e6;
}
static void generate_users(int n, double u[][2]) {
    const double c[4][2] = {{250,250},{750,250},{250,750},{750,750}};
    for (int i=0; i<n; ++i) {
        int q = rand() % 4;
        u[i][0] = c[q][0] + 120.0 * normal01();
        u[i][1] = c[q][1] + 120.0 * normal01();
        if (u[i][0] < 0) u[i][0] = 0;
        if (u[i][0] > 1000) u[i][0] = 1000;
        if (u[i][1] < 0) u[i][1] = 0;
        if (u[i][1] > 1000) u[i][1] = 1000;
    }
}
static double optimize_height(double x, double y, int n, double u[][2]) {
    double best_h = H_MIN, best = -1.0;
    for (int s=0; s<=8; ++s) {
        double h = H_MIN + (H_MAX-H_MIN)*s/8.0, score = 0;
        for (int i=0; i<n; ++i) score += rate_mbps(dist(x,y,u[i][0],u[i][1]),h,0.0);
        if (score > best) { best = score; best_h = h; }
    }
    return best_h;
}
static void evaluate(Solution *s, int n, double u[][2]) {
    for (int j=0; j<s->K; ++j) s->load[j] = 0;
    s->served = 0; s->throughput = 0;
    for (int i=0; i<n; ++i) {
        double signal[MAX_UAVS], total_signal = 0.0;
        for (int j=0; j<s->K; ++j) {
            double d = dist(u[i][0],u[i][1],s->xy[j][0],s->xy[j][1]);
            signal[j] = d <= R_MAX ? PT_MW * pow(10.0, -pl_db(d,s->h[j]) / 10.0) : 0.0;
            total_signal += signal[j];
        }
        int best_j = -1; double best_rate = 0;
        for (int j=0; j<s->K; ++j) {
            if (s->load[j] >= N_PRB) continue;
            double d = dist(u[i][0],u[i][1],s->xy[j][0],s->xy[j][1]);
            if (d > R_MAX) continue;
            double interference = total_signal - signal[j];
            double r=rate_mbps(d,s->h[j],interference);
            if (r >= R_MIN && r > best_rate) { best_rate=r; best_j=j; }
        }
        s->assignment[i]=best_j; s->rate[i]=best_rate;
        if (best_j>=0) { s->load[best_j]++; s->served++; s->throughput+=best_rate; }
    }
    /* Service has lexical priority; fleet penalty selects compact feasible layouts. */
    s->utility = 100000.0*s->served + s->throughput - 100.0*s->K;
}
static void gno_initialize(Solution *s, int n, double u[][2]) {
    int target=(n+TAU-1)/TAU; if(target>MAX_UAVS) target=MAX_UAVS;
    s->K=target;
    /* Contiguous, nearly equal blocks in the stored user order. */
    for(int j=0;j<target;++j) {
        int begin=j*n/target, end=(j+1)*n/target; double x=0,y=0;
        for(int i=begin;i<end;++i){x+=u[i][0];y+=u[i][1];}
        int count=end-begin; s->xy[j][0]=x/count; s->xy[j][1]=y/count;
        s->h[j]=optimize_height(s->xy[j][0],s->xy[j][1],count,&u[begin]);
    }
    evaluate(s,n,u);
}
static void relocate_sca(Solution *s, int n, double u[][2]) {
    for (int j=0;j<s->K;++j) {
        double x=0,y=0; int c=0;
        for(int i=0;i<n;++i) if(s->assignment[i]==j){x+=u[i][0];y+=u[i][1];c++;}
        if(c>0) {
            double assigned[MAX_USERS][2];
            int a = 0;
            for(int i=0;i<n;++i) if(s->assignment[i]==j) { assigned[a][0]=u[i][0]; assigned[a][1]=u[i][1]; ++a; }
            double nx=x/c, ny=y/c;
            /* Trust region: never move more than 80 m in one surrogate update. */
            double dx=nx-s->xy[j][0], dy=ny-s->xy[j][1], d=sqrt(dx*dx+dy*dy);
            if(d>80.0){nx=s->xy[j][0]+80.0*dx/d; ny=s->xy[j][1]+80.0*dy/d;}
            s->xy[j][0]=nx; s->xy[j][1]=ny;
            s->h[j]=optimize_height(nx,ny,a,assigned);
        }
    }
}
static void remove_uav(Solution *s, int victim) {
    for(int j=victim;j<s->K-1;++j){s->xy[j][0]=s->xy[j+1][0];s->xy[j][1]=s->xy[j+1][1];s->h[j]=s->h[j+1];}
    s->K--;
}
static void alns_sca(Solution *best, int n, double u[][2]) {
    Solution current=*best;
    for(int it=0;it<ALNS_ITERS;++it) {
        Solution candidate=current;
        /* ALNS destruction: alternate least-load removal and random neighborhood removal. */
        if(candidate.K>1) {
            int victim=0;
            if(it%2==0){for(int j=1;j<candidate.K;++j)if(candidate.load[j]<candidate.load[victim])victim=j;}
            else victim=rand()%candidate.K;
            remove_uav(&candidate,victim);
        }
        evaluate(&candidate,n,u);
        if (it % 2 == 0) relocate_sca(&candidate,n,u); /* bounded continuous refinement */
        evaluate(&candidate,n,u);
        /* Feasibility preserving acceptance; never trade away served users. */
        if(candidate.served>=current.served && candidate.utility>=current.utility) current=candidate;
    }
    *best=current;
}
static void add(Stats *s,double v){s->sum+=v;s->sum_sq+=v*v;}
static double mean(const Stats*s){return s->sum/RUNS;}
static double sd(const Stats*s){double m=mean(s);double v=(s->sum_sq-RUNS*m*m)/(RUNS-1);return v>0?sqrt(v):0;}

int main(void) {
    PT_MW=pow(10.0,PT_DBM/10.0); NOISE_MW=pow(10.0,NOISE_DBM/10.0);
    const int ns[]={100,200,300,400};
    FILE *fp=fopen("results/reproduced/main_scaling_30_seeds.csv","w"); if(!fp) return 1;
    fprintf(fp,"N,CR_GNO_mean,CR_GNO_sd,K_GNO_mean,K_GNO_sd,CR_ALNS_mean,CR_ALNS_sd,K_ALNS_mean,K_ALNS_sd,Utility_ALNS_mean,Utility_ALNS_sd,Time_ALNS_mean,Time_ALNS_sd\n");
    for(int q=0;q<4;++q){
        Stats cg={0},kg={0},ca={0},ka={0},ua={0},ta={0}; int n=ns[q];
        for(int run=0;run<RUNS;++run){
            double u[MAX_USERS][2]; Solution gno, alns;
            srand(1000+q*100+run); generate_users(n,u); gno_initialize(&gno,n,u); alns=gno;
            clock_t t=clock(); alns_sca(&alns,n,u); double elapsed=(double)(clock()-t)/CLOCKS_PER_SEC;
            add(&cg,(double)gno.served/n); add(&kg,gno.K); add(&ca,(double)alns.served/n); add(&ka,alns.K); add(&ua,alns.utility); add(&ta,elapsed);
        }
        fprintf(fp,"%d,%.4f,%.4f,%.2f,%.2f,%.4f,%.4f,%.2f,%.2f,%.2f,%.2f,%.4f,%.4f\n",n,mean(&cg),sd(&cg),mean(&kg),sd(&kg),mean(&ca),sd(&ca),mean(&ka),sd(&ka),mean(&ua),sd(&ua),mean(&ta),sd(&ta));
    }
    fclose(fp); puts("Results saved to results/reproduced/main_scaling_30_seeds.csv"); return 0;
}
