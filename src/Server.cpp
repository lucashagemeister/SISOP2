#include "../include/Server.hpp"

using namespace std;


Server::Server(map<string, int> possibleServerAddresses)
{
    this->electionStarted = false;
    this->gotAnsweredInElection = false;

    this->possibleServerAddresses = possibleServerAddresses;

    this->notification_id_counter = 0;
    this->serverConfirmation = -1;

    connectedServersMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond_notification_empty, NULL);
    pthread_cond_init(&cond_notification_full, NULL);
    pthread_mutex_init(&connectedServersMutex, NULL);
    pthread_mutex_init(&electionMutex, NULL);
    pthread_mutex_init(&confirmedEventsMutex, NULL);
    pthread_mutex_init(&seqn_transaction_serializer, NULL);
}

Server::Server(host_address address)
{
    this->electionStarted - false;
    this->gotAnsweredInElection = false;
    this->notification_id_counter = 0;
	this->ip = address.ipv4;
	this->port = address.port;
    this->serverConfirmation = -1;

    connectedServersMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond_notification_empty, NULL);
    pthread_cond_init(&cond_notification_full, NULL);
    pthread_mutex_init(&connectedServersMutex, NULL);
    pthread_mutex_init(&electionMutex, NULL);
    pthread_mutex_init(&confirmedEventsMutex, NULL);
    pthread_mutex_init(&seqn_transaction_serializer, NULL);
}


void Server::updatePrimaryServerInfo(string ip, int listeningPort, int id){
    this->primarySeverIP = ip;
    this->primarySeverPort = listeningPort;
    this->primarySeverID = id;
}

void Server::updatePrimaryServerInfo(string ip, int listeningPort){
    this->primarySeverIP = ip;
    this->primarySeverPort = listeningPort;
    this->primarySeverID = this->getIdFromAddress(ip, listeningPort);
}

void Server::removeSelfFromPossibleServerAddresses(){
    this->possibleServerAddresses.erase(this->possibleServerAddresses.find(this->ip));
}


pair<string, int> Server::getIpPortFromAddressString(string addressString){

    string delimiter = ":";
    string ip = addressString.substr(0, addressString.find(delimiter));
    int port;
    
    addressString.erase(0, addressString.find(delimiter) + delimiter.length());
    port = atoi(addressString.c_str());
    return pair<string, int>(ip, port);
}


int Server::getIdFromAddress(string ip, int port){
    int id;
    string stringId = ip + to_string(port);
    stringId.erase(remove(stringId.begin(), stringId.end(), '.'), stringId.end());
    id = atoi(stringId.c_str());
    return id;
}


void Server::setAddress(string ip, int port){
    this->ip = ip;
    this->port = port;
    this->id = this->getIdFromAddress(ip, port);
}


void Server::addPeerToConnectedServers(int peerID, Socket* connectedSocket){
    pthread_mutex_lock(&this->connectedServersMutex);
    this->connectedServers.insert(pair<int, Socket*>(peerID, connectedSocket));
    pthread_mutex_unlock(&this->connectedServersMutex);
}


void Server::removePeerFromConnectedServers(int peerID){
    pthread_mutex_lock(&this->connectedServersMutex);
    this->connectedServers.erase(this->connectedServers.find(peerID));
    pthread_mutex_unlock(&this->connectedServersMutex);
}


void Server::setAsPrimaryServer(){
    this->backupMode = false;

    this->updatePrimaryServerInfo(this->ip, this->port, this->id);

    // ####################################################################################
    // EFETUAR OUTROS "TRÂMITES" QUE PRECISAR PRA SETAR PASSAR ESSE SERVER COMO O PRIMÁRIO
    // ####################################################################################
}


bool Server::try_to_start_session(string user, host_address address)
{
    cout << "\nTrying to start session\n";
    pthread_mutex_lock(&seqn_transaction_serializer);
    uint16_t seqn = get_current_sequence();

    event session_event;
    session_event.seqn = seqn;
    session_event.command = OPEN_SESSION;
    strcpy(session_event.arg1, user.c_str());
    strcpy(session_event.arg2, address.ipv4.c_str());
    strcpy(session_event.arg3, to_string(address.port).c_str());
    session_event.committed = false; 

    deepcopy_user_sessions_semaphore(true);
    deepcopy_sessions(true);
    deepcopy_users_unread_notifications(true);
    deepcopy_followers(true);
    deepcopy_active_notifications(true);
    deepcopy_active_users_pending_notifications(true);

    if(!user_exists(user))
    {
        sem_t num_sessions;
        sem_init(&num_sessions, 0, 2);
        COPY_user_sessions_semaphore.insert({user, num_sessions}); // user is created with 2 sessions available
        COPY_sessions.insert({user, list<host_address>()});
        COPY_followers.insert(pair<string, list<string>>(user, list<string>()));
        COPY_users_unread_notifications.insert({user, list<uint32_t>()});
    } 
    
    int session_started = sem_trywait(&(COPY_user_sessions_semaphore[user])); // try to consume a session resource
    if(session_started == 0) // 0 if session started, -1 if not
    { 
        COPY_sessions[user].push_back(address);
        COPY_active_users_pending_notifications.insert({address, priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>>()});
    }

    bool committed;

    if (backupMode)
    {
        committed = wait_primary_commit(session_event);
    }
    else
    {

        committed = send_backup_change(session_event);
    }

    if (committed) 
    {
        deepcopy_user_sessions_semaphore(false);
        deepcopy_sessions(false);
        deepcopy_users_unread_notifications(false);
        deepcopy_followers(false);
        deepcopy_active_notifications(false);
        deepcopy_active_users_pending_notifications(false);
    }

    session_event.committed = committed;
    event_history.push_back(session_event);  

    print_sessions();
    print_events();
    pthread_mutex_unlock(&seqn_transaction_serializer);
    return committed && session_started == 0; 
}

