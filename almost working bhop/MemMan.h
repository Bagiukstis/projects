#pragma once
#include <Windows.h>
#include <vector>
#include "csgo.h"
#include <d3dx9.h>
#include "xmmintrin.h"
#include <TlHelp32.h>


class MemMan
{
public:
	MemMan();
	~MemMan();
	template <class val>
	val readMem(DWORD addr)
	{
		val x;
		ReadProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
		return x;
	}
	template <class val>
	val writeMem(DWORD addr, val x)
	{
		WriteProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
		return x;
	}
	DWORD getProcess(const char*);
	uintptr_t getModule(DWORD, const char*);
	DWORD getAddress(DWORD, std::vector<DWORD>);
	HANDLE handle;
	DWORD GetClosestTarget(int fov);
	D3DXVECTOR3 GetLocalEyePos();
	DWORD GetEntFromIndex(int i);
	D3DXVECTOR3 GetBonePos(DWORD player, int bone);
	D3DXVECTOR3 CalcAngle(const D3DXVECTOR3& src, const D3DXVECTOR3& dst);
	D3DXVECTOR3 ClampAngles(D3DXVECTOR3 angle);
	void VectorAngles(D3DXVECTOR3 forward, D3DXVECTOR3& angles);
	bool AimAtPlayer(DWORD player, float smooth, int bone);
	bool IsValid(DWORD player);
};
