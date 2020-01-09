#include <Windows.h>
#include "MemMan.h"
#include <iostream>
#include <d3dx9.h>
#include "csgo.h"
#include "Global.h"
values val;
MemMan MemClass;

void bhop(){
	while (true) {
		val.flag = MemClass.readMem<BYTE>(val.localPlayer + csgo::m_fFlags);
		if (GetAsyncKeyState(VK_SPACE) && val.flag & (1 << 0))
			//std::cout << "can jump" << std::endl;
			MemClass.writeMem<DWORD>(val.client + csgo::dwForceJump, 6);
			Sleep(1);		
		}
	}
	void aim() {
		while(true){
			if (GetAsyncKeyState(0x58)) {
				//std::cout << "pressing the key" << std::endl;
			//target selection
			//GetClosestTarget(10);
			//local angles
				DWORD player = MemClass.GetClosestTarget(10);
				MemClass.AimAtPlayer(player, 1.f, 8);
				Sleep(1);
			}
}
}
int main()
{
	val.process = MemClass.getProcess("csgo.exe"); //store ID to values struct
	val.client = MemClass.getModule(val.process, "client_panorama.dll");
	val.engine = MemClass.getModule(val.process, "engine.dll"); 
	val.clientstate = MemClass.readMem<DWORD>(val.engine + csgo::dwClientState);
	//std::cout << std::hex << MemClass.val.engine << std::endl;
	std::cout << csgo::m_iTeamNum << std::endl;
	val.localPlayer = MemClass.readMem<DWORD>(val.client + csgo::dwLocalPlayer);
	val.localTeam = MemClass.readMem<int>(val.localPlayer + csgo::m_iTeamNum);
	//std::cout << val.localPlayer << std::endl;
	//std::cout << val.localTeam << std::endl;
	std::cout << "on:D" << std::endl;
	
	if (val.localPlayer == NULL) //If local player is equal to null we r gonna run the loop to make sure its not while we r playing
		while (val.localPlayer == NULL)
			val.localPlayer = MemClass.readMem<DWORD>(val.client + csgo::dwLocalPlayer);
	bhop();
	aim();
}