uint16_t Server::get_current_sequence()
{
    if(event_history.empty())
    {
        return 1;
    } 

    return event_history.back().seqn + 1;
}

bool Server::user_exists(string user)
{
    return user_sessions_semaphore.find(user) != user_sessions_semaphore.end();
}


bool Server::didAllBackupsRespondedEvent(uint16_t eventSeqn){
    pthread_mutex_lock(&confirmedEventsMutex);
    pthread_mutex_lock(&connectedServersMutex);
    
    map<int, bool> eventMap;

    // Get the event from the map
    auto it = this->confirmedEvents.find(eventSeqn);
    if (it != this->confirmedEvents.end())
        eventMap = it->second;
    else
    {
        cout << "WARNING! Method didAllBackupsRespondedEvent() called but event is not inside the confirmedEvents map!\n";
        pthread_mutex_unlock(&connectedServersMutex);
        pthread_mutex_unlock(&confirmedEventsMutex);
        return false;
    }


    for (auto &peer : this->connectedServers)
    {
        auto it = eventMap.find(peer.first);

        if (it == eventMap.end()) // Not found
        {
            pthread_mutex_unlock(&connectedServersMutex);
            pthread_mutex_unlock(&confirmedEventsMutex);
            return false;
        } 
    }

    pthread_mutex_unlock(&connectedServersMutex);
    pthread_mutex_unlock(&confirmedEventsMutex);
    return true;
}


bool Server::didAllBackupsOkedEvent(uint16_t eventSeqn){
    pthread_mutex_lock(&confirmedEventsMutex);

    map<int, bool> eventMap;

    // Get the event from the map
    auto it = this->confirmedEvents.find(eventSeqn);
    if (it != this->confirmedEvents.end())
        eventMap = it->second;
    else{
        cout << "WARNING! Method didAllBackupsOkedEvent() called but event is not inside the confirmedEvents map!\n";
        pthread_mutex_unlock(&connectedServersMutex);
        pthread_mutex_unlock(&confirmedEventsMutex);
        return false;
    }

    for (auto &peer : this->connectedServers){

        auto it = eventMap.find(peer.first);

        if (it == eventMap.end())  { // Not found
            cout << "WARNING! Method didAllBackupsOkedEvent() called but a server haven't responded yet!\n";
            pthread_mutex_unlock(&connectedServersMutex);
            pthread_mutex_unlock(&confirmedEventsMutex);
            return false;
        } 

        // Else: peer responded something to event
        if (!it->second){   // Backup didn't oked event, event must be undone
            pthread_mutex_unlock(&connectedServersMutex);
            pthread_mutex_unlock(&confirmedEventsMutex);
            return false;
        }

    }

    pthread_mutex_unlock(&connectedServersMutex);
    pthread_mutex_unlock(&confirmedEventsMutex);
    return true;
}


bool Server::wait_primary_commit(event e)
{
    cout << "Confirming event alteration to primary replica.\n";
    this->sendPacketToPrimaryServer(Packet(OK, e));
    cout << "Confirmation sent.\n";
    // Wait primary response
    while(serverConfirmation == -1)
    {
        // busy wait thread change value;
        sleep(1);
    }
    int s = serverConfirmation;
    this->serverConfirmation = -1; 
    
    if (s)
        return true;
    else
        return false;
}

bool Server::send_backup_change(event e)
{
    cout << "Sending event to all backup replicas.\n";
    // Add event seqn to the map of confirmed events
    pthread_mutex_lock(&confirmedEventsMutex);
    confirmedEvents.insert({e.seqn, {}});
    pthread_mutex_unlock(&confirmedEventsMutex);

    // Send new event to backup servers
    Packet eventPacket = Packet(e.command, e);
    this->sendPacketToAllServersInTheGroup(eventPacket);

    time_t startTime = time(0);
    double secondsSiceStart;
    // Wait for all backup servers response until timeout
    while(!didAllBackupsRespondedEvent(e.seqn))
    {
        secondsSiceStart = difftime( time(0), startTime);

        if (secondsSiceStart >= BACKUPS_RESPONSE_TIMEOUT)  // Backups response timed out
        {
            cout << "Timeout for backup replicas response!\n";
            sendPacketToAllServersInTheGroup(Packet(SNOK, e));
            return false;  
        }

    }

    if (didAllBackupsOkedEvent(e.seqn)) {
        cout << "Responding SOK to backups!\n";
        sendPacketToAllServersInTheGroup(Packet(SOK, e));
        return true;
    }
    else{
        cout << "Responding SNOK to backups D:\n";
        sendPacketToAllServersInTheGroup(Packet(SNOK, e));
        return false;  
    }
}

