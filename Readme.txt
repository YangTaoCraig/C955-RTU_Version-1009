C955 RTU

Version 1005
070614
1.	Remove the control flow function of serial link SWC interface.
2.	Enable RTU reading the Serial link comms options from configuration file
3.	Fix the RTU LAN1 and LAN2 intercommunication alarm issue:  In the RMM module LinkMaster and LinkSlave function remove the unnecessary code.
4.	Update the Server command  logging process. And after RTU send SWC reply to server , add buffer time 300ms before RTU close the server socket.

150714    
1. For the keepalive message, remove the 300ms buffer, and change the close function from private to public.
