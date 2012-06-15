#include "settings_p.hpp"
#include "qbtd/fileerror.hpp"
#include "json.hpp"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>

#include <cassert>

using qbtd::utility::Settings;
using qbtd::exception::FileError;
using qbtd::exception::Exception;

std::unique_ptr< Settings, std::function< void ( Settings * ) > > Settings::Private::self;

void Settings::Private::destroy( Settings * data ) {
	delete data;
}

Settings::Private::Private( const QString & path ):
path( path ),
engine(),
data() {
	QFile fin( path );
	if( !fin.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
		throw FileError( fin.errorString() );
	}
	QTextStream sin( &fin );
	sin.setCodec( QTextCodec::codecForName( "UTF-8" ) );
	QString content = sin.readAll();
	fin.close();

	this->data = fromJSON( content, &this->engine ).toMap();
}

void Settings::Private::save() {
	QString json = toJSON( this->data, "\t", &this->engine );
	QFile fout( this->path );
	if( !fout.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
		throw FileError( fout.errorString() );
	}
	QTextStream sout( &fout );
	sout.setCodec( QTextCodec::codecForName( "UTF-8" ) );
	sout << json << endl;
	fout.close();;
}

void Settings::initialize( const QString & path ) {
	if( Private::self ) {
		return;
	}
	Private::self.reset( new Settings( path ) );
	Private::self.get_deleter() = &Private::destroy;
}

Settings & Settings::instance() {
	if( !Private::self ) {
		assert( !"qbtd::utility::Settings was not initialized" );
	}
	return *Private::self;
}

Settings::Settings( const QString & path ):
p_( std::make_shared< Private >( path ) ) {
}

Settings::~Settings() {
	try {
		this->p_->save();
	} catch( ... ) {
		// TODO log error to stderr
	}
}

QVariant Settings::get( const QString & key ) const {
	auto it = this->p_->data.find( key );
	if( it == this->p_->data.end() ) {
		return QVariant();
	}
	return it.value();
}

void Settings::set( const QString & key, const QVariant & value ) {
	auto it = this->p_->data.find( key );
	if( it == this->p_->data.end() ) {
		this->p_->data.insert( key, value );
	} else {
		it.value() = value;
	}
	// TODO let client save
	this->p_->save();
}
