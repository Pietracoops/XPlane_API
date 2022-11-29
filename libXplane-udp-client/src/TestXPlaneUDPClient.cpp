#include "ClientManager.h"

#include <iostream>
#include <future>

#include "Log.h"

using namespace std;
bool application_terminated = false;
BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
	if (CTRL_CLOSE_EVENT == dwCtrlType) 
	{
		application_terminated = true;
		return TRUE;
	}

	return FALSE;
}


int main() {

	// ENTRY POINT
	Log::Init();

	ClientManager server;
	
	if (FALSE == SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE)) { }

	server.run();

	server.terminate();
	std::cout << "Server terminated..." << std::endl;

	return 1;
}

