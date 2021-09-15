#include "RunAppModule.h"

#include "ConfControl/ConfControl.h"
#include "Debug/Assert.h"
#include "MAIN/Common_p.h"
#include "MaskProcessor/headers/DatasetMaskProcessor.h"
#include "MaskProcessor/headers/FileMaskProcessor.h"
#include "ReMisc/Exception.h"
#include "ReplicaTable/ReplicaVarSubstitutor.h"
#include "ReStrConvert.h"

#include <QStringBuilder>
#include <qprocess.h>

#include <fstream>
#include <exception>
#include <filesystem>

#include "AppProcess.h"


namespace sfs = std::filesystem;

class RunAppException : std::exception
{
private:
	int err_code;
public:
};

QString RunAppModule::getTitle ()
{
	return toQtString(MODULE_TITLE);
}

bool RunAppModule::validateParameters (const RunAppParameters &parameters, QString *p_error_string)
{
	Q_UNUSED(parameters);
	Q_UNUSED(p_error_string);

	return true;
}

RunAppModule::RunAppModule (COMMON_PARAM &first_frame_cp, const RunAppParameters &parameters):
	ReLoggingClass(qUtf8Printable(getTitle()), first_frame_cp.pLog),
	m_parameters(parameters)
{
	QString error_string;

	if (!RunAppModule::validateParameters(m_parameters, &error_string))
		throw Util::Exception(error_string);
	
	this->substituteReplicaInTemplatizedParameter(first_frame_cp);
	// do smth


	sfs::path applicationName(m_parameters.getApplicationName().toStdWString());
	ParametersString p = ParametersString(m_parameters.getApplicationParameters()).format();
	_sd = std::make_unique<SDataset>(SDataset::fromFlow(first_frame_cp));
	_app = std::make_unique<AppProcess>(_sd->getcwd().make_preferred(),
		applicationName.make_preferred(), p);
	QObject::connect(_app.get(), &AppProcess::report, [&first_frame_cp](const QString& message) {
		first_frame_cp.pLog->Log("Application Report", message.toStdString().c_str());
	});
	QObject::connect(_app.get(), &AppProcess::stdError, [&first_frame_cp](const QString& message) {
		if (!message.isEmpty())
			first_frame_cp.pLog->Log("Application Errors", message.toStdString().c_str());
	});
	QObject::connect(_app.get(), &AppProcess::depictWorkPercent, [&first_frame_cp](const int& percent) {
		if (percent >= 0 && percent <= 100)
			first_frame_cp.depictWorkPercent(percent);
	});

}

void RunAppModule::processFrame(COMMON_PARAM &cp)
{
	MODULE_VERIFY_CONFIGURATION(cp.dwConfiguration);

	QStringList files_list;
	QStringList dataset_list;

	DatasetMaskProcessor dataset_mask_processor(QLatin1String("DatasetMaskProcessor - name for logging"),
												cp.pLog, cp.dh);
	FileMaskProcessor file_mask_processor(QLatin1String("FileMaskProcessor - name for logging"), cp.pLog);

	dataset_mask_processor.processString("*", &dataset_list);
	file_mask_processor.processString("*.*", &files_list);


	_sd->toFile();
	
	_app->startApp();

	_sd = std::make_unique<SDataset>(SDataset::fromFile());

	_sd->toFlow(cp);

	_sd->clearSwap();
}

void RunAppModule::logParameters () const
{
	logReport(tr("Parameters") % u":\n" % m_parameters.makeLogText());
}

void RunAppModule::substituteReplicaInTemplatizedParameter(COMMON_PARAM &cp)
{
	ReplicaVarSubstitutor replica_substitutor(getTitle(), cp);

	QString substituted_string;

	ReplicaExpr::Validity substitution_status =
		replica_substitutor.substitute("Application Name"_latin1, m_parameters.getApplicationName(),
			&substituted_string);
	if (substitution_status == ReplicaExpr::Validity::Invalid)
		throw Util::Exception(tr("%1 replica variable substitution failed").arg(getTitle()));
	else if (substitution_status == ReplicaExpr::Validity::Valid)
		m_parameters.setApplicationName(substituted_string);


	substitution_status =
		replica_substitutor.substitute("Application Parameters"_latin1,
			ParametersString(m_parameters.getApplicationParameters()).format(), &substituted_string);
	if (substitution_status == ReplicaExpr::Validity::Invalid)
		throw Util::Exception(tr("%1 replica variable substitution failed").arg(getTitle()));
	else if (substitution_status == ReplicaExpr::Validity::Valid)
		m_parameters.setApplicationParameters(substituted_string);
}

