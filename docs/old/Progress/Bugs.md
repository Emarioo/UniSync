Issues that you have found in practise are put here.

- [ ] Closing two apps freezes the program. Main thread and file monitor thread gets stuck.
- [ ] Deleting a file does not always cause FileMonitor to update.
- [ ] MAX_PATH is different for some computers. If the app generated a save file on one computer and the program is compiled on another and uses the same save file a crash will occur. Fixed by having a constant instead of max_path or checking if save file is safe. 