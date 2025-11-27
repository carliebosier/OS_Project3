#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room_node *room_list;
extern struct dm_connection *dm_list;

extern char *server_MOTD;


/*
 *Main thread for each client.  Receives all messages
 *and passes the data off to the correct function.  Receives
 *a pointer to the file descriptor for the socket the thread
 *should listen on
 */

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // socket
  
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  //data buffer of 2K  
   char tmpbuf[MAXBUFF];  //data temp buffer of 1K  
   char cmd[MAXBUFF], username[20];
   char *arguments[80];
    
   send(client  , server_MOTD , strlen(server_MOTD) , 0 ); // Send Welcome Message of the Day.

   // Creating the guest user name
   writer_lock();
   sprintf(username,"guest%d", client);
   head = insertFirstU(head, client , username);
   
   // Add the GUEST to the DEFAULT ROOM (i.e. Lobby)
   struct room_node *lobby = findR(room_list, DEFAULT_ROOM);
   if(lobby != NULL) {
      addUserToRoom(lobby, client, username);
   }
   writer_unlock();

   while (1) {
       
      if ((received = read(client , buffer, MAXBUFF)) != 0) {
      
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);
         
            /////////////////////////////////////////////////////
            // we got some data from a client

            // 1. Tokenize the input in buf (split it on whitespace)

            // get the first token 

             arguments[0] = strtok(cmd, delimiters);

            // walk through other tokens 

             i = 0;
             while( arguments[i] != NULL ) {
                arguments[++i] = strtok(NULL, delimiters); 
                strcpy(arguments[i-1], trimwhitespace(arguments[i-1]));
             } 

             // Arg[0] = command
             // Arg[1] = user or room
             
             /////////////////////////////////////////////////////
             // 2. Execute command: TODO


            if(strcmp(arguments[0], "create") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Room name required.\nchat>");
                  send(client , buffer , strlen(buffer) , 0 );
               } else {
                  printf("create room: %s\n", arguments[1]); 
                  
                  writer_lock();
                  room_list = insertFirstR(room_list, arguments[1]);
                  writer_unlock();
                  
                  sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
                  send(client , buffer , strlen(buffer) , 0 );
               }
            }
            else if (strcmp(arguments[0], "join") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Room name required.\nchat>");
                  send(client , buffer , strlen(buffer) , 0 );
               } else {
                  printf("join room: %s\n", arguments[1]);  
                  
                  writer_lock();
                  struct room_node *room = findR(room_list, arguments[1]);
                  if(room != NULL) {
                     struct node *user = findUBySocket(head, client);
                     if(user != NULL) {
                        addUserToRoom(room, client, user->username);
                        sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                     } else {
                        sprintf(buffer, "Error: User not found.\nchat>");
                     }
                  } else {
                     sprintf(buffer, "Error: Room '%s' not found.\nchat>", arguments[1]);
                  }
                  writer_unlock();
                  
                  send(client , buffer , strlen(buffer) , 0 );
               }
            }
            else if (strcmp(arguments[0], "leave") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Room name required.\nchat>");
                  send(client , buffer , strlen(buffer) , 0 );
               } else {
                  printf("leave room: %s\n", arguments[1]); 
                  
                  writer_lock();
                  struct room_node *room = findR(room_list, arguments[1]);
                  if(room != NULL) {
                     if(strcmp(room->room_name, DEFAULT_ROOM) == 0) {
                        sprintf(buffer, "Error: Cannot leave the Lobby.\nchat>");
                     } else {
                        removeUserFromRoom(room, client);
                        sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
                     }
                  } else {
                     sprintf(buffer, "Error: Room '%s' not found.\nchat>", arguments[1]);
                  }
                  writer_unlock();
                  
                  send(client , buffer , strlen(buffer) , 0 );
               }
            } 
            else if (strcmp(arguments[0], "connect") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Username required.\nchat>");
                  send(client , buffer , strlen(buffer) , 0 );
               } else {
                  printf("connect to user: %s \n", arguments[1]);
                  
                  writer_lock();
                  struct node *target_user = findU(head, arguments[1]);
                  struct node *current_user = findUBySocket(head, client);
                  
                  if(target_user == NULL) {
                     sprintf(buffer, "Error: User '%s' not found.\nchat>", arguments[1]);
                  } else if(current_user == NULL) {
                     sprintf(buffer, "Error: Current user not found.\nchat>");
                  } else if(target_user->socket == client) {
                     sprintf(buffer, "Error: Cannot connect to yourself.\nchat>");
                  } else {
                     dm_list = addDM(dm_list, client, target_user->socket);
                     sprintf(buffer, "Connected to user '%s'.\nchat>", arguments[1]);
                  }
                  writer_unlock();
                  
                  send(client , buffer , strlen(buffer) , 0 );
               }
            }
            else if (strcmp(arguments[0], "disconnect") == 0)
            {             
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Username required.\nchat>");
                  send(client , buffer , strlen(buffer) , 0 );
               } else {
                  printf("disconnect from user: %s\n", arguments[1]);
                  
                  writer_lock();
                  struct node *target_user = findU(head, arguments[1]);
                  
                  if(target_user == NULL) {
                     sprintf(buffer, "Error: User '%s' not found.\nchat>", arguments[1]);
                  } else {
                     dm_list = removeDM(dm_list, client, target_user->socket);
                     sprintf(buffer, "Disconnected from user '%s'.\nchat>", arguments[1]);
                  }
                  writer_unlock();
                  
                  send(client , buffer , strlen(buffer) , 0 );
               }
            }                  
            else if (strcmp(arguments[0], "rooms") == 0)
            {
                printf("List all the rooms\n");
              
                reader_lock();
                strcpy(buffer, "Rooms:\n");
                struct room_node *room = room_list;
                int count = 0;
                while(room != NULL) {
                   char room_info[100];
                   sprintf(room_info, "  - %s\n", room->room_name);
                   strcat(buffer, room_info);
                   room = room->next;
                   count++;
                }
                if(count == 0) {
                   strcat(buffer, "  (no rooms)\n");
                }
                reader_unlock();
              
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );
            }   
            else if (strcmp(arguments[0], "users") == 0)
            {
                printf("List all the users\n");
              
                reader_lock();
                strcpy(buffer, "Users:\n");
                struct node *user = head;
                int count = 0;
                while(user != NULL) {
                   char user_info[100];
                   sprintf(user_info, "  - %s\n", user->username);
                   strcat(buffer, user_info);
                   user = user->next;
                   count++;
                }
                if(count == 0) {
                   strcat(buffer, "  (no users)\n");
                }
                reader_unlock();
                
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );
            }                           
            else if (strcmp(arguments[0], "login") == 0)
            {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Error: Username required.\nchat>");
                  send(client , buffer , strlen(buffer) , 0 );
               } else {
                  writer_lock();
                  struct node *user = findUBySocket(head, client);
                  if(user != NULL) {
                     char old_username[30];
                     strcpy(old_username, user->username);
                     
                     // Check if username already exists
                     struct node *existing = findU(head, arguments[1]);
                     if(existing != NULL && existing->socket != client) {
                        sprintf(buffer, "Error: Username '%s' already taken.\nchat>", arguments[1]);
                     } else {
                        // Update username in user list
                        updateUsername(head, client, arguments[1]);
                        
                        // Update username in all rooms
                        struct room_node *room = room_list;
                        while(room != NULL) {
                           struct node *room_user = findUBySocket(room->users_in_room, client);
                           if(room_user != NULL) {
                              strcpy(room_user->username, arguments[1]);
                           }
                           room = room->next;
                        }
                        
                        sprintf(buffer, "Logged in as '%s'.\nchat>", arguments[1]);
                     }
                  } else {
                     sprintf(buffer, "Error: User not found.\nchat>");
                  }
                  writer_unlock();
                  
                  send(client , buffer , strlen(buffer) , 0 );
               }
            } 
            else if (strcmp(arguments[0], "help") == 0 )
            {
                sprintf(buffer, "login <username> - \"login with username\" \ncreate <room> - \"create a room\" \njoin <room> - \"join a room\" \nleave <room> - \"leave a room\" \nusers - \"list all users\" \nrooms -  \"list all rooms\" \nconnect <user> - \"connect to user\" \nexit - \"exit chat\" \n");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client 
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0)
            {
               writer_lock();
               
               // Remove from all rooms
               struct room_node *room = room_list;
               while(room != NULL) {
                  removeUserFromRoom(room, client);
                  room = room->next;
               }
               
               // Remove all DM connections
               dm_list = removeAllDMs(dm_list, client);
               
               // Remove from user list
               head = deleteU(head, client);
               
               writer_unlock();
               
               close(client);
               return NULL;
            }                         
            else { 
                 /////////////////////////////////////////////////////////////
                 // 3. sending a message
           
                 // send a message in the following format followed by the promt chat> to the appropriate receipients based on rooms, DMs
                 // ::[userfrom]> <message>
              
                 reader_lock();
                 
                 // Get current user's username
                 struct node *sender = findUBySocket(head, client);
                 if(sender == NULL) {
                    reader_unlock();
                    continue;
                 }
                 
                 // Safely format message with snprintf to prevent overflow
                 // Reserve space for prefix "\n::username> " and suffix "\nchat>"
                 // Username is max 30 chars, so we need: 3 + 30 + 2 + message + 7 = message + 42
                 // To fit in MAXBUFF, message can be at most MAXBUFF - 42
                 int prefix_suffix_len = strlen(sender->username) + 12; // "\n::" + username + "> " + "\nchat>"
                 int max_msg_content = MAXBUFF - prefix_suffix_len - 1; // -1 for null terminator
                 if(max_msg_content < 0) max_msg_content = 0;
                 
                 // Truncate message content if necessary
                 if((int)strlen(sbuffer) > max_msg_content) {
                    sbuffer[max_msg_content] = '\0';
                 }
                 
                 snprintf(tmpbuf, MAXBUFF, "\n::%s> %s\nchat>", sender->username, sbuffer);
                 strcpy(sbuffer, tmpbuf);
                 
                 // Collect all recipients (users in same rooms + DM connections)
                 // Use a simple approach: track sockets to send to
                 int recipients[100];
                 int recipient_count = 0;
                 
                 // Find all users in same rooms as sender
                 struct room_node *room = room_list;
                 while(room != NULL) {
                    struct node *room_user = findUBySocket(room->users_in_room, client);
                    if(room_user != NULL) {
                       // Sender is in this room, add all other users in this room
                       struct node *user_in_room = room->users_in_room;
                       while(user_in_room != NULL) {
                          if(user_in_room->socket != client) {
                             // Check if already in recipients list
                             int found = 0;
                             for(int i = 0; i < recipient_count; i++) {
                                if(recipients[i] == user_in_room->socket) {
                                   found = 1;
                                   break;
                                }
                             }
                             if(!found && recipient_count < 100) {
                                recipients[recipient_count++] = user_in_room->socket;
                             }
                          }
                          user_in_room = user_in_room->next;
                       }
                    }
                    room = room->next;
                 }
                 
                 // Find all users directly connected (DM) to sender
                 struct dm_connection *dm = dm_list;
                 while(dm != NULL) {
                    int other_socket = -1;
                    if(dm->socket1 == client) {
                       other_socket = dm->socket2;
                    } else if(dm->socket2 == client) {
                       other_socket = dm->socket1;
                    }
                    
                    if(other_socket != -1) {
                       // Check if already in recipients list
                       int found = 0;
                       for(int i = 0; i < recipient_count; i++) {
                          if(recipients[i] == other_socket) {
                             found = 1;
                             break;
                          }
                       }
                       if(!found && recipient_count < 100) {
                          recipients[recipient_count++] = other_socket;
                       }
                    }
                    dm = dm->next;
                 }
                 
                 reader_unlock();
                 
                 // Send message to all recipients
                 for(int i = 0; i < recipient_count; i++) {
                    send(recipients[i], sbuffer, strlen(sbuffer), 0);
                 }
          
            }
 
         memset(buffer, 0, sizeof(1024));
      }
   }
   return NULL;
}