// call this function when new notification is created
bool Server::create_notification(string user, string body, time_t timestamp)
{
    cout << "\nNew notification!\n";
    pthread_mutex_lock(&seqn_transaction_serializer);
    uint16_t seqn = get_current_sequence();

    event create_notification_event;
    create_notification_event.seqn = seqn;
    create_notification_event.command = CREATE_NOTIFICATION;
    strcpy(create_notification_event.arg1, user.c_str());
    strcpy(create_notification_event.arg2, body.c_str());
    strcpy(create_notification_event.arg3, to_string(timestamp).c_str());
    create_notification_event.committed = false; 

    deepcopy_users_unread_notifications(true);
    deepcopy_active_notifications(true);
    deepcopy_active_users_pending_notifications(true);

    if (followers[user].size() > 0)
    {
        uint16_t pending_users{0};
        for (auto follower : followers[user])
        {                    
            COPY_users_unread_notifications[follower].push_back(notification_id_counter);
            pending_users++;
        }

        notification notif(notification_id_counter, user, timestamp, body, body.length(), pending_users);
        COPY_active_notifications.push_back(notif);
        assign_notification_to_active_sessions(notification_id_counter, followers[user]);
        notification_id_counter += 1;
    }

    bool committed;
    if (backupMode)
    {
        committed = wait_primary_commit(create_notification_event);
    }
    else
    {
        committed = send_backup_change(create_notification_event);
    }

    if (committed)
    {
        deepcopy_users_unread_notifications(false);
        deepcopy_active_notifications(false);
        deepcopy_active_users_pending_notifications(false);
    } 

    create_notification_event.committed = committed;
    event_history.push_back(create_notification_event);
    pthread_mutex_unlock(&seqn_transaction_serializer);

    print_events();
    print_users_unread_notifications();
    return committed;
}

// call this function after new notification is created
void Server::assign_notification_to_active_sessions(uint32_t notification_id, list<string> followers) 
{
    cout << "\nAssigning new notification to active sessions...\n";
    
    for (auto user : followers)
    {
        if(user_is_active(user)) 
        {
            for(auto address : sessions[user]) 
            {
                COPY_active_users_pending_notifications[address].push(notification_id);
            }
            // when all sessions from same user have notification on its entry, remove @ from list
            list<uint32_t>::iterator it = find(COPY_users_unread_notifications[user].begin(), COPY_users_unread_notifications[user].end(), notification_id);
            COPY_users_unread_notifications[user].erase(it);

            // signal as many consumers to send to clients as pending users times possible sessions
            for(int i = 0; i < followers.size()*2; i++)
            {
                pthread_cond_signal(&cond_notification_full);

            }
        }
    }

}

bool Server::user_is_active(string user) 
{
    return sessions.find(user) != sessions.end() && !(sessions[user].empty());
}

// call this function when new session is started (after try_to_start_session()) to wake notification producer to client
void Server::retrieve_notifications_from_offline_period(string user, host_address addr) 
{
    cout << "\nGetting notifications from offline period to active sessions...\n";
    pthread_mutex_lock(&seqn_transaction_serializer);
    uint16_t seqn = get_current_sequence();

    event read_from_offline_period_event;
    read_from_offline_period_event.seqn = seqn;
    read_from_offline_period_event.command = READ_OFFLINE;
    strcpy(read_from_offline_period_event.arg1, user.c_str());
    strcpy(read_from_offline_period_event.arg2, addr.ipv4.c_str());
    strcpy(read_from_offline_period_event.arg3, to_string(addr.port).c_str());
    read_from_offline_period_event.committed = false; 

    deepcopy_users_unread_notifications(true);
    deepcopy_active_users_pending_notifications(true);
    
    for(auto notification_id : users_unread_notifications[user]) 
    {
        COPY_active_users_pending_notifications[addr].push(notification_id);
    }
    
    (COPY_users_unread_notifications[user]).clear();

    bool committed;
    if (backupMode)
    {
        committed = wait_primary_commit(read_from_offline_period_event);
    }
    else
    {
        committed = send_backup_change(read_from_offline_period_event);
    }

    if (committed)
    {
        deepcopy_users_unread_notifications(false);
        deepcopy_active_users_pending_notifications(false);
    } 

    read_from_offline_period_event.committed = committed;
    event_history.push_back(read_from_offline_period_event);

    print_events();
    print_users_unread_notifications();

    // signal consumer
    pthread_cond_signal(&cond_notification_full);
    pthread_mutex_unlock(&seqn_transaction_serializer);
}

