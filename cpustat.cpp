#include <iostream>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace std;

struct cpuInfo
{
    uint64_t usr       = 0;
    uint64_t nice      = 0;
    uint64_t sys       = 0;
    uint64_t idle      = 0;
    uint64_t iowait    = 0;
    uint64_t irq       = 0;
    uint64_t sirq      = 0;
    uint64_t steal     = 0;
    uint64_t guest     = 0;
    uint64_t guestnice = 0;
};

using vcpu = std::vector<cpuInfo>;
void getcpuInfo( vcpu& a_vci );
void calculateCpu( const vcpu& a_vci_1, const vcpu& a_vci_2 );

/**
 * @brief get cpu usage from /proc/stat
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main( int argc, char* argv[] )
{
    struct timespec tm;
    struct timespec rem;
    tm.tv_nsec = 100000000; // 1.0s
    
    vcpu vcpi_1;
    vcpu vcpi_2;
    while( true )
    {
        getcpuInfo( vcpi_1 );
        if( -1 == nanosleep( &tm, &rem ) )
        {
            cout << "rem" << endl;
            tm.tv_nsec -= rem.tv_nsec;
        }
        getcpuInfo( vcpi_2 );
        calculateCpu( vcpi_1, vcpi_2 );
        cout << endl;

        vcpi_1.clear();
        vcpi_2.clear();
        if( -1 == nanosleep( &tm, &rem ) )
        {
            cout << "rem" << endl;
            tm.tv_nsec -= rem.tv_nsec;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief 
 * 
 * @param a_vci 
 */
void getcpuInfo( vcpu& a_vci )
{
    const char* path = "/proc/stat";
    FILE* fin = fopen( path, "r" );

    char szBuffer[100];
    char strCpu[6];
    unsigned long long usr       = 0;
    unsigned long long nice      = 0;
    unsigned long long sys       = 0;
    unsigned long long idle      = 0;
    unsigned long long iowait    = 0;
    unsigned long long irq       = 0;
    unsigned long long sirq      = 0;
    unsigned long long steal     = 0;
    unsigned long long guest     = 0;
    unsigned long long guestnice = 0;

    while( fgets( szBuffer, sizeof(szBuffer), fin)  )
    {
        if( strncmp( szBuffer, "cpu", 3 ) != 0 ) break;

        //     cpu  456406 836 161258 9659356 2256 0 13621 0 14709 0
        sscanf( szBuffer, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
            strCpu,
            &usr,
            &nice,
            &sys,
            &idle,
            &iowait,
            &irq,
            &sirq,
            &steal,
            &guest,
            &guestnice );

        cpuInfo ci;
        ci.usr       = usr;
        ci.nice      = nice;
        ci.sys       = sys;
        ci.idle      = idle;
        ci.iowait    = iowait;
        ci.irq       = irq;
        ci.sirq      = sirq;
        ci.steal     = steal;
        ci.guest     = guest;
        ci.guestnice = guestnice;
        a_vci.push_back( ci );
    }
    fclose( fin );
}


/**
 * @brief calc cpu utilization by taking difference and div by sum of all usage
 * 
 * @param a_vci_1 
 * @param a_vci_2 
 */
void calculateCpu( const vcpu& a_vci_1, const vcpu& a_vci_2 )
{
    int16_t nIndex = 0;
    auto itcpu2 = a_vci_2.begin();
    for( cpuInfo cpu1 : a_vci_1 )
    {
        cpuInfo cpu2 = *itcpu2;
        ++itcpu2;

        double sum1 = cpu1.usr + cpu1.nice + cpu1.sys + cpu1.idle + cpu1.iowait + cpu1.irq + cpu1.sirq + cpu1.steal + cpu1.guest + cpu1.guestnice;
        double sum2 = cpu2.usr + cpu2.nice + cpu2.sys + cpu2.idle + cpu2.iowait + cpu2.irq + cpu2.sirq + cpu2.steal + cpu2.guest + cpu2.guestnice;
        
        double diff_usr       = (double)(cpu2.usr       - cpu1.usr);
        double diff_nice      = (double)(cpu2.nice      - cpu1.nice);
        double diff_sys       = (double)(cpu2.sys       - cpu1.sys);
        double diff_idle      = (double)(cpu2.idle      - cpu1.idle);
        double diff_iow       = (double)(cpu2.iowait    - cpu1.iowait);
        double diff_irq       = (double)(cpu2.irq       - cpu1.irq);
        double diff_sirq      = (double)(cpu2.sirq      - cpu1.sirq);
        double diff_steal     = (double)(cpu2.steal     - cpu1.steal);
        double diff_guest     = (double)(cpu2.guest     - cpu1.guest);
        double diff_guestnice = (double)(cpu2.guestnice - cpu1.guestnice);


        double pct_usr       = diff_usr      / (sum2-sum1)*100.0;
        double pct_nice      = diff_nice     / (sum2-sum1)*100.0;
        double pct_sys       = diff_sys      / (sum2-sum1)*100.0;
        double pct_idle      = diff_idle     / (sum2-sum1)*100.0;
        double pct_iow       = diff_iow      / (sum2-sum1)*100.0;
        double pct_irq       = diff_irq      / (sum2-sum1)*100.0;
        double pct_sirq      = diff_sirq     / (sum2-sum1)*100.0;
        double pct_steal     = diff_steal    / (sum2-sum1)*100.0;
        double pct_guest     = diff_guest    / (sum2-sum1)*100.0;
        double pct_guestnice = diff_guestnice/ (sum2-sum1)*100.0;

        if( nIndex == 0 )
        {
            printf( "cpu ave: %%usr:%5.1lf %%nice:%5.1lf %%sys:%5.1lf %%idle:%5.1lf %%iowait:%5.1lf %%irq:%5.1lf %%sirq:%5.1lf %%steal:%5.1lf %%guest:%5.1lf %%guest nice:%5.1lf\n", 
                pct_usr, pct_nice, pct_sys, pct_idle, pct_iow, pct_irq, pct_sirq, pct_steal, pct_guest, pct_guestnice );

        } else
        {
            printf( "cpu%02d  : %%usr:%5.1lf %%nice:%5.1lf %%sys:%5.1lf %%idle:%5.1lf %%iowait:%5.1lf %%irq:%5.1lf %%sirq:%5.1lf %%steal:%5.1lf %%guest:%5.1lf %%guest nice:%5.1lf\n", 
                nIndex-1, pct_usr, pct_nice, pct_sys, pct_idle, pct_iow, pct_irq, pct_sirq, pct_steal, pct_guest, pct_guestnice );
        }
        ++nIndex;
    }
}

