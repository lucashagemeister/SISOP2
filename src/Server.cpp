#include "../include/Server.hpp"

using namespace std;
vector<int> possiblePorts { PORT, PORT1, PORT2, PORT3 };


Server::Server(int port)
{
    this->electionStarted = false;
    this->gotAnsweredInElection = false;

    this->port = port;
    
    // Infer neihbor server nodes
    std::vector<int> possibleServerPeerPorts;
    for (int i : possiblePorts){
        if (i != this->port)
            possibleServerPeerPorts.push_back(i);
    } 
    this->possibleServerPeerPorts = possibleServerPeerPorts;

    this->notification_id_counter = 0;
    mutex_session = PTHREAD_MUTEX_INITIALIZER;
    follow_mutex = PTHREAD_MUTEX_INITIALIZER;
    follower_count_mutex = PTHREAD_MUTEX_INITIALIZER;
    packetsToBeSentMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond_notification_empty, NULL);
    pthread_cond_init(&cond_notification_full, NULL);
    pthread_mutex_init(&mutex_notification_sender, NULL);
    pthread_mutex_init(&packetsToBeSentMutex, NULL);
}

Server::Server(host_address address)
{
    this->electionStarted - false;
    this->gotAnsweredInElection = false;
    this->notification_id_counter = 0;
	this->ip = address.ipv4;
	this->port = address.port;

    mutex_session = PTHREAD_MUTEX_INITIALIZER;
    follow_mutex = PTHREAD_MUTEX_INITIALIZER;
    follower_count_mutex = PTHREAD_MUTEX_INITIALIZER;
    packetsToBeSentMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond_notification_empty, NULL);
    pthread_cond_init(&cond_notification_full, NULL);
    pthread_mutex_init(&mutex_notification_sender, NULL);
    pthread_mutex_init(&packetsToBeSentMutex, NULL);

}


void Server::updatePossibleServerPeerPorts(){
    std::vector<int> possibleServerPeerPorts;
    for (int i : possiblePorts){
        if (i != this->port)
            possibleServerPeerPorts.push_back(i);
    } 
    this->possibleServerPeerPorts = possibleServerPeerPorts;
}


void Server::setAsPrimaryServer(){
    this->backupMode = false;
    this->portPrimarySever = this->port;

    // ####################################################################################
    // EFETUAR OUTROS "TRÂMITES" QUE PRECISAR PRA SETAR PASSAR ESSE SERVER COMO O PRIMÁRIO
    // ####################################################################################
}


// se não tiver um host_address, eu posso fazer e devolver um session_id
bool Server::try_to_start_session(string user, host_address address)
{
    pthread_mutex_lock(&mutex_session);

    if(!user_exists(user))
    {
        sem_t num_sessions;
        sem_init(&num_sessions, 0, 2);
        user_sessions_semaphore.insert({user, num_sessions}); // user is created with 2 sessions available
        sessions.insert({user, list<host_address>()});
        followers.insert(pair<string, list<string>>(user, list<string>()));
        users_unread_notifications.insert({user, list<uint32_t>()});
    } 
    
    int session_started = sem_trywait(&(user_sessions_semaphore[user])); // try to consume a session resource
    if(session_started == 0) // 0 if session started, -1 if not
    { 
        sessions[user].push_back(address);
        active_users_pending_notifications.insert({address, priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>>()});
    }
    pthread_mutex_unlock(&mutex_session); // Fim SC
    return session_started == 0; 
}

bool Server::user_exists(string user)
{
    return user_sessions_semaphore.find(user) != user_sessions_semaphore.end();
}


// call this function when new notification is created
void Server::create_notification(string user, string body, time_t timestamp)
{
    cout << "\nNew notification!\n";
    pthread_mutex_lock(&follower_count_mutex);
    if (followers[user].size() > 0)
    {
        uint16_t pending_users{0};
        for (auto follower : followers[user])
        {                    
            pthread_mutex_lock(&mutex_notification_sender);
            users_unread_notifications[follower].push_back(notification_id_counter);
            pthread_mutex_unlock(&mutex_notification_sender);

            pending_users++;
        }

        notification notif(notification_id_counter, user, timestamp, body, body.length(), pending_users);
        active_notifications.push_back(notif);
        assign_notification_to_active_sessions(notification_id_counter, followers[user]);
        notification_id_counter += 1;
    }
    pthread_mutex_unlock(&follower_count_mutex);
}

