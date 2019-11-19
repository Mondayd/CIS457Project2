# CIS457Project2
Project 2 P2P Connection
Created By: Devin Monday

INSTRUCTIONS:

Running the central server and ftp server requires a port# as an argument.

Client has several commands:

CONNECT: Connect to a server using the hostname and port number. If running this on the same computer, host name is localhost.
  Connecting to the central server will ask you for your username, portnumber, and a file with all of your file descriptions.
  Connecting to the ftp_server will connect like normal. 
  
List: (Central_Server): Lists all of the user information curently connected.
Search: (Central Server): Asks for a keyword and brings up the file and file description for a match along with the username.

Retrieve:(Ftp Server): Retrieves a file from the ftp server that you are connected do.

Quit: Exits the program and disconnects from the Central Server or FTP Server. When disconnected from the Central Server, the username 
    and of the files on the central server will be removed.
