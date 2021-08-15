#pragma once
#include <qprocess.h>
#include <qthread.h>
#include <qstringlist.h>

#include <filesystem>

#include "MAIN/common_p.h"
#include "ReLogManager/ReLoggingClass.h"


namespace sfs = std::filesystem;

class AppProcess :
	public QObject
{
	Q_OBJECT
public:
	AppProcess(const sfs::path& wd, const sfs::path& application,
		 const QString& parameters, QObject *parent = nullptr);

	void startApp();
	void stopApp();

signals:
	void report(const QString& message);
	void stdError(const QString& message);

private slots:
	void errorOccured(const QProcess::ProcessError& error);
	void readyReadStandardOutput();
	void stateChanged(const QProcess::ProcessState& newState);
	void finished(const int& exitCode, const QProcess::ExitStatus& exitStatus);

private:
	bool _listening;
	QProcess _application;

};