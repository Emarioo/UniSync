UniSync has two parts

- Synchronization code and data.
- User interface.


# Sync system
Our job is to synchronize files.

We need a server or client to communicate with others.

We need a file monitor to detect changes.

We need meta data, timestamps, sha-1 checksums too to see if files are new or old.

A thread for the server

A thread for file monitor (the Win32 api is blocking unless you want to get complicated)

Then a thread for rendering but that's separate.

What scenarios do we deal with?

No file has changed, do nothing
Client file changed, send to server
Client and server file changed, user solve collision manually

If a collision happens user decides which file to keep. They can use meld or other program to solve the difference.
UniSync only syncronizes, it doesn't solve collisions. It however download the file that is different but named 'the_file (remote)'.

## Synchronization of files
Some files we can't know how we should synchronize. So we ask the user which files to keep and delete (they can merge manually).

We should syncrhonize files we know won't collide. If files have the same sha1 then do nothing, they are the same.
How do we know if the 

## File monitor


## Server/client
These are the messages we want to send:
- 

# Interface