// call this function on consumer thread that will feed the user with its notifications
void Server::read_notifications(host_address addr, vector<notification>* notifications) 
{
    pthread_mutex_lock(&seqn_transaction_serializer);
    cout << "\nReading notifications of active session...\n";

    while (active_users_pending_notifications[addr].empty()) { 
        // sleep while user doesn't have notifications to read
        cout << "No notifications for address " << addr.ipv4 <<":"<< addr.port << ". Sleeping...\n";
        pthread_cond_wait(&cond_notification_full, &seqn_transaction_serializer); 
    }

    cout << "Assembling notifications...\n";
    uint16_t seqn = get_current_sequence();
    event read_notification_event;
    read_notification_event.seqn = seqn;
    read_notification_event.command = READ_NOTIFICATIONS;
    strcpy(read_notification_event.arg1, addr.ipv4.c_str());
    strcpy(read_notification_event.arg2, to_string(addr.port).c_str());
    strcpy(read_notification_event.arg3, "");
    read_notification_event.committed = false; 

    deepcopy_active_users_pending_notifications(true);

    while(!COPY_active_users_pending_notifications[addr].empty()) 
    {
        uint32_t notification_id = (COPY_active_users_pending_notifications[addr]).top();
        for(auto notif : active_notifications)
        {
            if(notif.id == notification_id)
            {
                notifications->push_back(notif);
                break;
            }
        }
        
        (COPY_active_users_pending_notifications[addr]).pop();
    }

    bool committed;
    if (backupMode)
    {
        committed = wait_primary_commit(read_notification_event);
    }
    else
    {
        committed = send_backup_change(read_notification_event);
    }

    if (committed)
    {
        deepcopy_active_users_pending_notifications(false);
    } 
    else
    {
        notifications->clear();
    }
    

    read_notification_event.committed = committed;
    event_history.push_back(read_notification_event);

    print_events();
    print_users_unread_notifications();

    // signal producer
    pthread_cond_signal(&cond_notification_empty);
    pthread_cond_signal(&cond_notification_full); // waking someone again can improve client consumption flow
    pthread_mutex_unlock(&seqn_transaction_serializer);
}

// call this function when client presses ctrl+c or ctrl+d
void Server::close_session(string user, host_address address) 
{
    pthread_mutex_lock(&seqn_transaction_serializer);
    uint16_t seqn = get_current_sequence();

    event close_session_event;
    close_session_event.seqn = seqn;
    close_session_event.command = CLOSE_SESSION;
    strcpy(close_session_event.arg1, user.c_str());
    strcpy(close_session_event.arg2, address.ipv4.c_str());
    strcpy(close_session_event.arg3, to_string(address.port).c_str());
    close_session_event.committed = false; 

    deepcopy_active_users_pending_notifications(true);
    deepcopy_sessions(true);
    deepcopy_user_sessions_semaphore(true);

    list<host_address>::iterator it = find(sessions[user].begin(), sessions[user].end(), address);
    if(it != COPY_sessions[user].end()) // remove address from sessions map and < (ip, port), notification to send > 
    {
        COPY_sessions[user].erase(it);
        COPY_active_users_pending_notifications.erase(address);

        // signal semaphore
        sem_post(&(COPY_user_sessions_semaphore[user]));
    }

    bool committed;
    if (backupMode)
    {
        committed = wait_primary_commit(close_session_event);
    }
    else
    {
        committed = send_backup_change(close_session_event);
    }

    if (committed)
    {
        deepcopy_active_users_pending_notifications(false);
        deepcopy_sessions(false);
        deepcopy_user_sessions_semaphore(false);
    }

    close_session_event.committed = committed;
    event_history.push_back(close_session_event);

    pthread_mutex_unlock(&seqn_transaction_serializer);
}

bool Server::follow_user(string user, string user_to_follow)
{
    pthread_mutex_lock(&seqn_transaction_serializer);
    uint16_t seqn = get_current_sequence();

    event follow_event;
    follow_event.seqn = seqn;
    follow_event.command = FOLLOW;
    strcpy(follow_event.arg1, user.c_str());
    strcpy(follow_event.arg2, user_to_follow.c_str());
    strcpy(follow_event.arg3, "");
    follow_event.committed = false; 

    deepcopy_followers(true);

    if (find(COPY_followers[user_to_follow].begin(), COPY_followers[user_to_follow].end(), user) == COPY_followers[user_to_follow].end()
        && user_exists(user_to_follow))
    {
        COPY_followers[user_to_follow].push_back(user);
    }

    bool committed;
    if (backupMode)
    {
        committed = wait_primary_commit(follow_event);
    }
    else
    {
        committed = send_backup_change(follow_event);
    }

    if (committed)
    {
        deepcopy_followers(false);
    }

    follow_event.committed = committed;
    event_history.push_back(follow_event);

    print_events();
    print_followers();

    pthread_mutex_unlock(&seqn_transaction_serializer);

    return committed;
}


