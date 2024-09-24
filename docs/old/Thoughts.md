I recently lost notes from a lecture about Agile and Scrum. This really sucks because I put a lot of effort in the quality of those notes. I wrote the notes in Obsidian and used Google Drive to synchronize the notes to the cloud. I suspect something strange happened with Google Drive and it deleted the notes from my computer. It could also be my fault but I really don't think so because I have lost text files before in a similar fashion.

LOSING DATA CANNOT HAPPEN with software which manages files like this (google drive). It's not okay!

This must be true with **UniSync** too! Files cannot be lost. It is crucial that which handles files and it's data doesn't have critical bugs. To do this the code must be:
- Simple, small, and easy to understand -> less mistakes, less bugs, easier to reason whether it has bugs or not.
- 


**Side note**: It's not a big deal that I lost the notes because the lecture was on video so I can write them again. It's probably even good to watch the lecture again and write them because I have forgotten a bit about it.


# Implementation
Code is messy. It's gonna take time but i thinks it's worth to rewrite it to achieve clarity and quality.

Client/Server
Unit with roots and files
Connections between clients and servers. (an application can have multiple client and server connections).
Secure with encryption and password

**On disc**
The files that are managed by the sync app are just files. You can move them and do whatever you want.
The addition is a `.unisync` file which stores the state of the synchronization.
UniSync also provides a file called `.unisync-user` which takes care of your profile. What connections do you have, what units do you have. Or maybe not? .unisync should contain units and roots.
