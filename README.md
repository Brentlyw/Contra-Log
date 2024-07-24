# Contra-Log
A c++ program with a multifaceted protection mechanism to stop common keyloggers.

# Features
- No lag to the user!
- Hook persistence via order of operations.
- Obfuscation of keys based on a rotating & randomized keymap.
- Sends randomized keystrokes to two different levels in the system at random intervals, confusing and bloating any keylogger logs.

# Flow
1. User presses key
2. Low-level keyboard hook captures the key
3. Check if the key is an action key (Shift, etc.)
4. Obfuscate the key according to current keymap rotation
5. Check if the key is marked as randomized input routine
6. Send obfuscated keystroke (Original keystroke ignored)
8. Output normally to the user

*This program does not guarantee protection against all keyloggers.*
