# RemoteCMD


A backdoor which is similar to Meterpreter.

This is a very useful backdoor, it is actually a reverse shell. 
Its capable of holding more then one clinets at once, and connecting to them.
This only accesses the Windows Command Prompt of the target, nothing more, but still, its powerful.
For the networking part I only used winsock, and I only tested it on localhost.

Features/Commands: <br />
zombies - view your zombies connected to the server <br />
help - shows help <br />
useful - useful cmd commands <br />
select + (the number of the client shown in the zombies menu) - Selects the client and connects to its cmd. <br />
leave - leaves the session <br />
kill - deletes the client <br />


