/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP.h
 *  Description: KPEngine Cpu reckognition implementation
 *				 - CPU vendor mapping
 *				 - SIMD support check
 *
 *****************************************************************
*/

#include <windows.h>	// for memset()
#include "KPCPU.h"

// Check whether the CPU supports the CPUID instruction
bool CPUID_Chk(void)
{
	__try {
		_asm {
			xor eax,	eax		// Set EAX to 0
			CPUID
		}
	}
	// We suppress the warning message of the constant Exception_Execute_Handler
	#pragma warning (suppress: 6320)
	__except (EXCEPTION_EXECUTE_HANDLER) {
		// If we get an exception here, that means we could not execute
		// the CPUID instruction
		return false;
	}
	return true;
} // ! CPUID_Chk()

// SIMD_OS_Support_Chk Function
///////////////////////////////
//
// Checks whether the OS Supports the SIMD instruction
// Returns true if the OS supports it, false if doesn't.
bool SIMD_OS_Support_Chk(DWORD dwFeature)
{
	__try
	{

		switch (dwFeature)
		{

		case CPU_FEATURE_SSE:
			__asm {
				xorps xmm0, xmm0	// Try to execute SSE instruction
			}
			break;

		case CPU_FEATURE_SSE2:
			__asm {
				xorpd xmm0, xmm0	// Try to execute SSE2 instruction
			}
			break;

		case CPU_FEATURE_SSE3:
			__asm {
				addsubpd xmm0, xmm1	// Try to execute SSE3 instruction
			}
			break;

		case CPU_FEATURE_SSSE3:
			__asm {
				pabsb xmm1, xmm2	// Try to execute SSSE3 instruction
			}
			break;

		case CPU_FEATURE_SSE41:
			__asm {
				ptest xmm1, xmm2	// Try to execute SSE4.1 instruction
			}
			break;

		case CPU_FEATURE_SSE42:
			__asm {
				pcmpgtq xmm1, xmm2	// Try to execute SSE4.2 instruction
			}

		case CPU_FEATURE_3DNOW:
			__asm {
				pfrcp mm0, mm0		// Try to execute 3DNow! instruction
				emms				// Empties the multimedia state, needed to reset MMX register states
			}
			break;

		case CPU_FEATURE_3DNOWEX:
			__asm {
				pswapd mm0,mm1		// Try to execute 3DNow!+ instruction
				emms				// Empties the multimedia state, needed to reset MMX register states
			}
			break;

		case CPU_FEATURE_MMX:
			__asm {
				pxor mm0, mm0		// Try to execute MMX instruction
				emms				// Empties the multimedia state, needed to reset MMX register states
			}
			break;

		case CPU_FEATURE_MMXEX:
			__asm {
				pshufw mm0, mm1, 00011011b	// Try to execute MMX+ instruction
				emms						// Empties the multimedia state, needed to reset MMX register states
			}
			break;

		} // ! switch
	} // ! try

	#pragma warning (suppress: 6320)
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if ( GetExceptionCode() == STATUS_ILLEGAL_INSTRUCTION )
		{
			// If any of the SIMD instructions were illegal
			// the OS does not support the feature
			return false;
		}

		// If it fails for any other reason
		return false;
	} // ! except

	return true;

}	// ! SIMD_OS_Support_Chk


