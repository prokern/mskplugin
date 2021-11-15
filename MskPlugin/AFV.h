#pragma once
#include <string>
#include <mutex>
#include <set>
#include <queue>

using namespace std;

class AFV
{
private:
	HHOOK __globalAFVHook = NULL;
	mutex __afv_lock;
	HWND __hiddenWindow = NULL;
	queue<string> __messages;
	void __FN_AFV();
	int __rxcount = 0;
	set<string> activeTransmittingPilots;
	set<string> previousActiveTransmittingPilots;

public:
	void AddAFVMessageToQueue(string message);
	bool IsTransmit(string CS);
	bool IsActiveTransmit(string CS);

	string GetPilotsForDraw();

	AFV();
	~AFV();
};

