/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP.h
 *  Description: KPEngine CPU reckognition and SIMD support
 *
 *****************************************************************
*/

#ifndef KP_CPU_H
#define KP_CPU_H

// SIMD Instruction Flags ////
#define CPU_FEATURE_MMX		0x0001	// MMX 
#define CPU_FEATURE_SSE		0x0002	// Streaming SIMD Extension
#define CPU_FEATURE_SSE2	0x0004  // Streaming SIMD Extension 2
#define CPU_FEATURE_SSE3	0x0008  // Streaming SIMD Extension 3
#define CPU_FEATURE_SSSE3	0x0010  // Supplemental Streaming SIMD Extension 3
#define CPU_FEATURE_SSE41	0x0020  // Streaming SIMD Extension 4.1
#define CPU_FEATURE_SSE42	0x0040  // Streaming SIMD Extension 4.2
#define CPU_FEATURE_3DNOW	0x0080  // 3DNow!
#define CPU_FEATURE_3DNOWEX 0x0100	// AMD extensions to 3DNow!
#define CPU_FEATURE_MMXEX	0x0200  // AMD extensions to MMX

// CPU Vendor & Model Name String Length ////
#define MAX_VNAME_LEN		13		// 3x4=12 bytes +1 for '/0'
#define MAX_MNAME_LEN		64

typedef unsigned long DWORD;
// PROCESSOR_INFORMATION STRUCTURE ////
typedef struct PROCESSOR_INFORMATION
{
	char vendorName[MAX_VNAME_LEN];		// Vendor name, 'GenuineIntel','AuthenticAMD', etc..
	char modelName[MAX_MNAME_LEN];		// Model name, e.g. 'Intel Pentium-Pro'

	int Family;							// Processor family, 6 = Pentium-Pro architecture
	int Model;							// Processor model, 1 = Pentium-Pro for family = 6
	int Stepping;						// Processor revision number
	
	DWORD Feature;						// DWORD bitfield of processor features
	DWORD OS_Support;					// DWORD bitfield of supported OS features
	DWORD Checks;						// DWORD bitfield mask of all SIMD feature flags that were tested.
										// Good for checking whether the CPU was tested against a feature

} CPUINFO;

// GetCPUInfo Function ////
int  GetCPUInfo(CPUINFO *info);

// CPUID SIMD Feature Flags Values in the EDX Register ////
#define EDX_FF_MMX		0x00800000	// 23rd bit
#define EDX_FF_SSE		0x02000000	// 25th bit
#define EDX_FF_SSE2		0x04000000	// 26th bit

// CPUID SIMD Feature Flags Values in the ECX Register ////
#define ECX_FF_SSE3		0x00000001	// 0th bit
#define ECX_FF_SSSE3	0x00000200  // 9th bit
#define ECX_FF_SSE41	0x00080000	// 19th bit
#define ECX_FF_SSE42	0x00100000	// 20th bit - GenuineIntel Only!

// CPUID SIMD Feature Flags Values in the EDX Register on AMD ////
// These are part of the Extended Feature Identifiers only on AMD
// Have to call CPUID with EAX set to 80000001h
#define EDX_FF_3DNOW	0x80000000	// 31st bit
#define EDX_FF_3DNOWEX	0x40000000	// 30th bit
#define EDX_FF_MMXEX	0x00400000	// 22th bit

// CPU Model Names ////
#define CPU_AMD_AM486			"AMD Am486"
#define CPU_AMD_K5				"AMD K5"
#define CPU_AMD_K6				"AMD K6"
#define CPU_AMD_K6_2			"AMD K6-2"
#define CPU_AMD_K6_3			"AMD K6-3"
#define CPU_AMD_K7_ATHLON		"AMD K7 Athlon"
#define CPU_AMD_K7_DURON		"AMD K7 Duron"
#define CPU_AMD_K8				"AMD K8 Athlon/Sempron/Opteron 64/FX/X2/M"
#define CPU_AMD_UNKNOWN			"Unknown AMD"

#define CPU_INTEL_486DX			"INTEL 486DX"
#define CPU_INTEL_486SX			"INTEL 486SX"
#define CPU_INTEL_486DX2		"INTEL 486DX2"
#define CPU_INTEL_486SL			"INTEL 486SL"
#define CPU_INTEL_486SX2		"INTEL 486SX2"
#define CPU_INTEL_486DX2E		"INTEL 486DX2E"
#define CPU_INTEL_486DX4		"INTEL 486DX4"
#define CPU_INTEL_PENTIUM		"INTEL Pentium"
#define CPU_INTEL_PENTIUM_MMX	"INTEL Pentium-MMX"
#define CPU_INTEL_PENTIUM_PRO	"INTEL Pentium-Pro"
#define CPU_INTEL_PENTIUM_II	"INTEL Pentium-II"
#define CPU_INTEL_CELERON		"INTEL Celeron"
#define CPU_INTEL_PENTIUM_M		"INTEL Pentium-M"
#define CPU_INTEL_PENTIUM_XEON	"INTEL Pentium-III Xeon"
#define CPU_INTEL_CORE			"INTEL Core"
#define CPU_INTEL_CORE2			"INTEL Core 2"
#define CPU_INTEL_ATOM			"INTEL Atom"
#define CPU_INTEL_CORE7			"INTEL Core i7"
#define CPU_INTEL_PENTIUM_III	"INTEL Pentium-III"
#define CPU_INTEL_PENTIUM_4		"INTEL Pentium-4"
#define CPU_INTEL_UNKNOWN		"Unknown INTEL"

#define CPU_CYRIX				"Cyrix"
#define CPU_CENTAUR				"Centaur"
#define CPU_UMC					"UMC"
#define CPU_NEXGEN				"NexGen"
#define CPU_NSC					"National Semiconductor"
#define CPU_RISE				"Rise Technology"
#define CPU_SIS					"SiS"
#define CPU_TM					"Transmeta"

#define CPU_UNKNOWN				"Unknown"


#endif // ! KP_CPU_H