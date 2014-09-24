// ThreadSelectorTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,          // Note: this is kernel mode only
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    MaxProcessInfoClass
    } PROCESSINFOCLASS;

typedef struct {
	ULONG Start;
	ULONG Length;
	LDT_ENTRY LdtEntries[1];
} PROCESS_LDT_INFORMATION;


BYTE test_buffer1[256];
BYTE test_buffer2[256];


void print_GDT_entries()
{
	LDT_ENTRY ldt;
	int index;
	DWORD dwSelector;

	printf("GDT Table:\n");
	for(index=0; index<256; ++index) {
		dwSelector = index*sizeof(LDT_ENTRY);
		if(::GetThreadSelectorEntry(GetCurrentThread(), dwSelector, &ldt)==0) {
			break;
		}
		printf("%d.%lx: ", index, dwSelector);
		printf("%lx %lx %lx: ", ldt.HighWord.Bytes.BaseHi, ldt.HighWord.Bytes.BaseMid, ldt.BaseLow );
		printf("%lx %lx: ", ldt.HighWord.Bits.LimitHi, ldt.LimitLow);
		printf("DPL:%x Type:%x 4k:%x\n", ldt.HighWord.Bits.Dpl, ldt.HighWord.Bits.Type, ldt.HighWord.Bits.Granularity);
	}
}


void print_LDT_entries()
{
	int index;
	DWORD dwSelector;
	PROCESS_LDT_INFORMATION ldtInfo;
	DWORD rc;
	LDT_ENTRY *ldt = &ldtInfo.LdtEntries[0];

	HMODULE hlib = GetModuleHandle(L"NTDLL");
	FARPROC fp = GetProcAddress(hlib, "NtQueryInformationProcess");
	NTSTATUS (CALLBACK *NtQueryInformationProcess) (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    *(FARPROC *)&NtQueryInformationProcess = fp;


	printf("LDT Table:\n");
	ldtInfo.Start = 0;
	ldtInfo.Length = 1*sizeof(LDT_ENTRY);
	rc = NtQueryInformationProcess(GetCurrentProcess(), ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo), NULL);
	int numEntries = ldtInfo.Length / sizeof(LDT_ENTRY);
	printf("%d entries\n", numEntries);
	for(index=0; index<numEntries; ++index) {
		dwSelector = index*sizeof(LDT_ENTRY);

		ldtInfo.Start = dwSelector ;//& 0xFFFFFFF8;  // selector --> offset
		ldtInfo.Length = 1*sizeof(LDT_ENTRY);
		rc = NtQueryInformationProcess(GetCurrentProcess(), ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo), NULL);
		
		if(rc!=0) {
			printf("LDT read failed %lx", rc);
			break;
		}
		
		printf(" %d: ", index);
		printf("%lx %lx %lx: ", ldt->HighWord.Bytes.BaseHi, ldt->HighWord.Bytes.BaseMid, ldt->BaseLow );
		printf("%lx %lx: ", ldt->HighWord.Bits.LimitHi, ldt->LimitLow);
		printf("DPL:%x Type:%x 4k:%x\n", ldt->HighWord.Bits.Dpl, ldt->HighWord.Bits.Type, ldt->HighWord.Bits.Granularity);
	}
}


