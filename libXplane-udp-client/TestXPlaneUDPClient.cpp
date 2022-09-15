
#include "ClientManager.h"

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
	ClientManager Server;



	auto thread1 = std::async(std::launch::async, [&Server] { Server.run(); });
	
	
	if (FALSE == SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE))
	{
		// Cannot register your handler? Check GetLastError()
	}

	while (!application_terminated) 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 60));
	}


	Server.terminate();
	std::cout << "Server terminated..." << std::endl;

	return 1;
}

