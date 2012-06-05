#ifndef QBTD_CLIENT_WIDGET_MAINWINDOW_HPP_
#define QBTD_CLIENT_WIDGET_MAINWINDOW_HPP_

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "serverdialog.hpp"
#include "qbtd/clientsession.hpp"
#include "uploaddialog.hpp"

namespace qbtd {
namespace widget {

class MainWindow::Private: public QObject {
	Q_OBJECT
public:
	explicit Private( MainWindow * owner );

public slots:
	void onConnectToServer();
	void onConnected();
	void onError( bool stop, const QString & message );
	void onResponsed( bool result, const QVariant & data );
	void onUploadTorrent();

public:
	MainWindow * owner;
	Ui::MainWindow ui;
	ServerDialog * serverDialog;
	control::ClientSession * session;
	UploadDialog * uploadDialog;
};

}
}

#endif