// call this function after new notification is created
void Server::assign_notification_to_active_sessions(uint32_t notification_id, list<string> followers) 
{
    cout << "\nAssigning new notification to active sessions...\n";
    
    pthread_mutex_lock(&mutex_notification_sender);
    for (auto user : followers)
    {
        if(user_is_active(user)) 
        {
            for(auto address : sessions[user]) 
            {
                //cout << "assigning " << address.ipv4 <<":"<< address.port << " notification " << notification_id <<"\n\n";
                active_users_pending_notifications[address].push(notification_id);
            }
            // when all sessions from same user have notification on its entry, remove @ from list
            list<uint32_t>::iterator it = find(users_unread_notifications[user].begin(), users_unread_notifications[user].end(), notification_id);
            users_unread_notifications[user].erase(it);

            // signal as many consumers to send to clients as pending users times possible sessions
            for(int i = 0; i < followers.size()*2; i++)
            {
                pthread_cond_signal(&cond_notification_full);

            }
        }
    }
    pthread_mutex_unlock(&mutex_notification_sender);

}

bool Server::user_is_active(string user) 
{
    return sessions.find(user) != sessions.end() && !(sessions[user].empty());
}


// call this function when new session is started (after try_to_start_session()) to wake notification producer to client
void Server::retrieve_notifications_from_offline_period(string user, host_address addr) 
{
    cout << "\nGetting notifications from offline period to active sessions...\n";
    //print_users_unread_notifications();
    pthread_mutex_lock(&mutex_notification_sender);
    
    for(auto notification_id : users_unread_notifications[user]) 
    {
        active_users_pending_notifications[addr].push(notification_id);
    }
    
    (users_unread_notifications[user]).clear();

    // signal consumer
    pthread_cond_signal(&cond_notification_full);
    pthread_mutex_unlock(&mutex_notification_sender);
}


// call this function on consumer thread that will feed the user with its notifications
void Server::read_notifications(host_address addr, vector<notification>* notifications) 
{
    pthread_mutex_lock(&mutex_notification_sender);
    
    cout << "Reading notifications...\n";
    while (active_users_pending_notifications[addr].empty()) { 
        // sleep while user doesn't have notifications to read
        cout << "No notifications for address " << addr.ipv4 <<":"<< addr.port << ". Sleeping...\n";
        pthread_cond_wait(&cond_notification_full, &mutex_notification_sender); 
    }
    cout << "Assembling notifications...\n";
    while(!active_users_pending_notifications[addr].empty()) 
    {
        uint32_t notification_id = (active_users_pending_notifications[addr]).top();
        for(auto notif : active_notifications)
        {
            if(notif.id == notification_id)
            {
                notifications->push_back(notif);
                break;
            }
        }
        
        (active_users_pending_notifications[addr]).pop();
    }

    // signal producer
    pthread_cond_signal(&cond_notification_empty);
    pthread_cond_signal(&cond_notification_full); // waking someone again can improve client consuming flow
    pthread_mutex_unlock(&mutex_notification_sender);

}


// call this function when client presses ctrl+c or ctrl+d
void Server::close_session(string user, host_address address) 
{
    pthread_mutex_lock(&mutex_session);
    
    list<host_address>::iterator it = find(sessions[user].begin(), sessions[user].end(), address);
    if(it != sessions[user].end()) // remove address from sessions map and < (ip, port), notification to send > 
    {
        sessions[user].erase(it);
        active_users_pending_notifications.erase(address);

        // signal semaphore
        sem_post(&(user_sessions_semaphore[user]));
    }
    pthread_mutex_unlock(&mutex_session);
}


