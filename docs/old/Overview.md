# Idea
An application which allows you to synchronize files between computers.

It would also be cool if you could turn UniSync into a library or console program which you can include in other projects. Like a code editor where you are multiple people working on a project and it synchronizes things in real time. (you would use git for large projects but in small projects for studies it could prove useful)
## User
The user's application holds information about which directories and files are sync units and which sync units they belong to. To access and download a sync unit the user needs identify the name of the unit and the password. Next step is to tell the application where the sync unit should be (what folder). When the user changes a synchronized file, the application will notice and send an update request to the server.

## Server
The server stores the sync units and transfers requests between users. When the server receives an update request the sync unit is updated with the change. Connected clients will then be notified of the change and updated.
# User interaction

## Renaming connection
Do nothing if not connected. Otherwise, send the names of your connections

When renaming is done, if there is a connection, send a syncunit name to connection. If it exists send a syncunit_link back, client then sends it's files to the server syncunit_link. see if the unit exists on the connection. If so, link them. Files that exist on the connection should be sent to the client and files on the client should be sent to the server. Begin with a pending request.
## Features
- Filtering files. like exe format
- When you hold a button like key 'i' and the down arrow key. When you pass files you should toggle the ignore status. It will be so smooth and nice

# Structure
![[overview.png]]
![[design.png]]
## Connection
A connection with ip and port.
- <span style="color:green" >Linked</span>: If the connection is active.
- <span style="color:red" >Failed</span>: If the connection has failed.
- <span style="color:gold" >Auto</span>: Automatically connect.
## Sync Unit
The sync unit contains a name, password, root and a list of files. The files are listed with directories up to a root directory and every file has a time when it was last updated.

## Sync File
An abstraction of a file. Contains filepath from the root, timestamp when it was last updated and statuses. 
- <span style="color:green">Static</span>: The file cannot be moved or deleted during synchronization.
- <span style="color:red">Pending</span>: The file is waiting to be accepted.
- <span style="color:gray">Ignore</span>: The file is ignored by the program.
## Synchronization
#### Local update
Looping through files in local folder and program save file

	If local doesn't exist
		add sync file
		send syncunit_1
	If file has changed
		send syncunit_1
	If new file
		send syncunit_1

#### Syncunit
Looping through files from msg and local syncfiles

	If remote file is newer
		send syncunit_2
	If remote file is older
		send file data
	If file is simular in timestamp
		do nothing
	if remote file doesn't exist
		create pending file
	if msg type is syncunit_1 (prevent com loop) and remote don't have file
		send syncunit_2