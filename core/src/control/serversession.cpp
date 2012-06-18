#include "serversession_p.hpp"
#include "localsessionsocket.hpp"
#include "tcpsessionsocket.hpp"
#include "utility/json.hpp"

#include <QtCore/QTextStream>

using qbtd::control::ServerSession;
using qbtd::control::SessionSocket;

ServerSession::Private::Private( SessionSocket * socket, ServerSession * host ):
QObject(),
host( host ),
socket( socket ),
engine( new QScriptEngine( this ) ) {
	this->connect( this->socket, SIGNAL( readyRead() ), SLOT( onSynReceived() ) );
	this->host->connect( this->socket, SIGNAL( closed() ), SIGNAL( disconnected() ) );
}

void ServerSession::Private::onSynReceived() {
	this->socket->disconnect( SIGNAL( readyRead() ), this, SLOT( onSynReceived() ) );
	QTextStream sin( this->socket );
	QString line = sin.readLine();
	if( line != "=0" ) {
		// TODO error report and close socket
		this->socket->close();
		return;
	}
	this->connect( this->socket, SIGNAL( readyRead() ), SLOT( onRequested() ) );
	this->socket->write( "=0\n" );
	// TODO set state to OK
}

void ServerSession::Private::onRequested() {
	if( !this->socket->canReadLine() ) {
		return;
	}
	QTextStream sin( this->socket );
	QString line = sin.readLine();

	QString json = QString::fromUtf8( QByteArray::fromBase64( line.toUtf8() ) );
	QVariantMap map = utility::fromJSON( json, this->engine ).toMap();

	QString command = map.value( "command" ).toString();
	QVariant args = map.value( "args" );
	emit this->requested( command, args );
}

ServerSession::ServerSession( QLocalSocket * socket, QObject * parent ):
QObject( parent ),
p_( new Private( new LocalSessionSocket( socket ), this ) ) {
	this->connect( this->p_.get(), SIGNAL( requested( const QString &, const QVariant & ) ), SIGNAL( requested( const QString &, const QVariant & ) ) );
}

ServerSession::ServerSession( QTcpSocket * socket, QObject * parent ):
QObject( parent ),
p_( new Private( new TcpSessionSocket( socket ), this ) ) {
	this->connect( this->p_.get(), SIGNAL( requested( const QString &, const QVariant & ) ), SIGNAL( requested( const QString &, const QVariant & ) ) );
}

void ServerSession::disconnectFromClient() {
	this->p_->socket->close();
}

void ServerSession::response( bool result, const QVariant & data ) {
	QVariantMap package;
	package.insert( "result", result );
	package.insert( "data", data );

	QString json = utility::toJSON( package, this->p_->engine );

	this->p_->socket->write( json.toUtf8().toBase64().append( "\n" ) );
}