void Server::follow_user(string user, string user_to_follow)
{
    pthread_mutex_lock(&follower_count_mutex);

    if (find(followers[user_to_follow].begin(), followers[user_to_follow].end(), user) == followers[user_to_follow].end())
    {
        followers[user_to_follow].push_back(user);
    }

    pthread_mutex_unlock(&follower_count_mutex);
}




void Server::print_users_unread_notifications() 
{
    cout << "\nUsers unread notifications: \n";

    for(auto it = users_unread_notifications.begin(); it != users_unread_notifications.end(); it++)
    {
        cout << it->first << ": [";
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            cout << *itl << ", ";
        }
        cout << "]\n";
    }
}


void Server::print_active_users_unread_notifications() 
{
    cout << "Active users notifications to receive: \n";

    for(auto it = active_users_pending_notifications.begin(); it != active_users_pending_notifications.end(); it++)
    {
        cout << it->first.ipv4 << ":" << it->first.port << ": [";
        cout << it->second.size() << "]\n";
    }
}

void Server::print_sessions() 
{
    cout << "\nSessions: " << sessions.size() << "\n";

    for(auto it = sessions.begin(); it != sessions.end(); it++)
    {
        cout << it->first << ": [";
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            cout << (*itl).ipv4 << ":" << (*itl).port << ", ";
        }
        cout << "]\n";
    }
}

void Server::print_active_notifications() 
{
    cout << "\nNotifications: " << active_notifications.size() << "\n";

    for(auto it = active_notifications.begin(); it != active_notifications.end(); it++)
    {
        cout << it->id << "\n";
        cout << it->author << "\n";
        cout << it->body << "\n";
        cout << "\n";
    }
}

void Server::print_followers() 
{
    cout << "\nFollowers: " << followers.size() << "\n";

    for(auto it = followers.begin(); it != followers.end(); it++)
    {
        cout << it->first << ": [";
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            cout << *itl << ", ";
        }
        cout << "]\n";
    }
}


void *Server::groupCommunicationHandler(void *handlerArgs){

    pthread_t readCommandsT;
    pthread_t sendNotificationsT;
    pthread_t electionTimeoutT;

    pthread_create(&readCommandsT, NULL, Server::groupReadMessagesHandler, handlerArgs);
    pthread_create(&sendNotificationsT, NULL, Server::groupSendMessagesHandler, handlerArgs);
    pthread_create(&electionTimeoutT, NULL, Server::electionTimeoutHandler, handlerArgs);


    pthread_join(readCommandsT, NULL);
    pthread_join(sendNotificationsT, NULL);
    pthread_join(electionTimeoutT, NULL);

    return NULL;
}


void *Server::electionTimeoutHandler(void *handlerArgs){

    // Unroll arguments
    struct group_communiction_handler_args *args = (struct group_communiction_handler_args *)handlerArgs;
    Server* server = args->server;
    
    while(true){

        if(server->electionStarted){
            cout << "yupi an election started! \n\n";
            sleep(2);
            if(!server->gotAnsweredInElection){
                pthread_mutex_lock(&server->packetsToBeSentMutex);
                server->packetsToBeSent.push_back(Packet(COORDINATOR, ""));
                server->setAsPrimaryServer();
                server->electionStarted = false;
                server->gotAnsweredInElection = false;
                pthread_mutex_unlock(&server->packetsToBeSentMutex);
            }
        }
    }
}