void allocateLDT()
{
	PROCESS_LDT_INFORMATION ldtInfo;
	DWORD rc;
	LDT_ENTRY *ldt = &ldtInfo.LdtEntries[0];

	DWORD addr;
	DWORD limit = 256;
	DWORD dwSelector;

	dwSelector = 1 * sizeof(LDT_ENTRY); // [1]
	addr = (DWORD)(&test_buffer1);

	ldt->BaseLow = addr & 0xFFFF;
	ldt->HighWord.Bytes.BaseMid = (BYTE)(addr >> 16);
	ldt->HighWord.Bytes.BaseHi  = (BYTE)(addr >> 24);

	ldt->HighWord.Bits.Granularity = 0; //0=bytes, 1=pages (4k)
	ldt->LimitLow = limit & 0xFFFF;
	ldt->HighWord.Bits.LimitHi = limit >> 16;

	ldt->HighWord.Bits.Default_Big = 1; //0=16bit, 1=32bit

	//Always make a normal RW data segment (for now)
	ldt->HighWord.Bits.Type = 0x1b;

	ldt->HighWord.Bits.Pres = 1;  //Presense bit
	ldt->HighWord.Bits.Dpl  = 3;  //ALWAYS ring 3
	ldt->HighWord.Bits.Sys  = 1;  // 0=sys, 1=user


	HMODULE hlib = GetModuleHandle(L"NTDLL");
	FARPROC fp = GetProcAddress(hlib, "NtSetInformationProcess");
	NTSTATUS (CALLBACK *NtSetInformationProcess) (HANDLE, PROCESSINFOCLASS, PVOID, ULONG);
    *(FARPROC *)&NtSetInformationProcess = fp;


	//Write LDT entry
	//
	ldtInfo.Start = dwSelector;
	ldtInfo.Length = 1*sizeof(LDT_ENTRY); // size
	rc = NtSetInformationProcess(GetCurrentProcess(), ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo));

	if(rc==0) { //NTSTATUS OK
		//ok

		printf("LDT [16]\n");

		addr = (DWORD)(&test_buffer2);
		dwSelector = 16 * sizeof(LDT_ENTRY); // [1]

		ldt->BaseLow = addr & 0xFFFF;
		ldt->HighWord.Bytes.BaseMid = (BYTE)(addr >> 16);
		ldt->HighWord.Bytes.BaseHi  = (BYTE)(addr >> 24);

		ldtInfo.Start = dwSelector;
		rc = NtSetInformationProcess(GetCurrentProcess(), ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo));
		if(rc!=0) {
			printf("!fail %lx\n", rc);
		}

		//Copy of an LDT from trcing busybox through keow
		//Fails - test why
		printf("Keow LDT\n");
		ldt->HighWord.Bits.BaseHi		= 0x08;
		ldt->HighWord.Bits.BaseMid	    = 0x18;
		ldt->BaseLow					= 0x4800;
		ldt->HighWord.Bits.LimitHi	    = 0x0f;
		ldt->LimitLow					= 0xffff;
		ldt->HighWord.Bits.Type		    = 0x13;
		ldt->HighWord.Bits.Dpl		    = 0x03;
		ldt->HighWord.Bits.Pres		    = 0x01;
		ldt->HighWord.Bits.Sys		    = 0x01;
		ldt->HighWord.Bits.Reserved_0	= 0x00;
		ldt->HighWord.Bits.Default_Big	= 0x01;
		ldt->HighWord.Bits.Granularity	= 0x01;
		//Seems to fail because base+limit(4k) >= 0x7FFFFFFF = user address limit

		dwSelector = 12 * sizeof(LDT_ENTRY); 
		ldtInfo.Start = dwSelector;
		rc = NtSetInformationProcess(GetCurrentProcess(), ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo));
		if(rc!=0) {
			printf("!fail %lx\n", rc);
			exit(0);
		}

		return;
	}
	else 
	{
		printf("Selector set error %lx\n", rc);
		return;
	}

}


void printFS()
{
	int i=0;
	__asm {
		mov eax,0
		mov ax,fs
		mov i, eax
	}
	printf("FS:%lx Segment=%d\n", i, (i>>3));
}


void testSelectorAccess()
{
	USHORT selector;
	DWORD tmp;

	printf("access test\n");

	//Use fs: to access the buffer using the LDT entries we set up
	int entry;

	// See 386INTEL.txt, Section:  5.1.3  Selectors
	entry = 1;
	selector = (entry<<3) | 0x4 | 0x3; // entry | LDT=1 | DPL=3
	_asm {
		push fs  ;save

		//access fs:xx
		mov fs, selector
		mov eax, fs:13
		mov tmp, eax

		pop fs ;restore
	}
	printf("buff1[13] = %x\n", test_buffer1[13]);
	printf("fs:buff1[13] = %x\n", tmp);

	// See 386INTEL.txt, Section:  5.1.3  Selectors
	entry = 16;
	selector = (entry<<3) | 0x4 | 0x3; // entry | LDT=1 | DPL=3
	_asm {
		push fs  ;save

		//access fs:xx
		mov fs, selector
		mov eax, fs:13
		mov tmp, eax

		pop fs ;restore
	}
	printf("buff2[13] = %x\n", test_buffer2[13]);
	printf("fs:buff2[13] = %x\n", tmp);
}


DWORD WINAPI ThreadEntry(LPVOID lpParameter)
{
	printf("New thread\n");
	printFS();
	print_GDT_entries();
	print_LDT_entries();
	Sleep(5000);
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	for(int i=0; i<256; ++i) {
		test_buffer1[i] = i & 0xFF;
		test_buffer2[i] = 0x56;
	}

	printFS();
	print_GDT_entries();


	printf(":LDT alloc\n");
	printFS();
	allocateLDT();
	printFS();
	print_GDT_entries();
	print_LDT_entries();

	HANDLE hThread = CreateThread(NULL, 0, ThreadEntry, 0, 0, NULL);
	Sleep(2000);
	printf("Orig thread\n");
	print_GDT_entries();
	print_LDT_entries();

	testSelectorAccess();

	printf("Press return");
	getchar();
	return 0;
}

