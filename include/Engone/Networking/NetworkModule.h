#pragma once

#include "Engone/Networking/Server.h"
#include "Engone/Networking/Client.h"

#define NETWORK_LEVEL_ALL 0xffffffff
#define NETWORK_LEVEL_INFO 0x01
#define NETWORK_LEVEL_SPAM 0x08
#define NETWORK_LEVEL_WARNING 0x02
#define NETWORK_LEVEL_ERROR 0x04
#define NETWORK_LEVEL 841725

/*
	Asynchronous Client/Server library using WinSock

	Why library is good
	- It is lightweight.
	- It is easy to use.
	- The structure of the library is made so that you can call any functions whenever without breaking anything.

	Bad points
	- Only works for Windows. (rip mac and linux)
	- Mutex and threads may not be flawless. Some testing needs to be done.
	
	Things to watch out for when dealing with mutex and threads

		thread = std::thread([](){
			mutex.lock();
			...
		});
		mutex.lock();
		thread.join();
		
		this code will freeze if thread is created, mutex is locked in main thread, joining thread and finally mutex inside thread is locked.
		doing thread.join before mutex.lock will fix it but may cause wierd behavior if other threads are doing things to the thread.
		maybe use a threadMutex as well.

	Notes:
		Client::stop will freeze if you called start before and it's still trying to connect. This is because the worker thread is busy.
			You can fix it by using another thread.

		Should there be some consistencies where Stopped Event always showing up after Disconnect events for server.
		At least, Disconnect event should always show up.

		Creating MessageBuffers requires allocating memory. If send 100 messages every second that would be a lot of allocations. (especially since the buffer is reallocated when you push more data)
		To improve this, the sender can have a backlog of available messages. You then request a MessageBuffer from the sender(server or client) and if the backlog is used up.
		A new MessageBuffer is created. This way you reuse buffers and minimize the amount of allocations.
		The question then is when to clean the backlog. Say you need a message with 1 MB once in your program. Then it would be a waste of memory to reuse that
		buffer during the whole program. The message should be deleted when not needed. The sender needs to be smart about what messages it is reusing for which request of it.
		If you need 100 bytes then the messageBuffer should not provide a message with 100000 bytes. This optimization requires the requester to know the size in advance.
		Which isn't always the case.
		
		Message Requesting should be optional. You should be able to allocate your own message buffer if you want.

		ISSUE: Using both asynchronous and synchronous when sending message will two messages at the same time. This could be bad depending on how winsock deals with this.

*/

namespace engone {
	// Will initialize itself if you make a server or client.
	void InitNetworking();
	// Engone will call this when destroyed.
	void DestroyNetworking();
}