Swap::Swap(sfs::path storageDir) :
	_storageDir(storageDir.make_preferred()),
	_tracesIO_fn((_storageDir / "traces").string()),
	_headersIO_fn((_storageDir / "headers").string()),
	_headerNamesO_fn((_storageDir / "headerNames.txt").string())
{
}

void Swap::open_out() 
{
	_tracesOutput.open(_tracesIO_fn, std::ios::out | std::ios::binary);
	_headersOutput.open(_headersIO_fn, std::ios::out | std::ios::binary);
	_headerNamesOutput.open(_headerNamesO_fn, std::ios::out);
}

void Swap::close_out() 
{
	if (_tracesOutput.is_open())
		_tracesOutput.close();
	if (_headersOutput.is_open())
		_headersOutput.close();
	if (_headerNamesOutput.is_open())
		_headerNamesOutput.close();
}

void Swap::open_in() 
{
	_tracesInput.open(_tracesIO_fn, std::ios::in | std::ios::binary);
	_headersInput.open(_headersIO_fn, std::ios::in | std::ios::binary);
}

void Swap::close_in() 
{
	if (_tracesInput.is_open())
		_tracesInput.close();
	if (_headersInput.is_open())
		_headersInput.close();
}

SDataset::SDataset(unsigned int traceCount, unsigned int sampleCount, unsigned int headerCount, sfs::path storage_dir)
	: Swap(storage_dir),
	_traceCount(traceCount),
	_sampleCount(sampleCount),
	_headerCount(headerCount),
	_headerNames(std::vector<std::string>(_headerCount, "")),
	_traces(std::vector<float>(_traceCount * _sampleCount, 0.0)),
	_headers(std::vector<double>(_traceCount * _headerCount, 0.0))
{
}

SDataset::SDataset(sfs::path storageDir)
	: Swap(storageDir),
	_traceCount(0),
	_sampleCount(0),
	_headerCount(0),
	_headerNames(std::vector<std::string>(0, "")),
	_traces(std::vector<float>(0, 0.0)),
	_headers(std::vector<double>(0, 0.0))
{
}

sfs::path SDataset::getcwd()
{
	return _storageDir;
}

SDataset SDataset::fromFlow(COMMON_PARAM& cp, unsigned int traceCount, unsigned int sampleCount,
							unsigned int headerCount, sfs::path storageDir)
{
	SDataset sd = (traceCount == 0 && sampleCount == 0 && headerCount == 0)
		? SDataset(cp.n_tr, cp.np, cp.nf, storageDir)
		: SDataset(traceCount, sampleCount, headerCount, storageDir);

	if (sd._traces.data() != nullptr && sd._headers.data() != nullptr)
	{
		for (unsigned int i = 0; i < sd._traceCount; i++)
		{
			memcpy(sd._traces.data() + i * sd._sampleCount, cp.tr[i].d, sizeof(float) * sd._sampleCount);
			for (unsigned int j = 0; j < sd._headerCount; j++)
				sd._headers[i * sd._headerCount + j] = cp.getHeaderValue(j, i);
		}

		for (unsigned int i = 0; i < sd._headerCount; i++)
			sd._headerNames[i] = std::string(reinterpret_cast<char*>(cp.de[i].name));

	}
	else if (sd._traces.data() != nullptr)
		for (unsigned int i = 0; i < sd._traceCount; i++)
			memcpy(sd._traces.data() + i * sd._sampleCount, cp.tr[i].d, sizeof(float) * sd._sampleCount);
	else if (sd._headers.data() != nullptr)
	{
		for (unsigned int i = 0; i < sd._traceCount; i++)
			for (unsigned int j = 0; j < sd._headerCount; j++)
				sd._headers[i * sd._headerCount + j] = cp.getHeaderValue(j, i);

		for (unsigned int i = 0; i < sd._headerCount; i++)
			sd._headerNames[i] = std::string(reinterpret_cast<char*>(cp.de[i].name));
	}

	return std::move(sd);
}

