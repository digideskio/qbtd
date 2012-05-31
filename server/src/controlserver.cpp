#include "controlserver_p.hpp"
#include "qbtd/serversession.hpp"

#include <QtCore/QtDebug>

#include <cassert>

using qbtd::control::ControlServer;

std::unique_ptr< ControlServer, std::function< void ( ControlServer * ) > > ControlServer::Private::self;

void ControlServer::Private::destory( ControlServer * data ) {
	delete data;
}

ControlServer::Private::Private():
server(),
sessions() {
	this->connect( &this->server, SIGNAL( newConnection() ), SLOT( onNewConnection() ) );
}

void ControlServer::Private::onNewConnection() {
	while( this->server.hasPendingConnections() ) {
		ControlSession * session = new ControlSession( this->server.nextPendingConnection(), this );
		this->connect( session, SIGNAL( disconnected() ), SLOT( onSessionDisconnected() ) );
		this->sessions.push_back( session );
	}
}

void ControlServer::Private::onSessionDisconnected() {
	ControlSession * session = static_cast< ControlSession * >( this->sender() );
	assert( session != nullptr || !"ControlSession casting failed" );
	auto it = std::remove_if( this->sessions.begin(), this->sessions.end(), [session]( ControlSession * s )->bool {
		return session == s;
	} );
	std::for_each( it, this->sessions.end(), []( ControlSession * s )->void {
		s->deleteLater();
	} );
	this->sessions.erase( it, this->sessions.end() );
}

void ControlServer::initialize() {
	if( Private::self ) {
		return;
	}
	Private::self.reset( new ControlServer );
	Private::self.get_deleter() = Private::destory;
}

ControlServer & ControlServer::instance() {
	if( !Private::self ) {
		assert( !"not initialized" );
	}
	return *Private::self;
}

ControlServer::ControlServer():
p_( new Private ) {
}

ControlServer::~ControlServer() {
	// stop listen control session
	this->p_->server.close();
	// disconnect all session
	for( auto it = this->p_->sessions.begin(); it != this->p_->sessions.end(); ++it ) {
		( *it )->close();
	}
}

bool ControlServer::listen( const QString & path ) {
	return this->p_->server.listen( path );
}

bool ControlServer::listen( const QHostAddress & address ) {
	return false;
}
