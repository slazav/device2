## Simple pipe protocol (SPP)
---

I introduced this protocol at about 2016, for human-readable text
communications with command-line programs. I'm using it in many tools,
including `device_c` program for device2 package, `graphene` database,
etc. I'm using it for secure remote connections to programs and devices
via `ssh` links. Probably, it is not used anywhere else.

Programs receive single-line text messages and reply with a multi-line
answer followed by either `#OK` of `#Error: <message>` line.
This works great for simple text communication, for binary data or
multi-line commands it is still messy (but I'm using it too).

#### version 001

There are two programs, "server" and "client" connected with unix pipes.
All data are read and written line by line in a human-readable form if
possible.

When connection is opened, server writes a line with the special symbol
('#' in this example, but it can be any symbol), protocol name and
version: `#SPP<version>`. Then it can write some text for the user which can be
ignored. Then it either writes `#Error: <message>` and exits or writes
`#OK` and start listening for user requests. Simbol '#' here is a special
symbol which was selected in the beginning of the conversation.

Request is one line of text.

Answer of the server is a few lines of text, followed by '#Error:
<message>' or '#OK' line. Lines of the answer text starting with the
'#' symbol should be protected by doubling the symbol.

It is recommended to implement *idn? command which returns ID of the
device.

#### version 002

If any fatal error appear server can print a line `#Fatal: <message>`
and exit.

* TODO: requests with several lines
* TODO: timeouts, safe closing of the channel...
* TODO: raw data transfer

#### Conversation example
```
$ ./spp_server_test.tcl
#SPP002
Welcome, dear user!
Please, type "help" if you do not know what to do.
#OK
h
#Error: Unknown command: h
help
spp_server_test -- an example of the command-line interface program.
Commands:
  write <k> <v> -- write value v into a memory slot k=0..9
  read <k>      -- read value from a memory slot k
  list -- list all commands
  help -- show this message

#OK
write 1 abc
#OK
read
#Error: wrong # args: should be "testSrv0 read k"
read 1
abc
#OK
```
