#include <stdio.h>
#include <string.h>
#include <windows.h>

//#define EASYINJECTION
#define TEXTFILE_OUTPUT
	
	
int __stdcall check_ignore_msg(unsigned int msg);
void __stdcall lol_OutputDebugString(unsigned int msg);


int tlsIndex;


int __stdcall func2() {
	if (TlsGetValue(tlsIndex) == 0) {
		TlsSetValue(tlsIndex, (void*)((int)TlsGetValue(tlsIndex) + 1));
		return 1;
	}
	return 0;
}

void __stdcall func3() {
	TlsSetValue(tlsIndex, (void*)((int)TlsGetValue(tlsIndex) - 1));
}


unsigned int D3__std__String = 0x3CE4A528;
unsigned int D3__TextFormat__PrintToString = 0x3CB4E280;
unsigned int D3__Descriptor__full_name = 0x3C9E0220;
unsigned int D3__Message__GetDescriptor = 0x3CAEB630;
unsigned int hook_return = 0x3CAD8894;
unsigned int outputhook_return = 0x3CD1C1D4;
unsigned int outputhead_return = 0x3CD03639;


#define REBASE(lib, var, addr, oldbase) var = lib + addr - oldbase;

void RebaseFunctions()
{
	unsigned int bnetlib = (unsigned int) GetModuleHandle("bnet/battle.net.dll");
	REBASE(bnetlib, D3__std__String, 0x3CE4A528, 0x3C910000)
	REBASE(bnetlib, D3__TextFormat__PrintToString, 0x3CB4E280, 0x3C910000)
	REBASE(bnetlib, D3__Message__GetDescriptor, 0x3CAEB630, 0x3C910000)
	REBASE(bnetlib, D3__Descriptor__full_name, 0x3C9E0220, 0x3C910000)
	REBASE(bnetlib, hook_return, 0x3CAD8894, 0x3C910000)
	REBASE(bnetlib, outputhook_return, 0x3CD1C1D4, 0x3C910000)
	REBASE(bnetlib, outputhead_return, 0x3CD03639, 0x3C910000)
	
}

char* D3__std__string_to_char(unsigned int astr) 
{
	if (*(int *)(astr + 0x18) > 0x0f) {
		return *(char **)(astr + 4);
	} else {
		return (char *)(astr + 4);
	
	}
}

void __stdcall LDebugString(const char* stringptr) 
{
    #ifdef TEXTFILE_OUTPUT
	FILE *file;	
	file = fopen("dbgoutput.txt","a+");       
    fprintf(file, "%s\n", stringptr);  
    fclose(file); 
	#endif
	
	OutputDebugString(stringptr);
}

void __stdcall lol_OutputDebugString(unsigned int msg) 
{
	char* stringptr = D3__std__string_to_char(msg);
	
    LDebugString(stringptr);
}

void hexdump(char* data, int size, char * outbuf, int osize) {
	char *abuf = outbuf;
	int remaining = osize - 4;
	for (int i=0; i < size; i++) {
		unsigned int next = (unsigned char) *data++;
		if (next < 0x10) {
			snprintf(abuf, remaining, "0%x ",next);
		} else {
			snprintf(abuf, remaining, "%x ", next);
		}
		abuf+=3;
		remaining-=3;
		if ((i+1) % 16 == 0) {
			snprintf(abuf, remaining, "\n");
			abuf += 1;
			remaining -= 1;
		}
	}
}

