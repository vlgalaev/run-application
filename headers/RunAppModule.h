#pragma once

#include "RunAppParameters.h"
#include "AppProcess.h"
#include <fstream>

#include "ReLogManager/ReLoggingClass.h"

#include <QCoreApplication>

namespace sfs = std::filesystem;

struct COMMON_PARAM;

class QString;

class SDataset;

class Swap {
public:
	sfs::path _storageDir;
	std::string _tracesIO_fn;
	std::string _headersIO_fn;
	std::string _headerNamesO_fn;

	std::fstream _tracesInput;
	std::fstream _headersInput;
	std::fstream _tracesOutput;
	std::fstream _headersOutput;
	std::fstream _headerNamesOutput;

protected:
	Swap(sfs::path storage_dir = sfs::path());

	void open_in();
	void open_out();
	void close_in();
	void close_out();

};

class RunAppModule : public ReLoggingClass
{
	Q_DECLARE_TR_FUNCTIONS(RunAppModule)

public:
	//! @brief Returns the name of the module
	static QString getTitle ();

	//! @brief  Validates parameters
	//! @return true/false
	//! @see areParametersValid()
	static bool validateParameters (const RunAppParameters &parameters, QString *p_error_string = nullptr);

public:
	RunAppModule (COMMON_PARAM &first_frame_cp, const RunAppParameters &parameters);

	//! @brief  Checks if parameters are valid
	//! @return true/false
	//! @see validateParameters()
	inline bool areParametersValid () const;

	//! @brief Logs module parameters
	void logParameters () const;

	//! @brief Processes a frame of data
	void processFrame (COMMON_PARAM &cp);

private:
	void substituteReplicaInTemplatizedParameter(COMMON_PARAM &cp);

private:
	RunAppParameters m_parameters;

};

class SDataset : public Swap
{
public:
	unsigned int _traceCount;
	unsigned int _sampleCount;
	unsigned int _headerCount;

	std::vector<std::string> _headerNames;
	std::vector<float> _traces;
	std::vector<double> _headers;

public:
	SDataset(unsigned int traceCount=0, unsigned int sampleCount=0, unsigned int headerCount=0,
		sfs::path storage_dir = sfs::path());
	SDataset(sfs::path storageDir);

	sfs::path getcwd();

	void toFlow(COMMON_PARAM &cp);
	static SDataset fromFlow(COMMON_PARAM &cp, unsigned int traceCount=0, unsigned int sampleCount=0,
		unsigned int headerCount=0, sfs::path storageDir = sfs::current_path() / "tmp");

	void toFile();
	static SDataset fromFile(sfs::path storageDir = sfs::current_path() / "tmp");

	void clearSwap();
};

class ParametersString : public QString
{
public:
	ParametersString(const QString&);
	QString format();
};

// ----- Implementation -----

bool RunAppModule::areParametersValid () const
{
	return RunAppModule::validateParameters(m_parameters);
}