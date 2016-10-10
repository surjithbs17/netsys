Author : Surjith Bhagavath Singh
File details	: Read me for udp (Programming Assignment 1 - Network Systems)

There are three main files for this programming assignment
	1.server.c
	2.client.c
	3.Makefile

server.c
	This file serves as the server for this assignment. It takes port number as its argument.
	eg - ./server 5011 
	This executes in a while loop until it recieves an exit command.

client.c
	This file is in cli/client.c . It takes Ip address and port number as its argument.
	eg - ./client 128.138.45.63 5011

	This accepts 4 types of commands (ls,get <filename>, put <filename> , exit).

	This code executes stop and wait protocol for reliability. It throws out error for unrecognised
	commands and files.

Makefile
	This compiles both the server and client code. Do the following  to compile a code.
	1.make clean
	2.make all
	3../server <portnumber>
	4.cd cli/
	5./client <ipaddress> <portnumber>
	6.Follow the instructions given in terminal /type the commands (ls,put,get,exit)