void Server::deepcopy_user_sessions_semaphore(bool save)
{
    map<string, sem_t> from;
    map<string, sem_t>* to;

    if (save)
    {
        from = user_sessions_semaphore;
        to = &COPY_user_sessions_semaphore;
    }
    else
    {
        from = COPY_user_sessions_semaphore;
        to = &user_sessions_semaphore;
    }

    to->clear();

    for(auto it = from.begin(); it != from.end(); it++)
    {
        sem_t num_sessions;
        int value;
        sem_getvalue(&(it->second), &value);
        sem_init(&num_sessions, 0, value);

        to->insert({it->first, num_sessions});
    }
}
void Server::deepcopy_sessions(bool save)
{
    map< string, list<host_address> > from;
    map< string, list<host_address> >* to;

    if (save)
    {
        from = sessions;
        to = &COPY_sessions;
    }
    else
    {
        from = COPY_sessions;
        to = &sessions;
    }

    to->clear();

    for(auto it = from.begin(); it != from.end(); it++)
    {
        list<host_address> addresses;
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            host_address addr;
            addr.ipv4 = itl->ipv4;
            addr.port = itl->port;
            addresses.push_back(addr);
        }
        to->insert({it->first, addresses});
    }
}
void Server::deepcopy_users_unread_notifications(bool save) 
{
    map< string, list< uint32_t>> from;
    map< string, list< uint32_t>>* to;

    if (save)
    {
        from = users_unread_notifications;
        to = &COPY_users_unread_notifications;
    }
    else
    {
        from = COPY_users_unread_notifications;
        to = &users_unread_notifications;
    }

    to->clear();

    for(auto it = from.begin(); it != from.end(); it++)
    {
        list< uint32_t> notifications;
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            notifications.push_back(*itl);
        }
        to->insert({it->first, notifications});
    }
}
void Server::deepcopy_followers(bool save) // save or retrieve
{
    map<string, list<string>> from;
    map<string, list<string>>* to;

    if (save)
    {
        from = followers;
        to = &COPY_followers;
    }
    else
    {
        from = COPY_followers;
        to = &followers;
    }

    to->clear();

    for(auto it = from.begin(); it != from.end(); it++)
    {
        list< string> notifications;
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            notifications.push_back(*itl);
        }
        to->insert({it->first, notifications});
    }
}
void Server::deepcopy_active_notifications(bool save)
{
    vector<notification> from;
    vector<notification>* to;

    if (save)
    {
        from = active_notifications;
        to = &COPY_active_notifications;
    }
    else
    {
        from = COPY_active_notifications;
        to = &active_notifications;
    }

    to->clear();

    for(auto it = from.begin(); it != from.end(); it++)
    {
        to->push_back(*it);
    }
}
void Server::deepcopy_active_users_pending_notifications(bool save)
{
    map< host_address, priority_queue< uint32_t, vector<uint32_t>, greater<uint32_t>>> from;
    map< host_address, priority_queue< uint32_t, vector<uint32_t>, greater<uint32_t>>>* to;

    if (save)
    {
        from = active_users_pending_notifications;
        to = &COPY_active_users_pending_notifications;
    }
    else
    {
        from = COPY_active_users_pending_notifications;
        to = &active_users_pending_notifications;
    }

    to->clear();

    for(auto it = from.begin(); it != from.end(); it++)
    {
        vector<uint32_t> notifications;
        priority_queue< uint32_t, vector<uint32_t>, greater<uint32_t>> temp_queue;
        while(!((it->second).empty()))
        {
            uint32_t notif = (it->second).top();
            notifications.push_back(notif);
            temp_queue.push(notif);

            (it->second).pop();
        }
        for (auto itv = notifications.begin(); itv != notifications.end(); itv++)
        {
            (it->second).push(*itv);
        }
        
        to->insert({it->first, temp_queue});
    }
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
void Server::print_events() 
{
    cout << "\nEvents: " << event_history.size() << "\n";

    for(auto it = event_history.begin(); it != event_history.end(); it++)
    {
        cout << it->seqn << ": ";
        cout << it->command << "(";
        cout << it->arg1 << ", ";
        cout << it->arg2 << ", ";
        cout << it->arg3 << "), [";
        cout << it->committed << "]\n";
    }
}

void Server::print_COPY_users_unread_notifications() 
{
    cout << "\nCOPY_Users unread notifications: \n";

    for(auto it = COPY_users_unread_notifications.begin(); it != COPY_users_unread_notifications.end(); it++)
    {
        cout << it->first << ": [";
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            cout << *itl << ", ";
        }
        cout << "]\n";
    }
}
void Server::print_COPY_active_users_unread_notifications() 
{
    cout << "COPY_Active users notifications to receive: \n";

    for(auto it = COPY_active_users_pending_notifications.begin(); it != COPY_active_users_pending_notifications.end(); it++)
    {
        cout << it->first.ipv4 << ":" << it->first.port << ": [";
        cout << it->second.size() << "]\n";
    }
}
void Server::print_COPY_sessions() 
{
    cout << "\nCOPY_Sessions: " << COPY_sessions.size() << "\n";

    for(auto it = COPY_sessions.begin(); it != COPY_sessions.end(); it++)
    {
        cout << it->first << ": [";
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            cout << (*itl).ipv4 << ":" << (*itl).port << ", ";
        }
        cout << "]\n";
    }
}
void Server::print_COPY_active_notifications() 
{
    cout << "\nCOPY_Notifications: " << COPY_active_notifications.size() << "\n";

    for(auto it = COPY_active_notifications.begin(); it != COPY_active_notifications.end(); it++)
    {
        cout << it->id << "\n";
        cout << it->author << "\n";
        cout << it->body << "\n";
        cout << "\n";
    }
}
void Server::print_COPY_followers() 
{
    cout << "\nCOPY_Followers: " << COPY_followers.size() << "\n";

    for(auto it = COPY_followers.begin(); it != COPY_followers.end(); it++)
    {
        cout << it->first << ": [";
        for(auto itl = (it->second).begin(); itl != (it->second).end(); itl++)
        {
            cout << *itl << ", ";
        }
        cout << "]\n";
    }
}


