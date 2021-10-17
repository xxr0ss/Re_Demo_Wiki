#include <Windows.h>

#define _FLOWER_1 { \
	_asm xor eax, eax \
	_asm jz $+7 \
	_asm _emit 0xe8 \
}

#define _FLOWER_2 { \
	_asm jb $ + 13 \
	_asm jnb $ + 7 \
	_asm _emit 0xe3 \
}

#define _FLOWER_3 { \
	_asm call $ + 11 \
	_asm _emit 0xeb \
	_asm jmp $ + 10 \
	_asm add dword ptr[esp], 1 \
	_asm retn \
}



int main() {
	_FLOWER_1;
	MessageBoxA(NULL, "", "", MB_OK);
}