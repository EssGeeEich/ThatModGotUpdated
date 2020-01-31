#include "main.h"
#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <iostream>

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	
	// QTimer is needed!
	// If no QTimer is used, QFactorioWebApiTester constructor
	//  will be unable to exit() the program!
	std::unique_ptr<QFactorioWebApiTester> tester;
	QTimer::singleShot(0,[&](){
		tester = std::make_unique<QFactorioWebApiTester>(app);
	});

	return app.exec();
}

void QFactorioWebApiTester::onFullModInfoReceived(WebApi::MetaType* payload)
{
	try {
		auto info = payload->Get<WebApi::Result<Factorio::FullModInfo>>();
		
		if(info && info->Response() == WebApi::RC_OK)
		{
			Factorio::FullModInfo const* mod = info->Get();
			if(!mod)
			{
				std::cerr << tr("Could not retrieve mod informations for mod \"%1\".")
							 .arg(m_currentMod)
							 .toStdString() << std::endl;
				m_nonMatchMods.push_back(std::move(m_currentMod));
			}
			else
			{
				QVector<Factorio::ModReleaseInfo> const& releases
					= mod->m_basicInfo.m_releases;
				
				Factorio::ModReleaseInfo const* latestMatchingRelease = nullptr;
				
				for(auto const& it : releases)
				{
					if(it.m_releaseTime > m_minT &&
						it.m_releaseTime < m_maxT &&
						(
							!latestMatchingRelease ||
							it.m_releaseTime > latestMatchingRelease->m_releaseTime
						)
					)
					{
						latestMatchingRelease = &it;
					}
				}
				if(latestMatchingRelease)
				{
					if(!m_args.isSet(m_quiet))
						std::cout << tr("Mod \"%1\" matched the filter: Last release (version %2) is dated %3.")
									 .arg(m_currentMod)
									 .arg(latestMatchingRelease->m_modVersion)
									 .arg(latestMatchingRelease->m_releaseTime.toString("yyyy-MM-ddTHH:mm:ss.zzz000Z"))
									 .toStdString() << std::endl;
					m_matchMods.push_back(std::move(m_currentMod));
				}
				else
					m_nonMatchMods.push_back(std::move(m_currentMod));
			}
		}
	} catch(...) {}
	
	loadNextMod();
}

void QFactorioWebApiTester::loadNextMod()
{
	if(m_args.isSet(m_quiet))
	{
		if((!m_matchMods.isEmpty()) && m_args.isSet(m_one))
		{
			qApp->exit(0);
			return;
		}
		
		if((!m_nonMatchMods.isEmpty()) && (!m_args.isSet(m_one)))
		{
			qApp->exit(1);
			return;
		}
	}
	
	if(m_mods.isEmpty())
	{
		if(!m_args.isSet(m_quiet))
		{
			if(!m_matchMods.isEmpty())
			{
				std::cout << tr("%1 mod/s matched the filter.")
							 .arg(m_matchMods.size())
							 .toStdString() << std::endl;
				if(m_args.isSet(m_verbose))
				{
					for(auto const& it : m_matchMods)
					{
						std::cout << "    " << it.toStdString() << std::endl;
					}
				}
			}
			if(!m_nonMatchMods.isEmpty())
			{
				std::cout << tr("%1 mod/s did not match the filter.")
							 .arg(m_nonMatchMods.size())
							 .toStdString() << std::endl;
				
				if(m_args.isSet(m_verbose))
				{
					for(auto const& it : m_nonMatchMods)
					{
						std::cout << "    " << it.toStdString() << std::endl;
					}
				}
			}
		}
		if(m_nonMatchMods.isEmpty())
			qApp->exit(0);
		//if(m_matchMods.isEmpty())
		//	qApp->exit(1);
		qApp->exit(1);
		return;
	}
	
	m_currentMod = std::move(m_mods.front());
	m_mods.pop_front();
	
	WebApi::MetaConnector connector = factorio.GetFullModInfo(m_token, m_currentMod);
	connect(connector.get(), &WebApi::MetaConnectorImpl::onFinished,
			this, &QFactorioWebApiTester::onFullModInfoReceived);
}