void *Server::groupReadMessagesHandler(void *handlerArgs){

    // Unroll arguments
    struct group_communiction_handler_args *args = (struct group_communiction_handler_args *)handlerArgs;
    Server* server = args->server;
    int peerPort = args->peerPort;
    Socket* connectedSocket = args->connectedSocket;

    string primaryServerPort;
    while(1){
        Packet* receivedPacket = connectedSocket->readPacket();
        
        if (receivedPacket == NULL){ 
            if (peerPort == server->portPrimarySever){
                pthread_mutex_lock(&server->packetsToBeSentMutex);
                cout << "Lost connection with primary server, initializing election... \n";
                server->packetsToBeSent.push_back(Packet(ELECTION, ""));
                pthread_mutex_unlock(&server->packetsToBeSentMutex);
            }
            return NULL;
        }

        switch(receivedPacket->getType()){

            case ASK_PRIMARY:
                primaryServerPort = std::to_string(server->portPrimarySever);
                connectedSocket->sendPacket(Packet(PRIMARY_SERVER_PORT, primaryServerPort.c_str()));
                break;
            
            case PRIMARY_SERVER_PORT:
                cout << "li primary server: " << atoi(receivedPacket->getPayload()) << "\n";
                if (receivedPacket->getType() == PRIMARY_SERVER_PORT)
                    server->portPrimarySever = atoi(receivedPacket->getPayload());
                break;
            
            case ELECTION:
                cout << "recebi coiso que alguem iniciou eleição..\n";
                connectedSocket->sendPacket(Packet(ANSWER, ""));
                break;

            case ANSWER:
                server->gotAnsweredInElection = true;
                break;

            case COORDINATOR:
                cout << "esse carinha se elegeu: " << peerPort << "\n";
                server->portPrimarySever = peerPort;



            // ########################################################################
            // Implement behavior for different kind of server-server messages arriving
            // ########################################################################

            default:
                break;
        }
    }    

}

void *Server::groupSendMessagesHandler(void *handlerArgs){

    // Unroll arguments
    struct group_communiction_handler_args *args = (struct group_communiction_handler_args *)handlerArgs;
    Server* server = args->server;
    int peerPort = args->peerPort;
    Socket* connectedSocket = args->connectedSocket;
    bool isAcceptingConnection = args->isAcceptingConnection;
    int connectionStatus;


    if (!isAcceptingConnection){    // If the handler is being initialized by a server instance that have just started to execute

        connectionStatus = connectedSocket->sendPacket(Packet(SERVER_PEER_CONNECTING, ""));
        if (connectionStatus < 0)
            return NULL;
        
        connectionStatus = connectedSocket->sendPacket(Packet(SERVER_PEER_CONNECTING, std::to_string(server->port).c_str()));
        if (connectionStatus < 0)
            return NULL;
            
        if (server->backupMode){    // Asks peer who's the primary server
            int primaryServerPort;

            Packet askForPrimary = Packet(ASK_PRIMARY, "");
            connectionStatus = connectedSocket->sendPacket(askForPrimary);
            if (connectionStatus < 0)
                return NULL;
        }

        if (peerPort == server->portPrimarySever){
            // ############################################
            // Asks primary server for current server state
            // ############################################
        }
    } else {
        
    }


    vector<Packet>::iterator it;
    int i = 0;
    // Keeps consuming packetsToBeSent elements until connection closes
    while(true){
        pthread_mutex_lock(&server->packetsToBeSentMutex);

        it = server->packetsToBeSent.begin();
        while(it != server->packetsToBeSent.end()) {
            cout << "iterating through server packets to be sent \n";
            if (it->getType() == ELECTION){
                cout << "tem um packet election na lista!!\n";
                cout << "this port: " << server->port << " | peer port: " << peerPort << " \n";
                server->electionStarted = true;
                if (server->port < peerPort)
                    connectionStatus = connectedSocket->sendPacket(*it);

            } else {
                connectionStatus = connectedSocket->sendPacket(*it);
            }

            it = server->packetsToBeSent.erase(it);
        }
        pthread_mutex_unlock(&server->packetsToBeSentMutex);

        if (connectionStatus < 0)
            return NULL;
        
    }
}




ServerSocket::ServerSocket() : Socket(){
    
    this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_port = htons(PORT);
	this->serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(this->serv_addr.sin_zero), 8);

}

