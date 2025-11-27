#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

//insert link at the first location
struct node* insertFirstU(struct node *head, int socket, char *username) {
    
   if(findUBySocket(head, socket) == NULL) {
           
       //create a link
       struct node *link = (struct node*) malloc(sizeof(struct node));

       link->socket = socket;
       strcpy(link->username,username);
       
       //point it to old first node
       link->next = head;

       //point first to new first node
       head = link;
 
   }
   else
       printf("Duplicate socket: %d\n", socket);
   return head;
}

//find a link with given user
struct node* findU(struct node *head, char* username) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
    while(strcmp(current->username, username) != 0) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if username found, return the current Link
   return current;
}

//find a node with given socket
struct node* findUBySocket(struct node *head, int socket) {
   struct node* current = head;

   if(head == NULL) {
      return NULL;
   }

   while(current->socket != socket) {
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }
   
   return current;
}

//delete a node with given socket
struct node* deleteU(struct node *head, int socket) {
   struct node* current = head;
   struct node* previous = NULL;

   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->socket != socket) {
      if(current->next == NULL) {
         return head; // not found
      } else {
         previous = current;
         current = current->next;
      }
   }

   //found a match, update the list
   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }
   
   free(current);
   return head;
}

//update username for a given socket
void updateUsername(struct node *head, int socket, char *new_username) {
   struct node* user = findUBySocket(head, socket);
   if(user != NULL) {
      strcpy(user->username, new_username);
   }
}

/////////////////// ROOMLIST //////////////////////////

//insert room at the first location
struct room_node* insertFirstR(struct room_node *head, char *room_name) {
   if(findR(head, room_name) == NULL) {
      struct room_node *link = (struct room_node*) malloc(sizeof(struct room_node));
      strcpy(link->room_name, room_name);
      link->users_in_room = NULL;
      link->next = head;
      head = link;
   }
   return head;
}

//find a room with given name
struct room_node* findR(struct room_node *head, char *room_name) {
   struct room_node* current = head;

   if(head == NULL) {
      return NULL;
   }

   while(strcmp(current->room_name, room_name) != 0) {
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }
   
   return current;
}

//add user to a room
void addUserToRoom(struct room_node *room, int socket, char *username) {
   if(room == NULL) return;
   
   // Check if user already in room
   struct node* existing = findUBySocket(room->users_in_room, socket);
   if(existing == NULL) {
      room->users_in_room = insertFirstU(room->users_in_room, socket, username);
   }
}

//remove user from a room
void removeUserFromRoom(struct room_node *room, int socket) {
   if(room == NULL) return;
   room->users_in_room = deleteU(room->users_in_room, socket);
}

//get all users in a room
struct node* getUsersInRoom(struct room_node *room) {
   if(room == NULL) return NULL;
   return room->users_in_room;
}

//delete a room
struct room_node* deleteR(struct room_node *head, char *room_name) {
   struct room_node* current = head;
   struct room_node* previous = NULL;

   if(head == NULL) {
      return NULL;
   }

   while(strcmp(current->room_name, room_name) != 0) {
      if(current->next == NULL) {
         return head; // not found
      } else {
         previous = current;
         current = current->next;
      }
   }

   // Free all users in room
   struct node* user = current->users_in_room;
   while(user != NULL) {
      struct node* next = user->next;
      free(user);
      user = next;
   }

   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }
   
   free(current);
   return head;
}

/////////////////// DM CONNECTIONS //////////////////////////

//add a direct connection between two users
struct dm_connection* addDM(struct dm_connection *head, int socket1, int socket2) {
   // Check if connection already exists
   if(isDMConnected(head, socket1, socket2)) {
      return head;
   }
   
   struct dm_connection *link = (struct dm_connection*) malloc(sizeof(struct dm_connection));
   link->socket1 = socket1;
   link->socket2 = socket2;
   link->next = head;
   head = link;
   return head;
}

//remove a direct connection between two users
struct dm_connection* removeDM(struct dm_connection *head, int socket1, int socket2) {
   struct dm_connection* current = head;
   struct dm_connection* previous = NULL;

   if(head == NULL) {
      return NULL;
   }

   while(!((current->socket1 == socket1 && current->socket2 == socket2) ||
           (current->socket1 == socket2 && current->socket2 == socket1))) {
      if(current->next == NULL) {
         return head; // not found
      } else {
         previous = current;
         current = current->next;
      }
   }

   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }
   
   free(current);
   return head;
}

//check if two users are directly connected
int isDMConnected(struct dm_connection *head, int socket1, int socket2) {
   struct dm_connection* current = head;

   while(current != NULL) {
      if((current->socket1 == socket1 && current->socket2 == socket2) ||
         (current->socket1 == socket2 && current->socket2 == socket1)) {
         return 1;
      }
      current = current->next;
   }
   return 0;
}

//get all users directly connected to a given socket
struct node* getDMUsers(struct dm_connection *head, struct node *user_list, int socket) {
   struct node* dm_users = NULL;
   struct dm_connection* current = head;

   while(current != NULL) {
      int other_socket = -1;
      if(current->socket1 == socket) {
         other_socket = current->socket2;
      } else if(current->socket2 == socket) {
         other_socket = current->socket1;
      }
      
      if(other_socket != -1) {
         struct node* user = findUBySocket(user_list, other_socket);
         if(user != NULL) {
            // Add to dm_users list (just for checking, we'll use the actual user_list)
            // We'll return a list of sockets to check
            struct node* link = (struct node*) malloc(sizeof(struct node));
            link->socket = other_socket;
            strcpy(link->username, user->username);
            link->next = dm_users;
            dm_users = link;
         }
      }
      current = current->next;
   }
   
   return dm_users;
}

//remove all DMs involving a given socket
struct dm_connection* removeAllDMs(struct dm_connection *head, int socket) {
   struct dm_connection* current = head;
   struct dm_connection* previous = NULL;

   while(current != NULL) {
      if(current->socket1 == socket || current->socket2 == socket) {
         if(current == head) {
            head = head->next;
            free(current);
            current = head;
         } else {
            previous->next = current->next;
            free(current);
            current = previous->next;
         }
      } else {
         previous = current;
         current = current->next;
      }
   }
   
   return head;
}
