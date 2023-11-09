/*
    Credits to: https://github.com/Apxaey/Handle-Hijacking-Anti-Cheat-Bypass for the source and public sharing!

	Its a little bit messy as i tried to make it asap. All comments below this are from the original creator!
*/

/*
    This is a stand alone bypass made by Apxaey. Feel free to use this in your cheats but credit me for the bypass as i put alot of time into this.
    If you have some brain cells you will be able to incorporate this into your cheats and remain undetected by user-mode anticheats.
    Obviously standard cheat 'recommendations' still apply:
    1.) Use self-written or not signatured code
    2.) Dont write impossible values
    3.) If your going internal use a manual map injector

    If you follow the guidelines above and use this bypass you will be safe from usermode anticheats like VAC.
    Obviously you can build and adapt upon my code to suit your needs.
    If I was to make a cheat for myself i would put this bypass into something i call an 'external internal' cheat.
    Whereby you make a cheat and inject into a legitimate program like discord and add a check to the this bypass to only hijack a handle from the process you inject into, giving the appearence that nothing is out of the ordinary
    However you can implement this bypass into any form of cheat, its your decision.
    If you need want some more info i recommend you watch my YT video on this bypass.
    Anyways if you want to see more of my stuff feel free to join my discord server discord.gg/********. Here's my YT as well https://www.youtube.com/channel/UCPN6OOLxn1OaBP5jPThIiog.
*/

#include <Windows.h> 
#include <iostream>
#include <TlHelp32.h>
#include <string>

// macros we use.Some can be found in wintrnl.h
#define SeDebugPriv 20
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
#define NtCurrentProcess ( (HANDLE)(LONG_PTR) -1 ) 
#define ProcessHandleType 0x7
#define SystemHandleInformation 16 

/*
STRUCTURES NEEDED FOR NTOPENPROCESS:
*/
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWCH   Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG           Length;
	HANDLE          RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG           Attributes;
	PVOID           SecurityDescriptor;
	PVOID           SecurityQualityOfService;
}  OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

typedef struct _CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

/*
STRUCTURES NEEDED FOR HANDLE INFORMATION:
*/

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE; //i shortened it to SYSTEM_HANDLE for the sake of typing

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

/*
FUNCTION PROTOTYPES:
*/
typedef NTSTATUS(NTAPI* _NtDuplicateObject)(
	HANDLE SourceProcessHandle,
	HANDLE SourceHandle,
	HANDLE TargetProcessHandle,
	PHANDLE TargetHandle,
	ACCESS_MASK DesiredAccess,
	ULONG Attributes,
	ULONG Options
	);

typedef NTSTATUS(NTAPI* _RtlAdjustPrivilege)(
	ULONG Privilege,
	BOOLEAN Enable,
	BOOLEAN CurrentThread,
	PBOOLEAN Enabled
	);

typedef NTSYSAPI NTSTATUS(NTAPI* _NtOpenProcess)(
	PHANDLE            ProcessHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PCLIENT_ID         ClientId
	);

typedef NTSTATUS(NTAPI* _NtQuerySystemInformation)(
	ULONG SystemInformationClass, //your supposed to supply the whole class but microsoft kept the enum mostly empty so I just passed 16 instead for handle info. Thats why you get a warning in your code btw
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);

SYSTEM_HANDLE_INFORMATION* hInfo; //holds the handle information

//the handles we will need to use later on

namespace hj {
	HANDLE procHandle = NULL;
	HANDLE hProcess = NULL;
	HANDLE HijackedHandle = NULL;

	// simple function i made that will just initialize our Object_Attributes structure as NtOpenProcess will fail otherwise
    OBJECT_ATTRIBUTES InitObjectAttributes(PUNICODE_STRING name, ULONG attributes, HANDLE hRoot, PSECURITY_DESCRIPTOR security)
    {
        OBJECT_ATTRIBUTES object;

        object.Length = sizeof(OBJECT_ATTRIBUTES);
        object.ObjectName = name;
        object.Attributes = attributes;
        object.RootDirectory = hRoot;
        object.SecurityDescriptor = security;

        return object;
    }

