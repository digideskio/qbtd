#include <QtCore/QCoreApplication>

#include "torrentsession.hpp"
#include "controlserver.hpp"

#include <csignal>

void cleanup( int /*signum*/ ) {
	QCoreApplication::instance()->quit();
}

int main( int argc, char * argv[] ) {
	signal( SIGHUP, cleanup );
	signal( SIGINT, cleanup );
	signal( SIGTERM, cleanup );
	signal( SIGQUIT, cleanup );

	QCoreApplication a( argc, argv );

	qbtd::torrent::TorrentSession::initialize();
	qbtd::control::ControlServer::initialize();

	qbtd::control::ControlServer::instance().listen( "qbtd" );

	return a.exec();
}
