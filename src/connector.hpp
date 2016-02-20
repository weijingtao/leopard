#ifndef _SRC_NET_CONNECTOR_H
#define _SRC_NET_CONNECTOR_H

#include "event_handler.h"

class CStreamSocket;
class CSocketAddress;

class CConnector : public CEventHandler
{
public:
    CConnector(CStreamSocket *stream_socket, CReactor &reactor);

    virtual ~CConnector(){}

    void connect(CSocketAddress &address);

    void connect_nb(CSocketAddress &address, int time_out);

    virtual int handle_input();

    virtual int handle_timeout();
};

#endif //_SRC_NET_CONNECTOR_H