// MapCPUName Function
////////////////////////
//
// Maps the CPU family and model to a CPU name
// TODO: This is far from being precise
//		 Add rev/stepping cases to pinpoint exacpt CPU
//		 Add CPU speed, etc..
void MapCpuName( int family, int model, const char* vendorName, char* modelName )
{
	modelName[0] = '\0';

	if ( strncmp("AuthenticAMD", vendorName, 12) == 0 )
	{
		switch(family)
		{
		case 4:	// Am486/AM5x586
			strcpy_s(modelName, sizeof(CPU_AMD_AM486), CPU_AMD_AM486);
			break;

		case 5: // K5 & K6
			switch(model)
			{
			case 0:
			case 1:
			case 2:
			case 3:
				strcpy_s(modelName, sizeof(CPU_AMD_K5), CPU_AMD_K5);
				break;
			case 6:
			case 7:
				strcpy_s(modelName, sizeof(CPU_AMD_K6), CPU_AMD_K6);
				break;
			case 8:
				strcpy_s(modelName, sizeof(CPU_AMD_K6_2), CPU_AMD_K6_2);
				break;
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				strcpy_s(modelName, sizeof(CPU_AMD_K6_3), CPU_AMD_K6_3);
				break;
			} // ! switch model
			break;

		case 6: // K7
			switch(model)
			{
			case 0:
			case 1:
			case 2:
			case 4:
			case 6:
			case 8:
			case 10:
				strcpy_s(modelName, sizeof(CPU_AMD_K7_ATHLON), CPU_AMD_K7_ATHLON);
				break;
			case 3:
			case 7:
				strcpy_s(modelName, sizeof(CPU_AMD_K7_DURON), CPU_AMD_K7_DURON);
				break;
			} // ! switch model
			break;

		case 15: // K8
			strcpy_s(modelName, sizeof(CPU_AMD_K8), CPU_AMD_K8);
			break;

		default:
			strcpy_s(modelName, sizeof(CPU_AMD_UNKNOWN), CPU_AMD_UNKNOWN);

		} // ! switch family
	} // ! if AMD

	else if ( strncmp("GenuineIntel", vendorName, 12) == 0 )
	{
		switch (family)
		{
		case 4:
			switch (model)
			{
			case 0:
			case 1:
				strcpy_s(modelName, sizeof(CPU_INTEL_486DX), CPU_INTEL_486DX);
				break;
			case 2:
				strcpy_s(modelName, sizeof(CPU_INTEL_486SX), CPU_INTEL_486SX);
				break;
			case 3:
				strcpy_s(modelName, sizeof(CPU_INTEL_486DX2), CPU_INTEL_486DX2);
				break;
			case 4:
				strcpy_s(modelName, sizeof(CPU_INTEL_486SL), CPU_INTEL_486SL);
				break;
			case 5:
				strcpy_s(modelName, sizeof(CPU_INTEL_486SX2), CPU_INTEL_486DX);
				break;
			case 7:
				strcpy_s(modelName, sizeof(CPU_INTEL_486DX2E), CPU_INTEL_486DX2E);
				break;
			case 8:
				strcpy_s(modelName, sizeof(CPU_INTEL_486DX4), CPU_INTEL_486DX4);
				break;
			} // ! switch model
			break;

		case 5:
			switch (model)
			{
			case 1:
			case 2:
			case 3:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM), CPU_INTEL_PENTIUM);
				break;
			case 4:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_MMX), CPU_INTEL_PENTIUM_MMX);
				break;
			} // ! switch model
			break;

		case 6:
			switch (model)
			{
			case 1:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_PRO), CPU_INTEL_PENTIUM_PRO);
				break;
			case 3:
			case 5:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_II), CPU_INTEL_PENTIUM_II);
				break;
			case 6:
				strcpy_s(modelName, sizeof(CPU_INTEL_CELERON), CPU_INTEL_CELERON);
				break;
			case 8:
			case 11:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_III), CPU_INTEL_PENTIUM_III);
				break;
			case 9:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_M), CPU_INTEL_PENTIUM_M);
				break;
			case 10:
				strcpy_s(modelName, sizeof(CPU_INTEL_CORE7), CPU_INTEL_CORE7);
				break;
			case 12:
				strcpy_s(modelName, sizeof(CPU_INTEL_ATOM), CPU_INTEL_ATOM);
				break;
			case 13:
				strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_XEON), CPU_INTEL_PENTIUM_XEON);
				break;
			case 14:
				strcpy_s(modelName, sizeof(CPU_INTEL_CORE), CPU_INTEL_CORE);
				break;
			case 7:
			case 15:
				strcpy_s(modelName, sizeof(CPU_INTEL_CORE2), CPU_INTEL_CORE2);
				break;
			} // ! switch model
			break;

		case 15:
			// They all seem to be variatons of Pentium-IVs
			strcpy_s(modelName, sizeof(CPU_INTEL_PENTIUM_4), CPU_INTEL_PENTIUM_4);
			break;

		default:
			strcpy_s(modelName, sizeof(CPU_INTEL_UNKNOWN), CPU_INTEL_UNKNOWN);

		} // ! switch family
	} // ! elif INTEL

	else if ( strncmp("UMC UMC UMC", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_UMC), CPU_UMC);

	else if ( strncmp("CyrixInstead", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_CYRIX), CPU_CYRIX);

	else if ( strncmp("NexGenDriven", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_NEXGEN), CPU_NEXGEN);

	else if ( strncmp("CentaurHauls", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_CENTAUR), CPU_CENTAUR);

	else if ( strncmp("RiseRiseRise", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_RISE), CPU_RISE);

	else if ( strncmp("SiS SiS SiS", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_SIS), CPU_SIS);

	else if ( strncmp("GenuineTMx86", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_TM), CPU_TM);

	else if ( strncmp("Geode by NSC", vendorName, 12) == 0 )
		strcpy_s(modelName, sizeof(CPU_NSC), CPU_NSC);

	// If no Model Name was found, set it to unknown 
	if ( !modelName[0] )
		strcpy_s(modelName, sizeof(CPU_UNKNOWN), CPU_UNKNOWN);
}

