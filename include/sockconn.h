#ifndef SOCKCONN_H
#define SOCKCONN_H

#define MAXHOSTNAME 256
#define BACKLOG_LIMIT 5

int establishConnection(int portnum);
int connectToServer(const char *hostname, int portnum);

#endif // SOCKCONN_H