//
//  Copyright (c) 2008 Tweak Software.
//  All rights reserved.
//
//  SPDX-License-Identifier: Apache-2.0
//
//
#include <TwkQtChat/Server.h>
#include <TwkQtChat/Connection.h>
#include <QtNetwork/QtNetwork>

namespace TwkQtChat
{
    using namespace std;

    Server::Server(QObject* parent, int port, ConnectionFactory fact)
        : QTcpServer(parent)
        , m_connectionFactory(fact)
    {
        int fallbackCount = 10;
        if (getenv("TWK_SERVER_PORT_FALLBACK_COUNT"))
        {
            fallbackCount = atoi(getenv("TWK_SERVER_PORT_FALLBACK_COUNT"));
        }

        // Disable application-wide proxy if any for this QTcpServer
        setProxy(QNetworkProxy::NoProxy);

        //
        //  Attempting to listen is itself the availability test: it's atomic
        //  and authoritative for the address family we're about to serve on.
        //
        //  This used to probe each port with an outbound QTcpSocket
        //  connection and treat a successful connect as "port in use". That
        //  is not the same question as "can I bind this port", and on
        //  macOS 26 (Tahoe) with Qt 5.15 the two answers diverge:
        //  waitForConnected() reports ConnectedState for loopback ports that
        //  nothing is listening on, so every candidate looked busy and
        //  starting the network failed with "cannot find a free network
        //  port" even though every port in the range was in fact bindable.
        //
        //  The probe was also wrong in ways that don't depend on the OS: it
        //  only ever connected to IPv4 127.0.0.1 while we listen on
        //  QHostAddress::Any (dual-stack), so an IPv6 listener was invisible
        //  to it and listen() would then fail with EADDRINUSE; conversely a
        //  port held only on a non-loopback address answered the probe and
        //  was skipped despite being usable. It additionally cost up to
        //  fallbackCount * 300 ms on the calling thread.
        //
        for (int p = port; p <= port + fallbackCount; ++p)
        {
            if (listen(QHostAddress::Any, p))
                return;
        }

        //
        //  Every port in the preferred range is taken. Rather than leaving
        //  networking unavailable, let the OS assign a free port; the actual
        //  port is published through Client::serverPort(), so it reaches the
        //  network dialog and the port file rvpush reads.
        //
        if (listen(QHostAddress::Any, 0))
        {
            cerr << "WARNING: RvNetwork: ports " << port << "-" << port + fallbackCount << " are all in use, listening on port "
                 << serverPort() << " instead" << endl;
            return;
        }

        cerr << "ERROR: RvNetwork: cannot listen on any network port, tried " << port << "-" << port + fallbackCount
             << " and an OS-assigned port - " << errorString().toStdString() << endl;
    }

    void Server::incomingConnection(qintptr socketDescriptor)
    {
        Connection* connection = 0;
        if (m_connectionFactory)
            connection = (*m_connectionFactory)(this, true);
        else
            connection = new Connection(this, true);

        connection->setSocketDescriptor(socketDescriptor);
        emit newConnection(connection);
    }

} // namespace TwkQtChat
