#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
// This thread handles each client connection
class SocketThread : public Thread
{
private:
    // Reference to our connected socket
    Socket& socket;
    // The data we are receiving
    ByteArray data;
    //reference to terminate in ServerThread
    bool &terminate;

    //reference to sockThrHolder in ServerThread
    std::vector<SocketThread*> &sockThrHolder;
public:
    SocketThread(Socket& socket, bool &terminate, std::vector<SocketThread*> &clientSockThr)
    : socket(socket), terminate(terminate), sockThrHolder(clientSockThr)
    {}

    //termination of the thread
    ~SocketThread()
    {this->terminationEvent.Wait();}

    //method to get socket value of SocketThread
    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain() {

        try
        {
            while(!terminate)
            {
                
                //wait for data

                socket.Read(data);

                // Perform operations on the data
                std::string rec = data.ToString();

                //if message received is done, break the thread 
                if (rec=="done") {
                    sockThrHolder.erase(std::remove(sockThrHolder.begin(), sockThrHolder.end(), this), sockThrHolder.end());
                    terminate = true; 
                    break;      
                }
                
                rec.append("-analyzed");
                // Send it back
                socket.Write(ByteArray(rec));

            }
        //catching errors    
        }catch (std::string &s) {
            std::cout<<s<<std::endl;
        }
		
        catch (std::string err)
        {
            std::cout<<err<<std::endl;
        }
        std::cout<<"Client Disconnected" <<std::endl;
        return 0;
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    bool terminate = false;
    std::vector<SocketThread*> sockThrHolder; //array to hold all socketthreads
public:
    ServerThread(SocketServer& server)
    : server(server)
    {}

    //termination loops through all socketthreads and terminates their loop
    ~ServerThread()
    {
        // Cleanup
        for(auto thread: sockThrHolder) {
            try{
                Socket& toClose = thread->GetSocket();
                toClose.Close();
            } catch (...) {

            }
        } std::vector<SocketThread*>().swap(sockThrHolder);
        std::cout<<"Closing client from server"<<std::endl;
        terminate = true;
        
    }

    virtual long ThreadMain()
    {
        while (1) {
            try {
                // Wait for a client socket connection
                Socket* newConnection = new Socket(server.Accept());

                ThreadSem serverBlock(1);

                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference = *newConnection;
                sockThrHolder.push_back(new SocketThread(socketReference, terminate, std::ref(sockThrHolder)));
            } catch (std::string error)
            {
                std::cout << "ERROR: " << error << std::endl;
				// Exit thread function.
                return 1;
            }
			// In case of unexpected shutdown.
			catch (TerminationException terminationException)
			{
				std::cout << "Server has shut down!" << std::endl;
				// Exit with exception thrown.
				return terminationException;
			}
        }
        return 0;
    }
};


int main(void)
{
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to terminate the server...";
    std::cout.flush();
	
    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();

    // Shut down and clean up the server
    server.Shutdown();

}
