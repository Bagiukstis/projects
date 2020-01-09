#include "MemMan.h"
#include "Windows.h"
#include <TlHelp32.h>
#include "Global.h"
#include <iostream>
#define PI 3.14159265359f
extern values val;
extern MemMan MemClass;
MemMan::MemMan()
{
	handle = NULL;
}

MemMan::~MemMan()
{
	CloseHandle(handle);
}

DWORD MemMan::getProcess(const char* proc)
{
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD process;
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);

	do
	{
		if (!strcmp(pEntry.szExeFile, proc))
		{
			process = pEntry.th32ProcessID;
			CloseHandle(hProcessId);
			handle = OpenProcess(PROCESS_ALL_ACCESS, false, process);
		}

	} while (Process32Next(hProcessId, &pEntry));
	return process;
}

uintptr_t MemMan::getModule(DWORD procId, const char* modName)
{
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);

	do
	{
		if (!strcmp(mEntry.szModule, modName))
		{
			CloseHandle(hModule);
			return (DWORD)mEntry.hModule;
		}
	} while (Module32Next(hModule, &mEntry));
	return 0;
}

DWORD MemMan::getAddress(DWORD addr, std::vector<DWORD> vect)
{
	for (unsigned int i = 0; i < vect.size(); i++)
	{
		ReadProcessMemory(handle, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += vect[i];
	}
	return addr;
}
DWORD MemMan::GetClosestTarget(int fov) {
	D3DXVECTOR3 viewAngles = MemClass.readMem<D3DXVECTOR3>(val.clientstate + csgo::dwClientState + csgo::dwClientState_ViewAngles);
	D3DXVECTOR3 localEyePos = GetLocalEyePos();

	DWORD bestEntity = 0;
	for (int i = 0; i <= 64; i++) { //never more than 64 players
		DWORD cEnt = GetEntFromIndex(i);
		int entTeam = MemClass.readMem<int>(cEnt + csgo::m_iTeamNum); // our teammate
		if (cEnt != NULL && IsValid(cEnt) && entTeam != val.localTeam && cEnt != val.localPlayer){ //there are players in the server, we check that the player is dormant and alive + we dont want to shoot our teammates and oursevles
			D3DXVECTOR3 angle = CalcAngle(localEyePos, GetBonePos(cEnt, 8)); //index for the head is 8
			D3DXVECTOR3 cAngles = ClampAngles(angle - viewAngles);
			float delta = sqrt(cAngles.x * cAngles.x + cAngles.y * cAngles.y); //distance from the center
			if (delta < fov) {
				fov = delta;
				bestEntity = cEnt;
			}
		}
	}
	return bestEntity;
}

bool MemMan::AimAtPlayer(DWORD player, float smooth, int bone) {
	if (player != NULL) {
		D3DXVECTOR3 localEyePos = GetLocalEyePos();
		D3DXVECTOR3 enemyPos = GetBonePos(player, bone); //body is 4

		D3DXVECTOR3 aimAngles = enemyPos - localEyePos;
		VectorAngles(aimAngles, aimAngles);
		aimAngles -= MemClass.readMem<D3DXVECTOR3>(val.localPlayer + csgo::m_aimPunchAngle) *smooth;

		D3DXVECTOR3 cAngles = MemClass.readMem<D3DXVECTOR3>(val.clientstate + csgo::dwClientState_ViewAngles);
		D3DXVECTOR3 delta = aimAngles - cAngles;
		D3DXVECTOR3 sAngles = ClampAngles(cAngles + (delta / smooth));

		MemClass.writeMem<D3DXVECTOR3>(val.clientstate + csgo::dwClientState_ViewAngles, sAngles);
		return true;
	}
	return false;
}

D3DXVECTOR3 MemMan::ClampAngles(D3DXVECTOR3 angle) {
	while (angle.x < -180.0f)
		angle.x += 360.0f;

	while (angle.x > 180.0f)
		angle.x -= 360.0f;

	if (angle.x > 89.0f)
		angle.x = 89.0f;

	if (angle.x < -89.0f)
		angle.x = -89.0f;

	while (angle.y < -180.0f)
		angle.y += 360.0f;

	while (angle.y > 180.0f)
		angle.y -= 360.0f;

	angle.z = 0.0f;

	return angle;
}

D3DXVECTOR3 MemMan::CalcAngle(const D3DXVECTOR3& src, const D3DXVECTOR3& dst)
{
	//square root func faster than normal func youd use
	const auto sqrtss = [](float in)
	{
		__m128 reg = _mm_load_ss(&in);
		return _mm_mul_ss(reg, _mm_rsqrt_ss(reg)).m128_f32[0];
	};


	D3DXVECTOR3 angles;

	//getting delta between source and destination vectors
	D3DXVECTOR3 delta = src - dst;

	//finding the hypoteneuse using pythagoras theorem a squared + b squared = c squared
	//this gives us the vector to our enemy
	float hyp = sqrtss(delta.x * delta.x + delta.y * delta.y);

	//now we need to find the angle needed to aim at the vector (aim angles)
	angles.x = asinf(delta.z / hyp) * (180 / PI);
	angles.y = atanf(delta.y / delta.x) * (180 / PI) + !((*(DWORD*)& delta.x) >> 31 & 1) * 180;

	angles.z = 0;

	return angles;
}
void MemMan::VectorAngles(D3DXVECTOR3 forward, D3DXVECTOR3& angles)
{
	float yaw;
	float pitch;

	if (forward.z == 0 && forward.x == 0)
	{
		yaw = 0;
		pitch = 270;
	}
	else
	{
		float tmp;
		yaw = (atan2(forward.y, forward.x) * 180 / PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward.x * forward.x + forward.y * forward.y);
		pitch = (atan2(-forward.z, tmp) * 180 / PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles.x = pitch;
	angles.y = yaw;
	angles.z = 0;
}
D3DXVECTOR3 MemMan::GetLocalEyePos() {
	D3DXVECTOR3 LocalPos = MemClass.readMem<D3DXVECTOR3>(val.localPlayer + csgo::m_vecOrigin);
	LocalPos += MemClass.readMem<D3DXVECTOR3>(val.localPlayer + csgo::m_vecViewOffset); //adds to localpos
	return LocalPos;
}
D3DXVECTOR3 MemMan::GetBonePos(DWORD player, int bone) {
	Matrix3x4_t boneMatrix = MemClass.readMem<Matrix3x4_t>(MemClass.readMem<DWORD>(player + csgo::m_dwBoneMatrix) + bone * 0x30);

	return{
		boneMatrix.Matrix[0][3], //x angles
		boneMatrix.Matrix[1][3], //y
		boneMatrix.Matrix[2][3] //z
	};
}



DWORD MemMan::GetEntFromIndex(int i) {
	return MemClass.readMem<DWORD>(val.client + csgo::dwEntityList + (i * 0x10));
}
bool MemMan::IsValid(DWORD player) {
	bool dormant = MemClass.readMem<bool>(player + csgo::m_bDormant);
	int hp = MemClass.readMem<int>(player + csgo::m_iHealth);
	bool alive;
	if (hp >= 0) {
		alive = true;
	}
	return(!dormant && alive); // we return that the player is dormant and that he is alive :p
}