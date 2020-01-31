#ifndef MAIN_H
#define MAIN_H
#include "Factorio/factorio.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <memory>

class QFactorioWebApiTester : public QObject {
	Q_OBJECT;
	Factorio::Api factorio;
	Factorio::LoginToken m_token;
	
	QCommandLineParser m_args;
	QCommandLineOption m_quiet;
	QCommandLineOption m_one;
	QCommandLineOption m_verbose;
	
	QString m_currentMod;
	QStringList m_mods;
	QStringList m_matchMods;
	QStringList m_nonMatchMods;
	
	QDateTime m_minT;
	QDateTime m_maxT;
public:
	QFactorioWebApiTester(QCoreApplication& parent);
	void onLoginFinished(WebApi::MetaType*);
	void onTokenVerified(WebApi::MetaType*);
	void onFullModInfoReceived(WebApi::MetaType*);
private:
	void loadNextMod();
};

#endif // MAIN_H
