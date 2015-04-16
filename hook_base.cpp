#pragma comment(lib, "detours.lib")

#include <Windows.h>
#include <stdio.h>
#include "detours.h"
#include "item_set.h"
#include "toggle_options.h"

#ifndef __cplusplus
extern "C" {
#endif

#define SCORE_MAX_LENGTH 64
#define NUM_OPTIONS 4

typedef void (__stdcall *presources_changed)(short int usage_type, float queued, int unused);
typedef int (__stdcall *pscore_update)(int always_five, const char *player, int rgb_value, int unk1, int unk2,
    int unk3, int unk4, int unk5, int unk6, int unk7, int unk8);
int detour(void **function_to_detour, void *replacement, bool attach);

presources_changed resources_changed = (presources_changed)(0x00555440);
pscore_update score_update = (pscore_update)(0x0054A510);

item_set *base_pointers = NULL;
item_set *temp_pointer = NULL;
item_set *stats = NULL;
unsigned int toggle_option = 0;

__declspec(naked) void resources_changed_hook(short int res_type, float usage_type, int unused) {
	__asm {
		pushad
		mov eax, temp_pointer
		mov dword ptr[eax], ecx         //temp_pointer->base_pointer points to calling class
    }
    temp_pointer->player_name = (char*)(*(temp_pointer->base_pointer + (0x98 / sizeof(DWORD_PTR))));
    temp_pointer->player_stat = (player_stats*)(*(temp_pointer->base_pointer + (0xA8 / sizeof(DWORD_PTR))));
    if(items_insert(&base_pointers, temp_pointer) == true)
        temp_pointer = (item_set*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(item_set));
    __asm {
		popad
		jmp resources_changed
	}
}

__declspec(naked) int score_update_hook(int always_five, char *player, int rgb_value, int unk1, int unk2,
    int unk3, int unk4, int unk5, int unk6, int unk7, int unk8) {
    __asm pushad
    char *name; //Placeholder for address of name buffer
    __asm {
        mov ebx, dword ptr[esp+0x28]
        mov name, ebx
    }
    stats = items_find_by_name(&base_pointers, name);
    if(stats != NULL) {
        if(toggle_option == CURRENT_RES)
            _snprintf(name, SCORE_MAX_LENGTH, "W:%1.0f  F:%1.0f  G:%1.0f  S:%1.0f\0",
            stats->player_stat->wood, stats->player_stat->food, stats->player_stat->gold,
            stats->player_stat->stone);
        else if(toggle_option == ALL_RES)
            _snprintf(name, SCORE_MAX_LENGTH, "W:%1.0f  F:%1.0f  G:%1.0f  S:%1.0f\0",
            stats->player_stat->total_wood_gathered, stats->player_stat->total_food_gathered,
            stats->player_stat->total_gold_gathered, stats->player_stat->total_stone_gathered);
        else if(toggle_option == POP_AGE)
            _snprintf(name, SCORE_MAX_LENGTH, "Pop: %1.0f/%1.0f  Vil:%1.0f  Mil:%1.0f  Age:%1.0f\0",
            stats->player_stat->pop_current, (stats->player_stat->pop_current + stats->player_stat->pop_left),
            stats->player_stat->num_villagers, stats->player_stat->num_military, stats->player_stat->current_age);
    }
    __asm {
        popad
        jmp score_update
    }
}

LRESULT __declspec(dllexport) CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if(code < 0 || GetAsyncKeyState(wParam) == -32767)
		return CallNextHookEx(NULL, code, wParam, lParam);
    if(code == HC_ACTION) {
        if(wParam == VK_F5) {
            temp_pointer = (item_set*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(item_set));
            detour(&(PVOID&)resources_changed, resources_changed_hook, true);
            detour(&(PVOID&)score_update, score_update_hook, true);
        }
        else if(wParam == VK_F6) {
            items_clear(&base_pointers);
            detour(&(PVOID&)resources_changed, resources_changed_hook, false);
            detour(&(PVOID&)score_update, score_update_hook, false);
        }
        else if(wParam == VK_F7)
            toggle_option = (toggle_option + 1) % NUM_OPTIONS;
        else if(wParam == VK_F8)
            items_clear(&base_pointers);
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

int detour(void **function_to_detour, void *replacement, bool attach) {
    int ret = NO_ERROR;
    if((ret = DetourTransactionBegin()) != NO_ERROR)
        return ret;
    if((ret = DetourUpdateThread(GetCurrentThread())) != NO_ERROR) 
        return ret;
    attach == true ? (ret = DetourAttach(function_to_detour, replacement)) :
        (ret = DetourDetach(function_to_detour, replacement));
    if(ret != NO_ERROR)
        return ret;
    return DetourTransactionCommit();
}

int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID reserved) {
	if(fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
	}
	return TRUE;
}

#ifndef __cplusplus
}
#endif