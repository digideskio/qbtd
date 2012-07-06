#include "controlsession_p.hpp"
#include "qbtd/jsonerror.hpp"

#include <cassert>

using qbtd::control::ControlSession;
using qbtd::exception::JsonError;

std::unique_ptr< ControlSession, std::function< void ( ControlSession * ) > > ControlSession::Private::self( NULL, Private::destroy );

void ControlSession::Private::destroy( ControlSession * data ) {
	delete data;
}

ControlSession::Private::Private():
QObject(),
session( new ClientSession( this ) ),
success() {
	this->connect( this->session, SIGNAL( responsed( bool, const QVariant & ) ), SLOT( onResponsed( bool, const QVariant & ) ) );
}

void ControlSession::Private::onResponsed( bool result, const QVariant & data ) {
	if( !this->success ) {
		return;
	}
	this->success( result, data );
	this->success = NULL;
}

void ControlSession::initialize() {
	if( Private::self ) {
		return;
	}
	Private::self.reset( new ControlSession );
	Private::self.get_deleter() = &Private::destroy;
}

ControlSession & ControlSession::instance() {
	if( !Private::self ) {
		assert( !"ControlSession not initialized" );
	}
	return *Private::self;
}

ControlSession::ControlSession():
QObject(),
p_( new Private ) {
	this->connect( this->p_->session, SIGNAL( connected() ), SIGNAL( connected() ) );
	this->connect( this->p_->session, SIGNAL( disconnected() ), SIGNAL( disconnected() ) );
	this->connect( this->p_->session, SIGNAL( error( bool, const QString & ) ), SIGNAL( error( bool, const QString & ) ) );
}

ControlSession::~ControlSession() {
}

void ControlSession::connectToServer( const QString & path ) {
	this->p_->session->connectToServer( path );
}

void ControlSession::connectToServer( const QHostAddress & address, quint16 port ) {
	this->p_->session->connectToServer( address, port );
}

void ControlSession::disconnectFromServer() {
	this->p_->session->disconnectFromServer();
}

void ControlSession::request( const QString & command, const QVariant & args, SuccessCallback success ) {
	if( this->p_->success ) {
		return;
	}
	this->p_->success = success;
	try {
		this->p_->session->request( command, args );
	} catch( JsonError & e ) {
		this->p_->success = NULL;
		emit this->error( false, QObject::tr( "Can not encode request: %1" ).arg( e.getMessage() ) );
	}
}
