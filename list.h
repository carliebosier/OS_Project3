#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
   char username[30];
   int socket;
   struct node *next;
};

// Room structure: contains room name and list of users in the room
struct room_node {
   char room_name[50];
   struct node *users_in_room;  // linked list of users in this room
   struct room_node *next;
};

// Direct connection structure: tracks DM connections between users
struct dm_connection {
   int socket1;  // socket of first user
   int socket2;  // socket of second user
   struct dm_connection *next;
};

/////////////////// USERLIST //////////////////////////

//insert node at the first location
struct node* insertFirstU(struct node *head, int socket, char *username);

//find a node with given username
struct node* findU(struct node *head, char* username);

//find a node with given socket
struct node* findUBySocket(struct node *head, int socket);

//delete a node with given socket
struct node* deleteU(struct node *head, int socket);

//update username for a given socket
void updateUsername(struct node *head, int socket, char *new_username);

/////////////////// ROOMLIST //////////////////////////

//insert room at the first location
struct room_node* insertFirstR(struct room_node *head, char *room_name);

//find a room with given name
struct room_node* findR(struct room_node *head, char *room_name);

//add user to a room
void addUserToRoom(struct room_node *room, int socket, char *username);

//remove user from a room
void removeUserFromRoom(struct room_node *room, int socket);

//get all users in a room
struct node* getUsersInRoom(struct room_node *room);

//delete a room
struct room_node* deleteR(struct room_node *head, char *room_name);

/////////////////// DM CONNECTIONS //////////////////////////

//add a direct connection between two users
struct dm_connection* addDM(struct dm_connection *head, int socket1, int socket2);

//remove a direct connection between two users
struct dm_connection* removeDM(struct dm_connection *head, int socket1, int socket2);

//check if two users are directly connected
int isDMConnected(struct dm_connection *head, int socket1, int socket2);

//get all users directly connected to a given socket
struct node* getDMUsers(struct dm_connection *head, struct node *user_list, int socket);

//remove all DMs involving a given socket
struct dm_connection* removeAllDMs(struct dm_connection *head, int socket);