void Server::sendPacketToAllServersInTheGroup(Packet p){

    pthread_mutex_lock(&connectedServersMutex);
    for (auto &peer : this->connectedServers){
        peer.second->sendPacket(p);
    }
    pthread_mutex_unlock(&connectedServersMutex);
}

void Server::sendPacketToPrimaryServer(Packet p){

    pthread_mutex_lock(&connectedServersMutex);
    for (auto &peer : this->connectedServers){
        if (peer.first == this->primarySeverID){
            peer.second->sendPacket(p);
            break;
        }
    }
    pthread_mutex_unlock(&connectedServersMutex);
}


void Server::sendElectionPacketForGreaterIds(){
    for (auto &peer : this->connectedServers){
        if (peer.first > this->id) {
            cout << "Sending ELECTION packet to server with ID " << peer.first << "\n";
            peer.second->sendPacket(Packet(ELECTION, ""));
        }
            
    }
}


void *Server::electionTimeoutHandler(void *handlerArgs){

    // Unroll arguments
    struct group_communiction_handler_args *args = (struct group_communiction_handler_args *)handlerArgs;
    Server* server = args->server;
    string myAddressString = server->ip + ":" + to_string(server->port);

    while(true){

        pthread_mutex_lock(&server->electionMutex);
        if(server->electionStarted){
            pthread_mutex_unlock(&server->electionMutex);
            sleep(ELECTION_TIMEOUT);
            if(!server->gotAnsweredInElection){
                cout << "Didn't receive any ANSWER packets before timeout. Autoelecting as primary server...\n\n";
                pthread_mutex_lock(&server->electionMutex);
                server->setAsPrimaryServer();
                server->electionStarted = false;
                server->gotAnsweredInElection = false;
                server->sendPacketToAllServersInTheGroup(Packet(COORDINATOR, myAddressString.c_str()));
                pthread_mutex_unlock(&server->electionMutex);
            }
        }
        pthread_mutex_unlock(&server->electionMutex);
    }
}


