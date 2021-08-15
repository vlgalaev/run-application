#include "AppProcess.h"

AppProcess::AppProcess(const sfs::path& wd, const sfs::path& application,
	const QString& parameters, QObject *parent) : QObject(parent),
	_application(QProcess(this)),
	_listening(false)
{
	_application.setWorkingDirectory(QString(wd.string().c_str()));
	_application.setProgram(QString(application.string().c_str()));
	_application.setArguments(QStringList() << parameters);
	_application.setReadChannelMode(QProcess::SeparateChannels);
	//_application.setCurrentReadChannel(QProcess::StandardOutput);

	connect(&_application, &QProcess::errorOccurred,
		this, &AppProcess::errorOccured);
	connect(&_application, &QProcess::readyReadStandardOutput,
		this, &AppProcess::readyReadStandardOutput);
	connect(&_application, &QProcess::stateChanged,
		this, &AppProcess::stateChanged);

	connect(&_application, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		this, &AppProcess::finished);
}

void AppProcess::startApp() 
{
	_listening = true;
	_application.start();
	_application.waitForFinished(-1);

}

void AppProcess::stopApp() 
{
	_listening = false;
	emit report("Closed by user!");
	_application.kill();
}

void AppProcess::errorOccured(const QProcess::ProcessError& error) 
{
	if (!_listening) return;
	switch (error)
	{
	case QProcess::FailedToStart:
		emit report("Failed to start");
		break;
	case QProcess::Crashed:
		emit report("Crashed");
		break;
	case QProcess::Timedout:
		emit report("TimedOut");
		break;
	case QProcess::ReadError:
		emit report("Read error");
		break;
	case QProcess::WriteError:
		emit report("Write error");
		break;
	default:
		emit report("Unknnown error");
		break;
	}
}

void AppProcess::readyReadStandardOutput() 
{
	if (!_listening) return;
	emit report(_application.readAllStandardOutput().trimmed());
}

void AppProcess::stateChanged(const QProcess::ProcessState& newState) 
{
	if (!_listening) return;
	switch (newState)
	{
	case QProcess::NotRunning:
		emit report("Not Running");
		break;
	case QProcess::Starting:
		emit report("Starting...");
		break;
	case QProcess::Running:
		emit report("Running");
		break;
	}
}

void AppProcess::finished(const int& exitCode, const QProcess::ExitStatus& exitStatus)
{
	if (!_listening) return;
	emit report("ExitCode: " + QString::fromStdString(std::to_string(exitCode)) +
		" ExitStatus: " + QString(exitStatus ? "CrashExit" : "NormalExit"));
	emit stdError(_application.readAllStandardError().trimmed());

}