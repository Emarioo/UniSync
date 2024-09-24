Potential issues you have found but not tested or not sure if they are issues should be put here.

- [ ] When connecting to a socket in a thread the user could remove the connection item. If so, the pointer to the connection is longer the original connection. Undefined behavior.
- [ ] If you change name, time or root of the unit at the same time as using does when sending a network message. Something could go wrong. You edit a string which when sending seems to have one length but in reality has a much shorter length.
- [ ] Can you access files in System32 from a relative path without `..` because I heard something about links, shortcuts or mounting in UNIX systems. Should be fine on windows but who knows. Could also just ignore paths which have system32 in them but there may be other important files in other directories.
- [ ] When a file is deleted. It is deleted from the client. A sync message is sent. The server sees the deleted file as missing and sends a new file to the client. How to prevent it?
- [x] Program seems to freeze randomly when connecting?
- [x] Program freezes when many files are loaded. Is is because the network is busy?
- [x] Remove any `/../` in a any path. If you on the client hack in a file path like libs/../../../../System32/something.dll you might be able to destroy the server or another persons computer.