void ServerSocket::connectNewClientOrServer(pthread_t *threadID, Server* server){

    int *newsockfd = (int *) calloc(1, sizeof(int));
    Socket *newConnectionSocket = (Socket *) calloc(1, sizeof(Socket));
	socklen_t clilen;
	struct sockaddr_in cli_addr;
    host_address client_address;
    string user;
    

    // Accepting connection to start communicating
    clilen = sizeof(struct sockaddr_in);
    if ((*newsockfd = accept(this->getSocketfd(), (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        std::cout << "ERROR on accepting client or server connection" << std::endl;
        return;
    }
    newConnectionSocket = new Socket(*newsockfd);

    std::cout << "New connection estabilished on socket: " << *newsockfd << "\n\n";



    Packet* connectionType = newConnectionSocket->readPacket();
        
    if (connectionType->getType() == SERVER_PEER_CONNECTING){

        Packet* listenPortInfo = newConnectionSocket->readPacket();

        group_communiction_handler_args *args = (group_communiction_handler_args *) calloc(1, sizeof(group_communiction_handler_args));
        args->peerPort = atoi(listenPortInfo->getPayload());
        args->connectedSocket = newConnectionSocket;
        args->server = server;
        args->isAcceptingConnection = true;

        pthread_create(threadID, NULL, Server::groupCommunicationHandler, (void *)args);
        return;
    }


    // ELSE (a client is connecting):
    if (!server->backupMode)
        newConnectionSocket->sendPacket(Packet(ALREADY_PRIMARY, ""));
    else {
        newConnectionSocket->sendPacket(Packet(MESSAGE_PKT, SERVER_ADDR));
        newConnectionSocket->sendPacket(Packet(MESSAGE_PKT, std::to_string(server->portPrimarySever).c_str()));
    }

    
    // Verify if there are free sessions available
    // read client username from socket in 'user' var
    Packet *userPacket = newConnectionSocket->readPacket();

    if (userPacket == NULL){
        std::cout << "Unable to read user information. Closing connection." << std::endl;
        return;     // destructor automatically closes the socket
    } else 
        user = userPacket->getPayload();
    
    if (userPacket->getType() == USER_INFO_PKT){
        client_address.ipv4 = inet_ntoa(cli_addr.sin_addr);
        client_address.port = ntohs(cli_addr.sin_port);
        // mutex
        // server->save_sessions_current_state();
        bool sessionAvailable = server->try_to_start_session(user, client_address);
        /* 
        error = false
        if sessionAvailable: 
            for backup in backups:
                send session created
                wait response 
                if session started: 
                    next
                else:
                    abort
                    sessions state = previous state
                    error = true
                    for confirmed_backup in confirmed_backups:
                        abort/uncommit changes
                    break
        }
        mutex */

    
        Packet sessionResultPkt;
        if (!sessionAvailable){
            sessionResultPkt = Packet(SESSION_OPEN_FAILED, "Unable to connect to server: no sessions available.");
            newConnectionSocket->sendPacket(sessionResultPkt);
            return; // destructor automatically closes the socket
        } else{
            sessionResultPkt = Packet(SESSION_OPEN_SUCCEDED, "Connection succeded! Session established.");
            newConnectionSocket->sendPacket(sessionResultPkt);
        }
    }
    
    // Build args
    communiction_handler_args *args = (communiction_handler_args *) calloc(1, sizeof(communiction_handler_args));
    args->client_address = client_address;
    args->connectedSocket = newConnectionSocket;
    args->user = user;
    args->server = server;

    pthread_create(threadID, NULL, Server::communicationHandler, (void *)args);
}


void ServerSocket::connectToGroupMembers(Server* server){
    struct sockaddr_in serv_addr;
    struct hostent *serverHost = gethostbyname(SERVER_ADDR);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr *)serverHost->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
    

    bool atLeastOneConnection = false;
    for(int i : server->possibleServerPeerPorts){
        serv_addr.sin_port = htons(i);
        if (this->connectToMember(serv_addr, server))
            atLeastOneConnection = true;
    }

    if (atLeastOneConnection)
        server->backupMode = true;

    else{
        server->backupMode = false;
        server->portPrimarySever = server->port;
    }
 }


bool ServerSocket::connectToMember(sockaddr_in serv_addr, Server* server){

    Socket *serverServerSocket = new Socket();
    pthread_t serverServerThread;

    cout << "Trying to connect to peer server in port " << ntohs(serv_addr.sin_port) << "... ";

    if (connect(serverServerSocket->getSocketfd(),(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        cout << "Failed, no server instance running here.\n";
        return false;
    }

    // else 
    cout << "Connected!" << "\n";
    
    group_communiction_handler_args *args = (group_communiction_handler_args *) calloc(1, sizeof(group_communiction_handler_args));
    args->peerPort = ntohs(serv_addr.sin_port);
    args->connectedSocket = serverServerSocket;
    args->server = server;
    args->isAcceptingConnection = false;

    pthread_create(&serverServerThread, NULL, Server::groupCommunicationHandler, (void *)args);
    return true;
}


void ServerSocket::bindAndListen(Server* server){
    
    bool bindSucceeded = false;

    for (int i : possiblePorts){
        
        this->serv_addr.sin_port = htons(i);

        if (bind(this->getSocketfd(), (struct sockaddr *) &this->serv_addr, sizeof(this->serv_addr)) < 0) {
            cout << "Port " << i << "  already taken\n";
        } else {
            server->port = i;
            server->updatePossibleServerPeerPorts();
            bindSucceeded = true;
            break;
        }
    }
	
    if (bindSucceeded) {
        listen(this->getSocketfd(), MAX_TCP_CONNECTIONS);
        std::cout << "Listening on port " << ntohs(this->serv_addr.sin_port) << "...\n\n";
    } else {
        std::cout << "ERROR on biding!\n";
        exit(1);
    }	
}


void *Server::communicationHandler(void *handlerArgs){

    pthread_t readCommandsT;
    pthread_t sendNotificationsT;

    pthread_create(&readCommandsT, NULL, Server::readCommandsHandler, handlerArgs);
    pthread_create(&sendNotificationsT, NULL, Server::sendNotificationsHandler, handlerArgs);

    pthread_join(readCommandsT, NULL);
    pthread_join(sendNotificationsT, NULL);

    return NULL;
}


void *Server::readCommandsHandler(void *handlerArgs){
	struct communiction_handler_args *args = (struct communiction_handler_args *)handlerArgs;

    string userToFollow;
    string response;

    while(1){
        Packet* receivedPacket = args->connectedSocket->readPacket();
        if (receivedPacket == NULL){  // connection closed
            args->server->close_session(args->user, args->client_address);
            return NULL;
        }
        cout << receivedPacket->getPayload() << "\n\n";

        switch(receivedPacket->getType()){

            case COMMAND_FOLLOW_PKT:
                userToFollow = receivedPacket->getPayload();
                response = "Followed "+userToFollow+"!";
                // transaction
                args->server->follow_user(args->user, userToFollow);
                // transaction
                args->connectedSocket->sendPacket(Packet(MESSAGE_PKT, response.c_str()));
                break;

            case COMMAND_SEND_PKT:
                // transaction
                args->server->create_notification(args->user, receivedPacket->getPayload(), receivedPacket->getTimestamp());
                // transaction
                args->connectedSocket->sendPacket(Packet(MESSAGE_PKT, "Notification sent!"));
                break;

            default:
                break;
        }
    }
}


void *Server::sendNotificationsHandler(void *handlerArgs)
{
    struct communiction_handler_args *args = (struct communiction_handler_args *)handlerArgs;
    Packet notificationPacket;
    int n;

    // transaction
    args->server->retrieve_notifications_from_offline_period(args->user, args->client_address);
    // transaction
    while(1)
    {    
        vector<notification> notifications;

        args->server->read_notifications(args->client_address, &notifications);
        for(auto it = std::begin(notifications); it != std::end(notifications); ++it)
        {        
            notificationPacket = Packet(NOTIFICATION_PKT, it->timestamp, it->body.c_str(), it->author.c_str());
            
            n = args->connectedSocket->sendPacket(notificationPacket);
            if (n<0)
            {
                args->server->close_session(args->user, args->client_address);
                return NULL;
            }
        }
    }
}