SDataset SDataset::fromFile(sfs::path storageDir)
{
	SDataset sd(storageDir);
	bool exception_caught = true;
	if (!sd._storageDir.empty())
	{
		try {
			sd.open_in();
			
			unsigned int traceCount1, traceCount2, sampleCount, headerCount;
			sd._tracesInput.read(reinterpret_cast<char*>(&traceCount1), sizeof(int));
			sd._tracesInput.read(reinterpret_cast<char*>(&sampleCount), sizeof(int));
			sd._headersInput.read(reinterpret_cast<char*>(&traceCount2), sizeof(int));
			sd._headersInput.read(reinterpret_cast<char*>(&headerCount), sizeof(int));

			if (traceCount1 != traceCount2)
				throw std::exception("Error - n_tr in header and trace files are different.");
			sd.close_in();

			sd = SDataset(traceCount1, sampleCount, headerCount, storageDir);
			sd.open_in();
			if (sd._traces.data() != nullptr)
			{
				sd._tracesInput.seekg(2 * sizeof(int), std::ios::beg);
				sd._tracesInput.read(reinterpret_cast<char*>(sd._traces.data()),
					sizeof(float) * traceCount1 * sampleCount);
			}
			if (sd._headers.data() != nullptr)
			{
				sd._headersInput.seekg(2 * sizeof(int), std::ios::beg);
				sd._headersInput.read(reinterpret_cast<char*>(sd._headers.data()),
					sizeof(double) * traceCount1 * headerCount);
			}
			
			exception_caught = false;
		}
		catch (...)
		{
			sd.close_in();
			throw;
		}
		if (!exception_caught)
			sd.close_in();
	}

	return std::move(sd);
}

void SDataset::toFile()
{
	bool exception_caught = true;
	try
	{
		open_out();

		_tracesOutput.write(reinterpret_cast<char*>(&_traceCount), sizeof(_traceCount));
		_tracesOutput.write(reinterpret_cast<char*>(&_sampleCount), sizeof(_sampleCount));
		_tracesOutput.write(reinterpret_cast<char*>(_traces.data()), sizeof(float) * _traces.size());

		_headersOutput.write(reinterpret_cast<char*>(&_traceCount), sizeof(_traceCount));
		_headersOutput.write(reinterpret_cast<char*>(&_headerCount), sizeof(_headerCount));
		_headersOutput.write(reinterpret_cast<char*>(_headers.data()), sizeof(double) * _headers.size());

		for (unsigned long i = 0; i < _headerCount; i++)
			_headerNamesOutput << _headerNames[i].c_str() << std::endl;
		exception_caught = false;
	}
	catch (...)
	{
		close_out();
		throw;
	}
	if (!exception_caught)
		close_out();
}

void SDataset::toFlow(COMMON_PARAM &cp)
{
	int dt_index = cp.getHeaderIndex("dt");
	cp.np = _sampleCount;
	cp.dt = _headers[dt_index];
	cp.dx = 1;
	cp.nsf_ind = 0;
	cp.n_tr = _traceCount;
	auto stream_id = cp.CreateDataOutput(&cp, DO_SINGLE | DO_NEW, 0);
	
	OUTTRACE tr;
	tr.np = cp.np;
	for (unsigned int i = 0; i < _traceCount; i++)
	{
		tr.d = _traces.data() + i * _sampleCount;
		tr.h = _headers.data() + i * _headerCount;
		cp.TraceOutput(stream_id, &tr);
	}
}

void SDataset::clearSwap() 
{
	if (sfs::exists(_tracesIO_fn))
		std::remove(_tracesIO_fn.c_str());
	if (sfs::exists(_headersIO_fn))
		std::remove(_headersIO_fn.c_str());
	if (sfs::exists(_headerNamesO_fn))
		std::remove(_headerNamesO_fn.c_str());
}

ParametersString::ParametersString(const QString& text) : QString(text)
{
}

QString ParametersString::format() 
{
	QRegExp regex("(\\s|^)-{2}([A-Za-z_]+\\w*)\\s*=\\s*((\\{@\\w+\\})|(\\S+))(\\s|$)");
	QRegExp whitespace("\\s");
	QRegExp equal("\\s*=\\s*");

	QString formatedStr = "";
	QString tmp;
	int p = 0, q = 0;
	int length = 0;
	int index = regex.indexIn(*this);
	while (index >= 0)
	{
		length = regex.matchedLength();
		tmp = this->mid(index, length);
		for (int i = 0; i < tmp.length(); i++)
		{
			if (!whitespace.exactMatch(*(tmp.begin() + i)))
			{
				p = i;
				break;
			}
		}
		for (int i = tmp.length() - 1; i >= 0; i--)
		{
			if (!whitespace.exactMatch(*(tmp.begin() + i)))
			{
				q = i;
				break;
			}
		}
		formatedStr += " " + tmp.mid(p, q - p + 1).replace(equal,"=");
		if (whitespace.exactMatch(*(this->begin() + index + length - 1)))
			index--;
		index = regex.indexIn(*this, index + length);
	}
	return formatedStr;
}