void __stdcall print_send_header(int eax, int edi) {
	try {
		char buf[200];
		char hexout[200];
		hexdump((char*)edi, eax, hexout, sizeof(hexout));
		snprintf(buf, sizeof(buf)-1, "[ Send %s ]", hexout);
		LDebugString(buf);
	}
	catch( char *str )    {
       LDebugString(str);
   }	
}
void __stdcall print_recv_header(int eax, int edi) {
	try {
	char buf[200];
	char hexout[200];
	hexdump((char*)edi, eax, hexout, sizeof(hexout));
	snprintf(buf, sizeof(buf)-1, "[ Recv %s ] ", hexout);
	LDebugString(buf);
	}
	catch( char *str )    {
       LDebugString(str);
   }	
}
void __stdcall print_msg(int message)
{
	asm("		sub		$0x50, %esp\n\t");

	//alloc std::string
	asm("		lea		-0x30(%ebp), %ecx\n\t"
		"		movl	$0, 4(%ecx)\n\t");
	asm("		mov		%0, %%eax\n\t" : : "m"(D3__std__String));
	asm("		call	*(%eax)\n\t");
	
	//print messagetype
	asm("		movl	0x08(%ebp), %ecx\n\t");
	asm("		mov		%0, %%eax\n\t" : : "m"(D3__Message__GetDescriptor));
	asm("       call    *%eax\n\t"
		"		movl	%eax, %ecx\n\t");
	asm("		mov		%0, %%eax\n\t" : : "m"(D3__Descriptor__full_name));
	asm("       call    *%eax\n\t");
	asm("		movl	%eax, -0x34(%ebp)\n\t");//store result
	
	asm("		push	%eax\n\t");
	asm("		mov		%0, %%eax\n\t" : : "i"(check_ignore_msg));
	asm("       call    *%eax\n\t");
	asm("       test    %eax, %eax\n\t");
	asm("       jnz     1f\n\t");
	
	asm("		movl	-0x34(%ebp), %eax\n\t");
	asm("		push	%eax\n\t");
	asm("		mov		%0, %%eax\n\t" : : "i"(lol_OutputDebugString));
	asm("       call    *%eax\n\t");

	//print message content
	asm("		lea		-0x30(%ebp), %ecx\n\t"
		"		push	%ecx\n\t"
		"		movl	0x08(%ebp), %eax\n\t"
		"		push	%eax\n\t");
	asm("		mov		%0, %%eax\n\t" : : "m"(D3__TextFormat__PrintToString));
	asm("		movl	$0, %ecx\n\t"
		"		call	*%eax\n\t"
        "       add		$8, %esp\n\t"
		"		lea		-0x30(%ebp), %ecx\n\t"
		"		push	%ecx\n\t"
		);
	asm("		mov		%0, %%eax\n\t" : : "i"(lol_OutputDebugString));
	asm("       call    *%eax\n\t");

	asm("		1:\n\t"); //label to end
	asm("		add		$0x50, %esp\n\t");
}

void func1()
{
    asm(
		"		pop 	%ebp\n\t"

		"       movl     0x10(%eax), %edx\n\t"
        "       call    *%edx\n\t"
        "       movzx   %al, %eax\n\t"

		"		push    %eax\n\t"

		"		push 	0x0c(%ebp)\n\t" //param to the previous function
		"		push	%ebp\n\t"
		"		mov		%esp, %ebp\n\t"

		"		sub		$0x200, %esp\n\t"
		"		pusha\n\t"
	);
	
	//skip if anidated:
	asm("		mov		%0, %%eax\n\t" : : "i"(func2)); 
	asm("       call    *%eax\n\t"
		"		test	%eax, %eax\n\t"
		"		jz		1f\n\t");
	
	//print the message:
	asm("		movl	0x04(%ebp), %eax\n\t"
		"		push	%eax\n\t");
	asm("		mov		%0, %%eax\n\t" : : "i"(print_msg));
	asm("       call    *%eax\n\t");
	
	asm("		mov 	%0, %%eax\n\t" : : "i"(func3));
	asm("       call    *%eax\n\t"
		"		1:\n\t"

		"		popa	\n\t"
		"		add		$0x200, %esp\n\t"

		"		pop 	%ebp\n\t"
		"		pop 	%eax\n\t" //param
		"		pop 	%eax\n\t");

	asm("		push	%0\n\t" : : "m"(hook_return));
	asm("		ret		\n\t");
}

void __stdcall outputhook_(int message) {
	try {
		print_msg(message);
	} 
	catch(char*str) {
		LDebugString("exception:");
		LDebugString(str);
	}
}

void outputhook()
{
	asm(
		"	pop     %ebp\n\t"
		"	mov     -0x1c(%ebp),%eax\n\t"
		"	mov     (%ecx), %edx\n\t"
		"	mov     0x28(%edx), %edx\n\t"
	);
	asm("	pusha\n\t");
	asm("	push	%edi\n\t");
	asm("	push	%eax\n\t");
	asm("	mov		%0, %%eax\n\t" : : "i"(print_send_header));
	asm("	call    *%eax\n\t");
	asm("	popa\n\t");
	asm("	pusha\n\t");
	asm("	push	%ecx\n\t");
	asm("	mov		%0, %%eax\n\t" : : "i"(print_msg));
	asm("	call    *%eax\n\t");
	asm("	popa\n\t");
	asm("	push	%0\n\t" : : "m"(outputhook_return));
	asm("	ret		\n\t");
}

void outputhead() {
	asm("	pop %ebp\n\t");
	
	asm("	pusha\n\t");
	asm("	test	%eax, %eax\n\t");
	asm("	jz 1f\n\t");
	asm("	push	%edi\n\t");
	asm("	push	%eax\n\t");
	asm("	mov		%0, %%eax\n\t" : : "i"(print_recv_header));
	asm("	call    *%eax\n\t");
	asm("	1:\n\t");
	asm("	popa\n\t");
	
 asm (" mov     -0x2C4(%ebp), %cl");
 asm (" movzx   -0x2BC(%ebp), %ebx");
 asm ("	push	%0\n\t" : : "m"(outputhead_return));
 asm ("	ret		\n\t");
}

