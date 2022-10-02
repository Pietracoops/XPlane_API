#include "ClientManager.h"

#include <iostream>
#include <future>

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
	ClientManager server;



	auto thread = std::async(std::launch::async, [&server] { server.run(); });
	
	
	if (FALSE == SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE))
	{
		// Cannot register your handler? Check GetLastError()
	}

	while (!application_terminated) { }


	server.terminate();
	std::cout << "Server terminated..." << std::endl;

	return 1;
}