void *Server::groupReadMessagesHandler(void *handlerArgs)
{
    // Unroll arguments
    struct group_communiction_handler_args *args = (struct group_communiction_handler_args *)handlerArgs;
    Server* server = args->server;
    int peerID = args->peerID;
    Socket* connectedSocket = args->connectedSocket;

    string primaryServerAddress;
    string receivedPayload;
    pair<string, int> ipPort;
    host_address addrServ;

    while(1){
        Packet* receivedPacket = connectedSocket->readPacket();
        
        if (receivedPacket == NULL){ 
            server->removePeerFromConnectedServers(peerID);

            if (peerID == server->primarySeverID){
                server->serverConfirmation = 0;
                cout << "\nLost connection with primary server, initializing election... \n";
                pthread_mutex_lock(&server->electionMutex);
                server->electionStarted = true;
                server->gotAnsweredInElection = false;
                pthread_mutex_unlock(&server->electionMutex);
                server->sendElectionPacketForGreaterIds();
    
            }
            return NULL;
        }

        switch(receivedPacket->getType()){

            case ASK_PRIMARY:
                primaryServerAddress = server->primarySeverIP + ":" + to_string(server->primarySeverPort);
                connectedSocket->sendPacket(Packet(PRIMARY_SERVER_ADDRESS, primaryServerAddress.c_str()));
                break;
            
            case PRIMARY_SERVER_ADDRESS:
                ipPort = server->getIpPortFromAddressString(receivedPacket->getPayload());
                server->updatePrimaryServerInfo(ipPort.first, ipPort.second);
                break;
            
            case ELECTION:
                server->serverConfirmation = 0;
                cout << "Received ELECTION packet...\n";
                connectedSocket->sendPacket(Packet(ANSWER, ""));
                break;

            case ANSWER:
                cout << "Received ANSWER packet... \n";
                pthread_mutex_lock(&server->electionMutex);
                server->gotAnsweredInElection = true;
                pthread_mutex_unlock(&server->electionMutex);
                break;

            case COORDINATOR:
                cout << "New Primary server: " << peerID << "\n";
                server->primarySeverID = peerID;
                ipPort = server->getIpPortFromAddressString(receivedPacket->getPayload());
                server->updatePrimaryServerInfo(ipPort.first, ipPort.second);
                pthread_mutex_lock(&server->electionMutex);
                server->electionStarted = false;
                pthread_mutex_unlock(&server->electionMutex);
                break;

            // Backup confirmed alteration
            case OK: {
                pthread_mutex_lock(&server->confirmedEventsMutex);
                cout << "Received OK from backup "<<peerID<<", nice!\n";
                auto it = server->confirmedEvents.find(receivedPacket->e.seqn);
                if (it == server->confirmedEvents.end()){
                    cout << "WARNING! Backup server oked unexisting event! Ignoring...\n";
                } else {
                    it->second.insert({peerID, true});
                }
                pthread_mutex_unlock(&server->confirmedEventsMutex);
                break;
            }

            // All backups confirmed the event modification in server state
            case SOK:
                cout << "Received SOK from primary!\n";
                server->serverConfirmation = 1;
                break;

            // At least one backup didn't oked the event modification, need to revert it
            case SNOK:
                cout << "Received SNOK from primary :( damn!\n";
                server->serverConfirmation = 0;
                break;

            case OPEN_SESSION: {
                cout << "Replicating open session.\n";
                addrServ.ipv4 = receivedPacket->e.arg2;
                addrServ.port = atoi(receivedPacket->e.arg3);
                
                thread command_thread ([&]()
                { 
                    server->try_to_start_session(receivedPacket->e.arg1, addrServ);
                    cout << "FINISHED Replicating open session.\n";
                });
                command_thread.detach();
                                
                break;
            }

            case CLOSE_SESSION: {
                cout << "Replicating close session.\n";
                addrServ.ipv4 = receivedPacket->e.arg2;
                addrServ.port = atoi(receivedPacket->e.arg3);

                thread command_thread ([&]()
                { 
                    server->close_session(receivedPacket->e.arg1, addrServ);
                    cout << "FINISHED Replicating close session.\n";
                });
                command_thread.detach();

                break;
            }

            case FOLLOW: {
                cout << "Replicating FOLLOW command.\n";
                
                thread command_thread ([&]()
                { 
                    server->follow_user(receivedPacket->e.arg1, receivedPacket->e.arg2);
                    cout << "FINISHED Replicating FOLLOW command.\n";
                });
                command_thread.detach();

                break;
            }

            case CREATE_NOTIFICATION: {
                cout << "Replicating SEND command.\n";

                thread command_thread ([&]()
                { 
                    server->create_notification(receivedPacket->e.arg1, 
                                receivedPacket->e.arg2, atoi(receivedPacket->e.arg3));
                    cout << "FINISHED Replicating SEND command.\n";
                });
                command_thread.detach();
                
                break;
            }
            
            case READ_NOTIFICATIONS:{
                cout << "Replicating notification read.\n";
                addrServ.ipv4 = receivedPacket->e.arg1;
                addrServ.port = atoi(receivedPacket->e.arg2);
                vector<notification> n;

                thread command_thread ([&]()
                { 
                    server->read_notifications(addrServ, &n);
                    cout << "FINISHED Replicating notification read.\n";
                });
                command_thread.detach();
                
                break;
            }
            
            case READ_OFFLINE: {
                cout << "Replicating read offline notifications.\n";
                addrServ.ipv4 = receivedPacket->e.arg2;
                addrServ.port = atoi(receivedPacket->e.arg3);

                thread command_thread ([&]()
                { 
                    server->retrieve_notifications_from_offline_period(receivedPacket->e.arg1, addrServ);
                    cout << "FINISHED Replicating read offline notifications.\n";
                });
                command_thread.detach();
                
                break;
            }

            default:
                break;
        }
    }    

}


void Server::sendMessagesForConnectionEstablishment(Socket* connectedSocket, int peerID){

    int connectionStatus = connectedSocket->sendPacket(Packet(SERVER_PEER_CONNECTING, std::to_string(this->id).c_str()));
    if (connectionStatus < 0){
        this->removePeerFromConnectedServers(peerID);
        return;
    }
        
    if (this->backupMode){    // Asks peer who's the primary server
        int primaryServerPort;

        Packet askForPrimary = Packet(ASK_PRIMARY, "");
        connectionStatus = connectedSocket->sendPacket(askForPrimary);

        if (connectionStatus < 0){
            this->removePeerFromConnectedServers(peerID);
            return;
        }
    }

    if (peerID == this->primarySeverID){
        // ############################################
        // Asks primary server for current server state
        // ############################################
    }   
}