unsigned int __stdcall wrap_getname(unsigned int f)
{

	asm("		push 	%eax\n\t");
	asm("		pusha	\n\t");
	
	asm("		mov		8(%ebp), %ecx\n\t");

	asm("		mov		%0, %%eax\n\t" : : "m"(D3__Message__GetDescriptor));
	asm("       call    *%eax\n\t"
		"		movl	%eax, %ecx\n\t");
	asm("		mov		%0, %%eax\n\t" : : "m"(D3__Descriptor__full_name));
	asm("       call    *%eax\n\t");
	asm("       mov     %eax, -4(%ebp)\n\t");
	
	asm("		popa	\n\t");
	asm("		pop 	%eax\n\t");
}

int __stdcall check_ignore_msg(unsigned int msg)
{
	char* stringptr = D3__std__string_to_char(msg);

	if (strncmp("google.protobuf.FileDescriptorProto", stringptr, 35) == 0) {
		return 1;
	}
	return 0;
}

void hookPushRet( unsigned int address,	unsigned int jumpwhere)
{
  DWORD old;
  VirtualProtect((void*)address, 6, PAGE_EXECUTE_READWRITE, &old);
  *(unsigned char*)(address)  = 0x68;
  *(unsigned int*)(address+1)= jumpwhere;
  *(unsigned char*)(address+5) = 0xc3;
  VirtualProtect((void*)address, 6, old, &old);
  FlushInstructionCache(GetCurrentProcess(), (void*)address, 6);
}

void hookAddr( unsigned int address,	unsigned int calladdr)
{
  DWORD old;
  VirtualProtect((void*)address, 4, PAGE_EXECUTE_READWRITE, &old);

	*(unsigned int*)(address)  = calladdr;

  VirtualProtect((void*)address, 4, old, &old);
  FlushInstructionCache(GetCurrentProcess(), (void*)address, 4);
}

void TryHook() {
	HINSTANCE__* hlib = GetModuleHandle("battle.net.dll");
	if (hlib != 0) {
		RebaseFunctions();
		
		if (*(unsigned int*)((unsigned int)hlib + 0x3CD0362C - 0x3C910000) ==  (unsigned int)&outputhead) {
			LDebugString("Lib seems already hooked.");
			return;
		}
		hookPushRet((unsigned int)hlib + 0x3CD0362C - 0x3C910000, (unsigned int)&outputhead);

		hookPushRet((unsigned int)hlib + 0x3CAD888C - 0x3C910000, (unsigned int)&func1);
		hookPushRet((unsigned int)hlib + 0x3CD1C1CC - 0x3C910000, (unsigned int)&outputhook);

		LDebugString("Lib Should be hooked");
	} else {
		LDebugString("TryHook: battle.net.dll not loaded yet");
	}
}

HINSTANCE__* __stdcall loadlibrary_(LPCWSTR lpLibFileName) {
	HINSTANCE__* lib = LoadLibraryW(lpLibFileName);
	TryHook();
	return lib;
}

int __declspec(dllexport) __stdcall StartDll(int param)
{
  #ifdef TEXTFILE_OUTPUT
	FILE *file;	
	file = fopen("dbgoutput.txt","w+");       
    fprintf(file, "Starting Log\n");  
    fclose(file); 
  #endif
	
  if ((tlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
	MessageBox(NULL, "Error 0: cannt get tls slot", "",0);
	return 0;
  }
  
  //HINSTANCE hlib = LoadLibraryA("bnet/battle.net.dll");

  TryHook();
  
#ifndef EASYINJECTION
  hookAddr(0x011111F4, (unsigned int) &loadlibrary_);

  //code that jmps to the Entry Point of the exe
  TCHAR exepath[1000];
  if (0 == GetModuleFileName(0, exepath, 1000)){
	MessageBox(NULL, "Error 0: cannt getmodulefilename", "",0);
	return 0;
  }
  HINSTANCE__* mhandle = GetModuleHandle(exepath);

  PIMAGE_DOS_HEADER dos_header;
  PIMAGE_NT_HEADERS nt_header;
  dos_header = (PIMAGE_DOS_HEADER) mhandle;
  nt_header = (PIMAGE_NT_HEADERS)((unsigned int)mhandle + dos_header->e_lfanew);
  unsigned int entry_point = ((unsigned int)mhandle + nt_header->OptionalHeader.AddressOfEntryPoint);

  __asm(
          "mov %0, %%eax\n\t"
          "jmp *%%eax\n\t"
		  :
		  :"r"(entry_point)
		  :"%eax", "%ebx"
		);
#endif
}

BOOL WINAPI
DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            // Code to run when the DLL is loaded
			#ifdef EASYINJECTION
				StartDll(0);
			#endif
            break;

        case DLL_PROCESS_DETACH:
            // Code to run when the DLL is freed
            break;

        case DLL_THREAD_ATTACH:
            // Code to run when a thread is created during the DLL's lifetime
            break;

        case DLL_THREAD_DETACH:
            // Code to run when a thread ends normally.
            break;
    }
    return TRUE;
}