#ifndef NETWORK_UTILS__H
#define NETWORK_UTILS__H


/***** Network stuff to communicate with the cRIO *****/

#define MY_IP "127.0.0.1" // find this out programatically?
#define SOCK_CHANGE_PROFILE_PORT 1181 // read this from a config file, or hard-code it?
#define SOCK_CHANGE_DATA_REQUEST_PORT 1182 // read this from a config file, or hard-code it?

void *runChangeProfileServer(void *placeHolder);
void *runDataRequestServer(void *placeHolder);


#endif
