
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;
	bool &active;
	// Data to send to server
	ByteArray data;
	std::string data_str;
public:
	ClientThread(Socket& socket, bool &active)
	: socket(socket), active(active)
	{}

	~ClientThread()
	{}

	virtual long ThreadMain()
	{
		while(true) {
			try {
				std::cout << "Please input your data (done to exit): ";
				std::cout.flush();

				// Get the data
				std::getline(std::cin, data_str);
				data = ByteArray(data_str);

				//if done, break this thread
				if(data_str == "done") {
					active = false;
					break;
				}

				// Write to the server
				socket.Write(data);

				// Get the response
				int connection = socket.Read(data);

				//if connection to server is gone, break thread
				if(connection == 0) {
					active = false;
					break;
				}

				std::cout<<"Server Response: " << data.ToString() << std::endl;
			} catch (std::string err)
            {
                std::cout<<err<<std::endl;
            }
		}

		return 0;
	}
};

int main(void)
{
	// Welcome the user 
	std::cout << "SE3313 Lab 3 Client" << std::endl;
	bool active = true;

	// Create our socket
	Socket socket("127.0.0.1", 3000);
	ClientThread clientThread(socket, active);
	socket.Open();

	while(active) {
		sleep(1);
	}

	socket.Close();
	
	return 0;
}