	bool IsHandleValid(HANDLE handle) // i made this to simply check if a handle is valid rather than repeating the if statments
	{
		if (handle && handle != INVALID_HANDLE_VALUE)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	HANDLE HijackExistingHandle(DWORD dwTargetProcessId)
	{
		HMODULE Ntdll = GetModuleHandleA("ntdll"); // get the base address of ntdll.dll

		//get the address of RtlAdjustPrivilege in ntdll.dll so we can grant our process the highest permission possible
		_RtlAdjustPrivilege RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(Ntdll, "RtlAdjustPrivilege");

		boolean OldPriv; //store the old privileges

		// Give our program SeDeugPrivileges whcih allows us to get a handle to every process, even the highest privileged SYSTEM level processes.
		RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &OldPriv);

		//get the address of NtQuerySystemInformation in ntdll.dll so we can find all the open handles on our system
		_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(Ntdll, "NtQuerySystemInformation");

		//get the address of NtDuplicateObject in ntdll.dll so we can duplicate an existing handle into our cheat, basically performing the hijacking
		_NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(Ntdll, "NtDuplicateObject");

		//get the address of NtOpenProcess in ntdll.dll so wecan create a Duplicate handle
		_NtOpenProcess NtOpenProcess = (_NtOpenProcess)GetProcAddress(Ntdll, "NtOpenProcess");


		//initialize the Object Attributes structure, you can just set each member to NULL rather than create a function like i did
		OBJECT_ATTRIBUTES Obj_Attribute = InitObjectAttributes(NULL, NULL, NULL, NULL);

		//clientID is a PDWORD or DWORD* of the process id to create a handle to
		CLIENT_ID clientID = { 0 };


		//the size variable is the amount of bytes allocated to store all the open handles
		DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);

		//we allocate the memory to store all the handles on the heap rather than the stack becuase of the large amount of data
		hInfo = (SYSTEM_HANDLE_INFORMATION*) new byte[size];

		//zero the memory handle info
		ZeroMemory(hInfo, size);

		//we use this for checking if the Native functions succeed
		NTSTATUS NtRet = NULL;

		do
		{
			// delete the previously allocated memory on the heap because it wasn't large enough to store all the handles
			delete[] hInfo;

			//increase the amount of memory allocated by 50%
			size *= 1.5;
			try
			{
				//set and allocate the larger size on the heap
				hInfo = (PSYSTEM_HANDLE_INFORMATION) new byte[size];
			}
			catch (std::bad_alloc) //catch a bad heap allocation.
			{
				procHandle ? CloseHandle(procHandle) : 0;
			}
			Sleep(1); //sleep for the cpu

			//we continue this loop until all the handles have been stored
		} while ((NtRet = NtQuerySystemInformation(SystemHandleInformation, hInfo, size, NULL)) == STATUS_INFO_LENGTH_MISMATCH);

		//check if we got all the open handles on our system
		if (!NT_SUCCESS(NtRet))
		{
			procHandle ? CloseHandle(procHandle) : 0;
		}


		//loop through each handle on our system, and filter out handles that are invalid or cant be hijacked
		for (unsigned int i = 0; i < hInfo->HandleCount; ++i)
		{
			//a variable to store the number of handles OUR cheat has open.
			static DWORD NumOfOpenHandles;

			//get the amount of outgoing handles OUR cheat has open
			GetProcessHandleCount(GetCurrentProcess(), &NumOfOpenHandles);

			//you can do a higher number if this is triggering false positives. Its just to make sure we dont fuck up and create thousands of handles
			if (NumOfOpenHandles > 50)
			{
				procHandle ? CloseHandle(procHandle) : 0;
			}

			//check if the current handle is valid, otherwise increment i and check the next handle
			if (!IsHandleValid((HANDLE)hInfo->Handles[i].Handle))
			{
				continue;
			}

			//check the handle type is 0x7 meaning a process handle so we dont hijack a file handle for example
			if (hInfo->Handles[i].ObjectTypeNumber != ProcessHandleType)
			{
				continue;
			}


			//set clientID to a pointer to the process with the handle to out target
			clientID.UniqueProcess = (DWORD*)hInfo->Handles[i].ProcessId;

			//if procHandle is open, close it
			procHandle ? CloseHandle(procHandle) : 0;

			//create a a handle with duplicate only permissions to the process with a handle to our target. NOT OUR TARGET.
			NtRet = NtOpenProcess(&procHandle, PROCESS_DUP_HANDLE, &Obj_Attribute, &clientID);
			if (!IsHandleValid(procHandle) || !NT_SUCCESS(NtRet)) //check is the funcions succeeded and check the handle is valid
			{
				continue;
			}


			//we duplicate the handle another process has to our target into our cheat with whatever permissions we want. I did all access.
			NtRet = NtDuplicateObject(procHandle, (HANDLE)hInfo->Handles[i].Handle, NtCurrentProcess, &HijackedHandle, PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_DUP_HANDLE, 0, 0);
			if (!IsHandleValid(HijackedHandle) || !NT_SUCCESS(NtRet))//check is the funcions succeeded and check the handle is valid
			{

				continue;
			}

			//get the process id of the handle we duplicated and check its to our target
			if (GetProcessId(HijackedHandle) != dwTargetProcessId) {
				CloseHandle(HijackedHandle);
				continue;
			}



			hProcess = HijackedHandle;

			break;
		}

		procHandle ? CloseHandle(procHandle) : 0;
		
		return hProcess;
	}
}
