New tasks or thoughts about changes are put here.

# In focus
- [ ] Revise the SyncUnit/Connection structures.
- [ ] Separate rendering and sync logic.
- [ ] Revisit the network logic. Collisions aren't properly detected.
- Does units have ONE root or multiple. May it just has a concept of a list of files. That list is determined by the root directories you specify. Meaning: `1 unit != 1 directory`
# Security
- [ ] What happens if file responses are defragmented in strange ways or duplicated. An attacker may do so. Perhaps packets are corrupted for strange reasons. The application needs to handle this. Checksum would be good.
- [ ] When syncing. The server may send password to check with the client. Bad because the client now has the password.
- [ ] Implement password so that only people you want can link to the network. Otherwise a bad person could sync up a bunch of .exe files. **UPDATE:** Somewhat done but needs some testing.
- [ ] Before a client can send data to the server. The server needs to allow the client to do this. This could be done with passwords.
# Edge cases
- [ ] Does symlinks work?
# Other
- [ ] Design a method for testing the app from scripts using flags to the executable to setup the environment.
- [ ] Refactor the project. Especially the SyncApp class. The networking could probably be simplified too.
- [ ] Rename UserCache to something else. It's not a cache.
- [ ] When a file is deleted. It is deleted from the client. A sync message is sent. The server sees the deleted file as missing and sends a new file to the client. How to prevent it?
- [ ] Disable reconnect on startup. Do it in UserCache::load.
- Focus on file deletion. When files are removed. the pending request should be removed and unless static, files on disk should also be removed.
- Don't use keybindings. Only needed for games not apps like this.
# New features
- [ ] `--no-gui` flag. GDB-like user interface.
- [ ] The program makes it easy to make backups. That is comparing directories. You begin a backup session and choose some root folders to start from. Then you go through each folder/file and choose whether to merge/overwrite/skip or go into and make finer decisions. You also choose a folder where the backup goes. You choices are saved and you can use the same choices the next time you backup. You can do different choices but the recent ones are done by default.
- [ ] You may want to sync with one file or directory and not the rest in root. Done by freezing all new files. Some files would be Melting to indicate that they don't freeze. 
- [ ] Some files should be frozen automatically. (.exe) but you can also should not to freeze individual exe files. You should not have to unfreeze them every time you update the file.
- [ ] Button to pause all downloads. Resuming should work too.
- [ ] A way to view the files that are colliding. If `file.txt` collides with another version then the program would download the other version but name it `file(remote).txt`. The remote file could be temporarily stored in `.unisync` folder. Note that the program is allowed to delete or change the remote file at any time so do not change it.

# Completed
- [x] Update engone with the code from BetterThanBatch/APlayersDelight.
- [x] last modified time for files is not working. As a result, files are sent back and forth.
