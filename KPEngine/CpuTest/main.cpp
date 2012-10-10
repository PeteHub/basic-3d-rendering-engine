#include <stdio.h>
#include <vector>
#include <algorithm>

#include "kp_cpuid.h"

void expand(unsigned long avail, unsigned long mask)
{
    if (mask & CPU_FEATURE_MMX) {
        printf("\t%s\t_CPU_FEATURE_MMX\n",   avail & CPU_FEATURE_MMX ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_SSE) {
        printf("\t%s\t_CPU_FEATURE_SSE\n",   avail & CPU_FEATURE_SSE ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_SSE2) {
        printf("\t%s\t_CPU_FEATURE_SSE2\n",  avail & CPU_FEATURE_SSE2 ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_SSE3) {
        printf("\t%s\t_CPU_FEATURE_SSE3\n",  avail & CPU_FEATURE_SSE3 ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_SSSE3) {
        printf("\t%s\t_CPU_FEATURE_SSSE3\n", avail & CPU_FEATURE_SSSE3 ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_SSE41) {
        printf("\t%s\t_CPU_FEATURE_SSE41\n", avail & CPU_FEATURE_SSE41 ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_SSE42) {
        printf("\t%s\t_CPU_FEATURE_SSE42\n", avail & CPU_FEATURE_SSE42 ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_3DNOW) {
        printf("\t%s\t_CPU_FEATURE_3DNOW\n", avail & CPU_FEATURE_3DNOW ? "yes" : "no");
    }
	if (mask & CPU_FEATURE_3DNOWEX) {
        printf("\t%s\t_CPU_FEATURE_3DNOWEX\n", avail & CPU_FEATURE_3DNOWEX ? "yes" : "no");
    }
    if (mask & CPU_FEATURE_MMXEX) {
        printf("\t%s\t_CPU_FEATURE_MMXEX\n", avail & CPU_FEATURE_MMXEX ? "yes" : "no");
    }
}

template <class T> const T amax( const T a, const T b )
{
	return (b<a)?a:b;
}

template <class T> const T amax( const T a, const T b, const T c )
{
	std::vector<T> sortMe;
	sortMe.resize(3);
	
	sortMe[0] = a;
	sortMe[1] = b;
	sortMe[2] = c;

	std::sort(sortMe.begin(), sortMe.end() );

	return sortMe[2];
}

int main(void)
{
	/*
	CPUINFO info;
	
	GetCPUInfo(&info);

	printf("v_name:\t\t%s\n", info.vendorName);
	printf("model:\t\t%s\n", info.modelName);
    printf("family:\t\t%d\n", info.Family);
    printf("model:\t\t%d\n", info.Model);
    printf("stepping:\t%d\n", info.Stepping);
	printf("feature:\t%08x\n", info.Feature);
    expand(info.Feature, info.Checks);
	printf("os_support:\t%08x\n", info.OS_Support);
    expand(info.OS_Support, info.Checks);
    printf("checks:\t\t%08x\n", info.Checks);
	*/

	printf("test %d",amax(5,2,7) );
	printf("\n\nPress ENTER to exit. ");

	getchar();
	return 1;

}