void QFactorioWebApiTester::onLoginFinished(WebApi::MetaType* payload)
{
	auto token = payload->Get<WebApi::Result<Factorio::LoginToken>>();

	if(token && token->Response() == WebApi::RC_OK && token->Get() != nullptr)
	{
		m_token = *(token->Get());
		loadNextMod();
	}
	else
	{
		// Ignore "quiet" flag on purpose
		std::cerr << tr("Error: Login failed.")
					 .toStdString() << std::endl;
		qApp->exit(1);
	}
}

void QFactorioWebApiTester::onTokenVerified(WebApi::MetaType* payload)
{
	auto vstatus = payload->Get<WebApi::Result<bool>>();

	if(vstatus && vstatus->Response() == WebApi::RC_OK && vstatus->Get() != nullptr
			&& (*vstatus->Get()) == true)
	{
		loadNextMod();
	}
	else
	{
		// Ignore "quiet" flag on purpose
		std::cerr << tr("Error: Token verification failed.")
					 .toStdString() << std::endl;
		qApp->exit(1);
	}
}


QFactorioWebApiTester::QFactorioWebApiTester(QCoreApplication& parent)
	: QObject(&parent),
	  factorio(this),
	  m_quiet(QStringList{"q","quiet"},tr("Stop stdout. Ends program ASAP, might check fewer mods. Only useful for its return code. Overrides the verbose flag.")),
	  m_one(QStringList{"s","single"},tr("Change behavior of the program's exit code. Return exit code 0 if any mod matches the filter. Default: Require all mods to match filter.")),
	  m_verbose(QStringList{"v","verbose"},tr("List all the mods that have succeeded or failed the check.")),
	  m_minT(QDateTime::fromMSecsSinceEpoch(1)),
	  m_maxT(QDateTime::currentDateTime().addYears(1))
{
	m_args.setApplicationDescription(tr("Factorio WebApi Commandline Tool"));
	m_args.addHelpOption();
	
#ifdef REQUIRE_LOGIN
	m_args.addOption(QCommandLineOption(QStringList{"u","user"},tr("Username to identify as while requesting data."),"user"));
	m_args.addOption(QCommandLineOption(QStringList{"t","token"},tr("Token to use to confirm the identity of the user."),"token"));
	m_args.addOption(QCommandLineOption(QStringList{"p","password"},tr("Password to use to confirm the identity of the user."),"password"));
#else
	m_args.addOption(QCommandLineOption(QStringList{"u","user"},tr("Placeholder for Username."),"user"));
	m_args.addOption(QCommandLineOption(QStringList{"t","token"},tr("Placeholder for Token."),"token"));
	m_args.addOption(QCommandLineOption(QStringList{"p","password"},tr("Placeholder for Password."),"password"));
#endif
	m_args.addOption(QCommandLineOption(QStringList{"m","mod"},tr("Look up this mod's last update. May use multiple values."),"mod"));
	m_args.addOption(QCommandLineOption(QStringList{"d","disabled-file"},tr("Look up the mods' last update time extracting a list from a mod-list.json file (disabled entries only)."),"disabled-file"));
	m_args.addOption(QCommandLineOption(QStringList{"e","enabled-file"},tr("Look up the mods' last update time extracting a list from a mod-list.json file (enabled entries only)."),"enabled-file"));
	m_args.addOption(QCommandLineOption(QStringList{"a","after"},tr("List mods that have a release dated after this DateTime. Format: yyyy-MM-ddTHH:mm:ss.zzz000Z."),"after"));
	m_args.addOption(QCommandLineOption(QStringList{"b","before"},tr("List mods that have a release dated before this DateTime. Format: yyyy-MM-ddTHH:mm:ss.zzz000Z."),"before"));
	m_args.addOption(m_quiet);
	m_args.addOption(m_one);
	m_args.addOption(m_verbose);
	
	m_args.process(parent);
	if(m_args.isSet("after"))
	{
		m_minT = QDateTime::fromString(m_args.value("after"),"yyyy-MM-ddTHH:mm:ss.zzz000Z");
		if(!m_minT.isValid())
		{
			std::cerr << tr("Invalid after value. Format: yyyy-MM-ddTHH:mm:ss.zzz000Z.")
					  .toStdString() << std::endl
					  << tr("Example: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz000Z"))
					  .toStdString() << std::endl;
			qApp->exit(1);
			return;
		}
	}
	if(m_args.isSet("before"))
	{
		m_maxT = QDateTime::fromString(m_args.value("before"),"yyyy-MM-ddTHH:mm:ss.zzz000Z");
		if(!m_maxT.isValid())
		{
			std::cerr << tr("Invalid before value. Format: yyyy-MM-ddTHH:mm:ss.zzz000Z.")
						 .toStdString() << std::endl
						 << tr("Example: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz000Z"))
						 .toStdString() << std::endl;
			qApp->exit(1);
			return;
		}
	}
	
	m_mods = m_args.values("mod");
	if(m_args.isSet("disabled-file"))
	{
		QFile file(m_args.value("disabled-file"));
		if(!file.open(QFile::ReadOnly | QFile::Text))
		{
			std::cerr << tr("Invalid disabled-file. Cannot open file.")
						 .toStdString() << std::endl;
			qApp->exit(1);
			return;
		}
		QByteArray qba = file.readAll();
		QJsonDocument qjd = QJsonDocument::fromJson(qba);
		QJsonObject json = qjd.object();
		
		if(json.contains("mods"))
		{
			QJsonArray arr = json["mods"].toArray();
			for(auto const& it : arr)
			{
				QJsonObject entry = it.toObject();
				if(entry.contains("name") &&
						entry.contains("enabled") &&
						!entry.value("enabled").toBool())
					m_mods.push_back(entry.value("name").toString());
			}
		}
	}
	if(m_args.isSet("enabled-file"))
	{
		QFile file(m_args.value("enabled-file"));
		if(!file.open(QFile::ReadOnly | QFile::Text))
		{
			std::cerr << tr("Invalid enabled-file. Cannot open file.")
						 .toStdString() << std::endl;
			qApp->exit(1);
			return;
		}
		QByteArray qba = file.readAll();
		QJsonDocument qjd = QJsonDocument::fromJson(qba);
		QJsonObject json = qjd.object();
		
		if(json.contains("mods"))
		{
			QJsonArray arr = json["mods"].toArray();
			for(auto const& it : arr)
			{
				QJsonObject entry = it.toObject();
				if(entry.contains("name") &&
						entry.contains("enabled") &&
						entry.value("enabled").toBool())
					m_mods.push_back(entry.value("name").toString());
			}
		}
	}
	
	m_mods.removeDuplicates();
	m_mods.sort(Qt::CaseInsensitive);
	
#ifdef REQUIRE_LOGIN
	if(m_args.isSet("token"))
	{
		m_token = {m_args.value("user"), m_args.value("token")};
		WebApi::MetaConnector connector = factorio.ValidateLogin(m_token);
		connect(connector.get(), &WebApi::MetaConnectorImpl::onFinished,
			this, &QFactorioWebApiTester::onTokenVerified);
	}
	else if(m_args.isSet("password"))
	{
		WebApi::MetaConnector connector
			= factorio.TryLogin("my nick here", "my password here", true);
		connect(connector.get(), &WebApi::MetaConnectorImpl::onFinished,
			this, &QFactorioWebApiTester::onLoginFinished);
	}
	else
#endif
	{
		WebApi::Result<bool> tokenValidity(WebApi::RC_OK, true);
		WebApi::MetaType meta(this,&tokenValidity);
		onTokenVerified(&meta);
	}
}
