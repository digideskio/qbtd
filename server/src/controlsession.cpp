#include "controlsession_p.hpp"
#include "qbtd/serversession.hpp"
#include "commandhandler.hpp"
#include "qbtd/jsonerror.hpp"

#include <QtCore/QtDebug>

using qbtd::control::ControlSession;
using qbtd::control::ServerSession;
using qbtd::exception::JsonError;

ControlSession::Private::Private( ServerSession * session, ControlSession * host ):
host( host ),
session( session ) {
	this->session->setParent( this );
	this->host->connect( this->session, SIGNAL( disconnected() ), SIGNAL( disconnected() ) );
	this->connect( this->session, SIGNAL( requested( const QString &, const QVariant & ) ), SLOT( onRequested( const QString &, const QVariant & ) ) );
}

ControlSession::Private::~Private() {
	this->session->deleteLater();
}

void ControlSession::Private::onRequested( const QString & command, const QVariant & args ) {
	qDebug() << command << args;
	auto response = CommandHandler::instance().execute( command, args );
	try {
		this->session->response( response.first, response.second );
	} catch( JsonError & e ) {
		this->session->response( false, QString( "server can not encode response" ) );
	}
}

ControlSession::ControlSession( ServerSession * session, QObject * parent ):
QObject( parent ),
p_( new Private( session, this ) ) {
}

void ControlSession::close() {
	this->p_->session->disconnectFromClient();
}
