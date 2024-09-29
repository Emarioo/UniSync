This is the network design.

# The process
We have a scenario with a client and a server. They both have a unit of files that they want to synchronize with each other. This is the process how that is done.

Step 1: A client connects to the server.
Step 2: On the connect event on both sides, both client and server send data about which units of files they have.
Step 3: The program (both sides) look at the received file information and decide whether they want that file based on whether it's new.
Step 4: A program that wants a file sends a request to receive that file. Then the other side will send that file. Programs will ignore files that are sent without a request. This would indicate malicious activity, someone trying to forcefully send files.


## Messages
Both endpoints in a connection can send the same type of messages.

All messages have this header:
|Field|Data|
|-|-|
|Type, 4B|SYNC_UNITS_AND_FILES / REQUEST_FILE / STREAM_FILE|
|Version, 4B|Usually just 1|

### **SYNC_UNITS_AND_FILES**
An endpoint sends this type of message at startup. It sends it's units and files so that the other endpoint has
knowledge of which end points are up to date and old.

This is the format of the message (excluding message header):
|Field|Data|
|-|-|
|Number of units, 4B|The number of units|
|Unit data|Data for number of units|

### **REQUEST_FILE**
An endpoint sends a request

### **STREAM_FILE**


# Vulnerabilites
An endpoint will not accept streams of a file unless it has been request. A malicious actor can however send SYNC_UNITS_AND_FILES where all files have a very high timestamp which causes the endpoint to request all files. The malicious can then send files.

So, how do we keep ourselves safe?

We would like to skip files deemed suspicius like executables. But if we are synchronising files with code, a malicious person could modify the code file to contain something bad. Do we just not allow any code files, shell scripts, python, executables, object files, web files?