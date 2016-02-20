//
// Created by weitao on 8/25/15.
//

#include "connector.h"
#include "socket.h"
#include "stream_socket.h"

CConnector::CConnector(CStreamSocket *stream_socket, CReactor &reactor)
        : CEventHandler(stream_socket, reactor)
{
}

void CConnector::connect(CSocketAddress &address)
{
    m_socket->connect(address);
    m_reactor.add_event(EV_READ|EV_TIMEOUT, this);
}
void CConnector::connect_nb(CSocketAddress &address, int time_out)
{
    m_socket->connect_nb(address, time_out);
    m_reactor.add_event(EV_READ|EV_TIMEOUT, this);
}

int CConnector::handle_input()
{
    //连接成功或数据可读
    return 0;
}

int CConnector::handle_timeout()
{
    //超时，从reactor中注销
    m_reactor.del_event(EV_ALL, this);
    return 0;
}