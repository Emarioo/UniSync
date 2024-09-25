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

## File monitor


## Server/client
These are the messages we want to send:
- 

# Interface