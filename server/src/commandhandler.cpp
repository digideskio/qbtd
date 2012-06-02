#include "commandhandler_p.hpp"
#include "torrentsession.hpp"

#include <cassert>

using qbtd::control::CommandHandler;
using qbtd::torrent::TorrentSession;

std::unique_ptr< CommandHandler, std::function< void ( CommandHandler * ) > > CommandHandler::Private::self;

void CommandHandler::Private::destory( CommandHandler * data ) {
	delete data;
}

CommandHandler::Private::Private():
commands() {
	this->commands.insert( "add", std::bind( &Private::add, this, std::placeholders::_1 ) );
}

std::pair< bool, QVariant > CommandHandler::Private::add( const QVariant & args ) {
	QUrl url = args.toUrl();
	TorrentSession::instance().addTorrent( url );
	return std::make_pair( true, QVariant() );
}

void CommandHandler::initialize() {
	if( Private::self ) {
		return;
	}
	Private::self.reset( new CommandHandler );
	Private::self.get_deleter() = Private::destory;
}

CommandHandler & CommandHandler::instance() {
	if( !Private::self ) {
		assert( !"not initialized" );
	}
	return *Private::self;
}

CommandHandler::CommandHandler():
p_( new Private ) {
}

CommandHandler::~CommandHandler() {
}

std::pair< bool, QVariant > CommandHandler::execute( const QString & command, const QVariant & args ) const {
	auto it = this->p_->commands.find( command );
	if( it == this->p_->commands.end() ) {
		return std::make_pair( false, QObject::tr( "invalid command %1" ).arg( command ) );
	}
	return it.value()( args );
}
