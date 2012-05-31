#include "serversession_p.hpp"
#include "localsessionsocket.hpp"

#include <QtCore/QTextStream>

using qbtd::control::ServerSession;
using qbtd::control::SessionSocket;

ServerSession::Private::Private( SessionSocket * socket ):
QObject(),
socket( socket ),
engine( new QScriptEngine( this ) ) {
	this->connect( this->socket, SIGNAL( readyRead() ), SLOT( onSynReceived() ) );
	this->connect( this->socket, SIGNAL( closed() ), SLOT( onDisconnected() ) );
}

void ServerSession::Private::onDisconnected() {
	QMetaObject::invokeMethod( this->socket, "deleteLater" );

	emit this->disconnected();
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
	// TODO set state to OK
}

void ServerSession::Private::onRequested() {
	QTextStream sin( this->socket );
	QString line = sin.readLine();

	QString json = QString::fromUtf8( QByteArray::fromBase64( line.toUtf8() ) );
	this->engine->globalObject().setProperty( "tmp", json );
	QScriptValue v = this->engine->evaluate( "JSON.parse( tmp );" );
	this->engine->globalObject().setProperty( "tmp", this->engine->nullValue() );
	QVariantMap map = v.toVariant().toMap();

	QString command = map.value( "command" ).toString();
	QVariant args = map.value( "args" );
	emit this->requested( command, args );
}

ServerSession::ServerSession( QLocalSocket * socket, QObject * parent ):
QObject( parent ),
p_( new Private( new LocalSessionSocket( socket ) ) ) {
	this->connect( this->p_.get(), SIGNAL( requested( const QString &, const QVariant & ) ), SIGNAL( requested( const QString &, const QVariant & ) ) );
}

ServerSession::ServerSession( QTcpSocket * socket, QObject * parent ):
QObject( parent ),
p_( new Private( nullptr ) ) {
	this->connect( this->p_.get(), SIGNAL( requested( const QString &, const QVariant & ) ), SIGNAL( requested( const QString &, const QVariant & ) ) );
}

void ServerSession::close() {
	this->p_->socket->close();
}

void ServerSession::response( bool result, const QVariant & data ) {
	QVariantMap package;
	package.insert( "result", result );
	package.insert( "data", data );

	this->p_->engine->globalObject().setProperty( "tmp", this->p_->engine->newVariant( package ) );
	QString json = this->p_->engine->evaluate( "JSON.stringify( tmp );" ).toString();
	this->p_->engine->globalObject().setProperty( "tmp", this->p_->engine->nullValue() );

	this->p_->socket->write( json.toUtf8().toBase64().append( "\n" ) );
}
