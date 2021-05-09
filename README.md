# Memory Scanner
A memory scanner for Wesnoth that allows a user to search, filter, and edit memory inside the process. This code can be adapted to any target and is intended to show how tools like CheatEngine work.

The scanner has three main operations:
- search
- filter
- write

The search operation will scan all memory from 0x00000000 to 0x7FFFFFFF and use ReadProcessMemory to determine if the address holds a certain value. Due to ReadProcessMemory failing if a process doesn't have access to an address, the memory is scanned in blocks. Any values that match are saved to res.txt

The filter operation iterates over all addresses in res.txt to determine if they match the provided value. If so, they are saved to res_fil.txt. At the end, res_fil.txt is copied over to res.txt.

The write operation uses WriteProcessMemory to write a passed value to all address in res.txt

CreateToolhelp32Snapshot is used to find the Wesnoth process and OpenProcess is used to retrieve a handle. 

The full explanation for how this code works is available at: https://gamehacking.academy/lesson/35