ServerSocket::ServerSocket() : Socket(){
    this->serv_addr.sin_family = AF_INET;
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

    std::cout << "New connection established on socket: " << *newsockfd << "\n\n";



    Packet* connectionType = newConnectionSocket->readPacket();
        
    if (connectionType->getType() == SERVER_PEER_CONNECTING){

        group_communiction_handler_args *args = (group_communiction_handler_args *) calloc(1, sizeof(group_communiction_handler_args));
        args->peerID = atoi(connectionType->getPayload());
        args->connectedSocket = newConnectionSocket;
        args->server = server;
        server->addPeerToConnectedServers(args->peerID, newConnectionSocket);

        pthread_create(threadID, NULL, Server::groupReadMessagesHandler, (void *)args);
        return;
    }


    // ELSE (a client is connecting):
    // waits election finishes
    pthread_mutex_lock(&server->electionMutex);
    while (server->electionStarted){
        pthread_mutex_unlock(&server->electionMutex);
        pthread_mutex_lock(&server->electionMutex);
    }
    pthread_mutex_unlock(&server->electionMutex);

    // Sends primary server information
    if (server->backupMode){
        newConnectionSocket->sendPacket(Packet(MESSAGE_PKT, server->primarySeverIP.c_str()));
        newConnectionSocket->sendPacket(Packet(MESSAGE_PKT, std::to_string(server->primarySeverPort).c_str()));
        return;
    }
    else {
        newConnectionSocket->sendPacket(Packet(ALREADY_PRIMARY, ""));
    }

    
    // Verify if there are free sessions available
    // read client username from socket in 'user' var
    Packet *userPacket = newConnectionSocket->readPacket();

    if (userPacket == NULL){
        std::cout << "Unable to read user information. Closing connection.\n";
        return;     // destructor automatically closes the socket
    } else 
        user = userPacket->getPayload();
    

    if (userPacket->getType() == USER_INFO_PKT){
        client_address.ipv4 = inet_ntoa(cli_addr.sin_addr);
        client_address.port = ntohs(cli_addr.sin_port);

        bool sessionAvailable = server->try_to_start_session(user, client_address);

        Packet sessionResultPkt;
        if (!sessionAvailable){
            sessionResultPkt = Packet(SESSION_OPEN_FAILED, "Unable to connect to server: no sessions available or consistency precaution.");
            newConnectionSocket->sendPacket(sessionResultPkt);
            return; // destructor automatically closes the socket
        } else{
            sessionResultPkt = Packet(SESSION_OPEN_SUCCEDED, "Connection succeded! Session established.");
            newConnectionSocket->sendPacket(sessionResultPkt);
        }
    }

    else {// User was already connected, doesn't need to start session again
        Packet *clientOriginalPort = newConnectionSocket->readPacket();
        client_address.ipv4 = inet_ntoa(cli_addr.sin_addr);
        client_address.port = atoi(clientOriginalPort->getPayload());
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
    struct hostent *serverHost;
    serv_addr.sin_family = AF_INET;
    
    // Assume server starts in backup mode and only changes it if there already are server instances running
    server->backupMode = true;

    bool noConnections = true;
    for (auto &possibleAddress : server->possibleServerAddresses){
        
        // Set IP adress
        serverHost = gethostbyname(possibleAddress.first.c_str());
        serv_addr.sin_addr = *((struct in_addr *)serverHost->h_addr);

        // Set port
        serv_addr.sin_port = htons(possibleAddress.second);

        if (this->connectToMember(serv_addr, possibleAddress.first, server))
            noConnections = false;
    }

    if (noConnections)
        server->setAsPrimaryServer();
 }


bool ServerSocket::connectToMember(sockaddr_in serv_addr, string ip, Server* server){

    Socket *peerConnectedSocket = new Socket();
    pthread_t serverServerThread;
    int peerPort = ntohs(serv_addr.sin_port);

    cout << "Trying to connect to peer server at " << ip << ":" << peerPort << "... ";

    if (connect(peerConnectedSocket->getSocketfd(),(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        cout << "Failed, no server instance running here.\n";
        return false;
    }

    // else 
    cout << "Connected!" << "\n";

    int peerID = server->getIdFromAddress(ip, peerPort);
    server->addPeerToConnectedServers(peerID, peerConnectedSocket);
    server->sendMessagesForConnectionEstablishment(peerConnectedSocket, peerID);
    
    group_communiction_handler_args *args = (group_communiction_handler_args *) calloc(1, sizeof(group_communiction_handler_args));
    args->connectedSocket = peerConnectedSocket;
    args->server = server;
    args->peerID = peerID;

    pthread_create(&serverServerThread, NULL, Server::groupReadMessagesHandler, (void *)args);
    return true;
}


void ServerSocket::bindAndListen(Server* server){
    
    bool bindSucceeded = false;
    string ip;
    int port;

    for (auto & possibleAddr : server->possibleServerAddresses){
        
        ip = possibleAddr.first;
        port = possibleAddr.second;

        this->serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        this->serv_addr.sin_port = htons(port);


        if (bind(this->getSocketfd(), (struct sockaddr *) &this->serv_addr, sizeof(this->serv_addr)) < 0) {
            cout << "Address " << ip << ":" << port << "  already taken\n";
        } else {
            server->setAddress(ip, port);
            server->removeSelfFromPossibleServerAddresses();
            bindSucceeded = true;
            break;
        }
    }
	
    if (bindSucceeded) {
        listen(this->getSocketfd(), MAX_TCP_CONNECTIONS);
        std::cout << "Listening on address " << ip << ":" << port << " ...\n"; 
    } else {
        std::cout << "ERROR on binding!\n";
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
                if(args->server->follow_user(args->user, userToFollow))
                    args->connectedSocket->sendPacket(Packet(MESSAGE_PKT, response.c_str()));
                else 
                    args->connectedSocket->sendPacket(Packet(MESSAGE_PKT, "Follow failed, try again."));
                break;

            case COMMAND_SEND_PKT:
                if(args->server->create_notification(args->user, receivedPacket->getPayload(), receivedPacket->getTimestamp()))
                    args->connectedSocket->sendPacket(Packet(MESSAGE_PKT, "Notification sent!"));
                else
                    args->connectedSocket->sendPacket(Packet(MESSAGE_PKT, "Send failed, try again."));
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


    args->server->retrieve_notifications_from_offline_period(args->user, args->client_address);
    
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