// GetCPUInfo Function
//////////////////////
//
// This function uses inline assembly CPUID instructions
// for processor recognition.
// Asks for a pointer to a CPUINFO structure
// Returns 0 on error, 1 on success
int GetCPUInfo(CPUINFO *info)
{
	//CPUINFO info;
	//char* pchVendor		= info->vendorName;
	char* pchVendor	= info->vendorName;
	DWORD dwSignature	= 0;
	DWORD dwFeaturesEDX	= 0;
	DWORD dwFeaturesECX	= 0;
	DWORD dwExt			= 0;

	// Zero out info
	memset(info, 0, sizeof(CPUINFO));

	// Check if the CPUID instruction is available in the CPU
	if ( !CPUID_Chk() )
		return 0;

	_asm {

		// Get the CPU Vendor string
		xor	eax, eax	// Same as mov eax, 0 just in 1 cycle
		CPUID			// Calls Function(0), 00h

		// EBX,EDX,ECX contains the Vendor String in this order
		// We copy the 4 bytes of each register into info->vendorName.
		mov esi,	 pchVendor	// pointer to info->vendorName
		mov [esi],	 ebx
		mov [esi+4], edx
		mov [esi+8], ecx

		// Get the CPU Signature and Standart Features flags
		mov eax, 1
		CPUID
		mov dwSignature,	eax
		mov dwFeaturesEDX,	edx
		mov dwFeaturesECX,	ecx
	} // ! asm


	pchVendor = NULL;

	// Get AMD Specific Extended CPU Informations
	if( strncmp(info->vendorName, "AuthenticAMD", 12) == 0 )
	{
		__asm {

			mov eax,	80000001h			// Get the Extended Feature Flags from EDX
			CPUID
			mov dwExt,	edx
		}
	} // ! AuthenticAMD Section


	// Check the CPU and OS support for SIMD instructions
	// and fill out bitfields

	// MMX ////
	if (dwFeaturesEDX & EDX_FF_MMX)
	{
		info->Feature |= CPU_FEATURE_MMX;

		if ( SIMD_OS_Support_Chk(EDX_FF_MMX) )
			info->OS_Support |= CPU_FEATURE_MMX;
	}

	// SSE ////
	if (dwFeaturesEDX & EDX_FF_SSE)
	{
		info->Feature |= CPU_FEATURE_SSE;

		if ( SIMD_OS_Support_Chk(EDX_FF_SSE) )
			info->OS_Support |= CPU_FEATURE_SSE;
	}

	// SSE2 ////
	if (dwFeaturesEDX & EDX_FF_SSE2)
	{
		info->Feature |= CPU_FEATURE_SSE2;

		if ( SIMD_OS_Support_Chk(EDX_FF_SSE2) )
			info->OS_Support |= CPU_FEATURE_SSE2;
	}

	// SSE3 ////
	if (dwFeaturesECX & ECX_FF_SSE3)
	{
		info->Feature |= CPU_FEATURE_SSE3;

		if ( SIMD_OS_Support_Chk(ECX_FF_SSE3) )
			info->OS_Support |= CPU_FEATURE_SSE3;
	}

	// SSSE3 ////
	if (dwFeaturesECX & ECX_FF_SSSE3)
	{
		info->Feature |= CPU_FEATURE_SSSE3;

		if ( SIMD_OS_Support_Chk(ECX_FF_SSSE3) )
			info->OS_Support |= CPU_FEATURE_SSSE3;
	}

	// SSE4.1 ////
	if (dwFeaturesECX & ECX_FF_SSE41)
	{
		info->Feature |= CPU_FEATURE_SSE41;

		if ( SIMD_OS_Support_Chk(ECX_FF_SSE41) )
			info->OS_Support |= CPU_FEATURE_SSE41;
	}

	// SSE4.2 ////
	if (dwFeaturesECX & ECX_FF_SSE42)
	{
		info->Feature |= CPU_FEATURE_SSE42;

		if ( SIMD_OS_Support_Chk(ECX_FF_SSE42) )
			info->OS_Support |= CPU_FEATURE_SSE42;
	}

	// 3DNow! ////
	if (dwExt & EDX_FF_3DNOW)
	{
		info->Feature |= CPU_FEATURE_3DNOW;

		if ( SIMD_OS_Support_Chk(EDX_FF_3DNOW) )
			info->OS_Support |= CPU_FEATURE_3DNOW;
	}

	// 3DNow! Extensions ////
	if (dwExt & EDX_FF_3DNOWEX)
	{
		info->Feature |= CPU_FEATURE_3DNOWEX;

		if ( SIMD_OS_Support_Chk(EDX_FF_3DNOWEX) )
			info->OS_Support |= CPU_FEATURE_3DNOWEX;
	}

	// MMX Extensions ////
	if (dwExt & EDX_FF_MMXEX)
	{
		info->Feature |= CPU_FEATURE_MMXEX;

		if ( SIMD_OS_Support_Chk(EDX_FF_MMXEX) )
			info->OS_Support |= CPU_FEATURE_MMXEX;
	}


	// Retrieve CPU Family, Model & Stepping
	// Map them to CPU Model Names
	if (info)
	{
	info->Stepping	= dwSignature & 0xF;			// Zero everything except the 4 Stepping bits

	info->Model		= (dwSignature >> 4) & 0xF;		// Get rid of the stepping bits and zero everything but the model number
	if (info->Model == 15)
		info->Model |= (dwSignature >> 12) & 0xF;	// Extended Model ( >> by 12, this way we have 4 extra 0000 bits at the beginning)

	info->Family		= (dwSignature >> 8) & 0xF; // Get rid of stepping + model number and zero evrything except family code
	if (info->Family == 15)
		info->Family |= (dwSignature >> 16) & 0xF;	// Extended Family ( >> by 16, to have 4 extra 0000 bits at the beginning )

	info->vendorName[MAX_VNAME_LEN] = '\0';			// Add the string ending '\0'

	MapCpuName(info->Family, info->Model, info->vendorName, info->modelName);

	info->Checks =	CPU_FEATURE_MMX		|
					CPU_FEATURE_SSE		|
					CPU_FEATURE_SSE2	|	
					CPU_FEATURE_SSE3	|	
					CPU_FEATURE_SSSE3	|	
					CPU_FEATURE_SSE41	|	
					CPU_FEATURE_SSE42	|	
					CPU_FEATURE_3DNOW	|	
					CPU_FEATURE_3DNOWEX	|
					CPU_FEATURE_MMXEX;	

	} // ! if

	return 1;
} // ! GetCpuInfo function

