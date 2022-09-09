
#include "ClientManager.h"

using namespace std;

int main() {

	// ENTRY POINT
	ClientManager Server;


	auto thread1 = std::async(std::launch::async, [&Server] { Server.run(); });
	
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 60));
	
	Server.terminate();
	std::cout << "Server terminated..." << std::endl;



	while (true)
	{





	}



}

