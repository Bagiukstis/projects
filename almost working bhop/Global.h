#pragma once
#include "Windows.h"
extern struct values {
	DWORD localPlayer;
	DWORD process;
	DWORD client;
	DWORD engine;
	BYTE flag;
	int localTeam;
	DWORD clientstate;
};

extern struct Matrix3x4_t {
	float Matrix[3][4];
};