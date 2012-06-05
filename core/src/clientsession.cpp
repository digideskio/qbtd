#include "clientsession_p.hpp"
#include "localsessionsocket.hpp"
#include "json.hpp"

#include <QtCore/QTextStream>

using qbtd::control::ClientSession;

ClientSession::Private::Private():
QObject(),
socket( nullptr ),
engine( new QScriptEngine( this ) ) {
}

void ClientSession::Private::onDisconnected() {
	emit this->disconnected();
}

void ClientSession::Private::onConnected() {
	this->connect( this->socket, SIGNAL( readyRead() ), SLOT( onAckReceived() ) );
	this->socket->write( "=0\n" );
}

void ClientSession::Private::onAckReceived() {
	this->socket->disconnect( SIGNAL( readyRead() ), this, SLOT( onAckReceived() ) );
	QTextStream sin( this->socket );
	QString ack = sin.readLine();
	if( ack != "=0" ) {
		// TODO report error and close session
		this->socket->close();
		this->socket->deleteLater();
		this->socket = nullptr;
		return;
	}
	this->connect( this->socket, SIGNAL( readyRead() ), SLOT( onResponse() ) );
	emit this->connected();
}

void ClientSession::Private::onResponse() {
	if( !this->socket->canReadLine() ) {
		return;
	}
	QTextStream sin( this->socket );
	QString line = sin.readLine();
	line = QString::fromUtf8( QByteArray::fromBase64( line.toUtf8() ) );

	QVariantMap map = utility::fromJSON( line, this->engine ).toMap();

	bool result = map.value( "result" ).toBool();
	QVariant data = map.value( "data" );
	emit this->responsed( result, data );
}

ClientSession::ClientSession( QObject * parent ):
QObject( parent ),
p_( new Private ) {
	this->connect( this->p_.get(), SIGNAL( connected() ), SIGNAL( connected() ) );
	this->connect( this->p_.get(), SIGNAL( disconnected() ), SIGNAL( disconnected() ) );
	this->connect( this->p_.get(), SIGNAL( responsed( bool, const QVariant & ) ), SIGNAL( responsed( bool, const QVariant & ) ) );
}

void ClientSession::close() {
	this->p_->socket->close();
	this->p_->socket->deleteLater();
	this->p_->socket = nullptr;
}

void ClientSession::connectToServer( const QString & path ) {
	this->p_->socket = new LocalSessionSocket( path, this );
	this->p_->connect( this->p_->socket, SIGNAL( opened() ), SLOT( onConnected() ) );
	this->p_->connect( this->p_->socket, SIGNAL( closed() ), SLOT( onDisconnected() ) );
	this->connect( this->p_->socket, SIGNAL( error( bool, const QString & ) ), SIGNAL( error( bool, const QString & ) ) );
	this->p_->socket->open( QIODevice::ReadWrite );
}

void ClientSession::request( const QString & command, const QVariant & args ) {
	QVariantMap package;
	package.insert( "command", command );
	package.insert( "args", args );

	QString json = utility::toJSON( package, this->p_->engine );

	this->p_->socket->write( json.toUtf8().toBase64().append( "\n" ) );
}
