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

#include <fstream>
#include <exception>

#include <filesystem>
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
}

void RunAppModule::processFrame(COMMON_PARAM &cp)
{
	MODULE_VERIFY_CONFIGURATION(cp.dwConfiguration);

	QStringList files_list;
	QStringList dataset_list;

	DatasetMaskProcessor dataset_mask_processor(QLatin1String("DatasetMaskProcessor - name for logging"), cp.pLog, cp.dh);
	FileMaskProcessor file_mask_processor(QLatin1String("FileMaskProcessor - name for logging"), cp.pLog);

	dataset_mask_processor.processString("*", &dataset_list);
	file_mask_processor.processString("*.*", &files_list);

	QString applicationName = m_parameters.getApplicationName();
	std::wstring applicationName_wstring = applicationName.toStdWString();
	std::vector<wchar_t> applicationName_wcharar(applicationName_wstring.begin(),
		applicationName_wstring.end());
	applicationName_wcharar.push_back('\0');
	sfs::path applicationAbsName = sfs::path(applicationName_wstring).make_preferred();
	std::string traceFileName = (applicationAbsName.parent_path() / "traces").string();
	std::string headerFileName = (applicationAbsName.parent_path() / "headers").string();
	std::string headerNamesFileName = (applicationAbsName.parent_path() / "headerNames.txt").string();


	//***** Write amplitudes and headers to file
	SDataset sdataset(SDataset::fromFlow(cp));
	std::ofstream traceOutput(traceFileName, std::ios::binary);
	std::ofstream headerOutput(headerFileName, std::ios::binary);
	std::ofstream headerNamesOutput(headerNamesFileName);

	if (traceOutput && headerOutput && headerNamesOutput)
	{
		sdataset.toFile(traceOutput, headerOutput, headerNamesOutput);
		traceOutput.close();
		headerOutput.close();
		headerNamesOutput.close();
	}
	else if (traceOutput.is_open())
		traceOutput.close();
	else if (headerOutput.is_open())
		headerOutput.close();
	else if (headerNamesOutput.is_open())
		headerNamesOutput.close();

	//***** Execute the application
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, // No module name (use command line)
		&applicationName_wcharar[0],        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		applicationAbsName.parent_path().c_str(),           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		unsigned long error_code = GetLastError();
		if (error_code == ERROR_ACCESS_DENIED)
			throw std::exception(("Access is denied. Error code is: " + std::to_string(GetLastError())).c_str());
		else if (error_code == ERROR_FILE_NOT_FOUND)
			throw std::exception(("File not found. Error code is: " + std::to_string(GetLastError())).c_str());
		else
			throw std::exception(("The application execution error code is: " + std::to_string(GetLastError())).c_str());
	}


	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);


	unsigned long exit_code = 0;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	if (exit_code == 1)
		throw std::exception((
			"Unable to read the app parameters from file or read/write rdx dataset swap files. Exit code is: " +
			std::to_string(exit_code)).c_str());
	if (exit_code == 2)
		throw std::exception((
			"Not enought memory. Exit code is: " +
			std::to_string(exit_code)).c_str());
	if (exit_code == 101)
		throw std::exception((
			"The content of the parameters file is invalid. Exit code is: " +
			std::to_string(exit_code)).c_str());
	if (exit_code == 102)
		throw std::exception((
			"Unknown exception during the calculations. Exit code is: " +
			std::to_string(exit_code)).c_str());
	else if (exit_code)
		throw std::exception(("Fully unknown exception. Exit code is: " + std::to_string(exit_code)).c_str());


	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);


	//***** Read amplitudes and headers from file
	std::ifstream traceInput(traceFileName, std::ios::binary);
	std::ifstream headerInput(headerFileName, std::ios::binary);

	if (traceInput && headerInput)
	{
		try
		{
			sdataset = SDataset::fromFile(traceInput, headerInput);
		}
		catch (...)
		{
			traceInput.close();
			headerInput.close();
			throw;
		}
		traceInput.close();
		headerInput.close();
	}
	else if (traceInput)
		traceInput.close();
	else if (headerInput)
		headerInput.close();
	else
		cp.pLog->Log("FileRead: ", "Error", MessageType::Report);

	//***** Flow SDataset
	sdataset.toFlow(cp);

	//***** Remove files
	if (sfs::exists(traceFileName))
		std::remove(traceFileName.c_str());
	if (sfs::exists(headerFileName))
		std::remove(headerFileName.c_str());
	if (sfs::exists(headerNamesFileName))
		std::remove(headerNamesFileName.c_str());
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
		replica_substitutor.substitute("Application Name"_latin1, m_parameters.getApplicationName(), &substituted_string);
	if (substitution_status == ReplicaExpr::Validity::Invalid)
		throw Util::Exception(tr("%1 replica variable substitution failed").arg(getTitle()));
	else if (substitution_status == ReplicaExpr::Validity::Valid)
		m_parameters.setApplicationName(substituted_string);
}

SDataset SDataset::fromFlow(COMMON_PARAM& cp, unsigned int traceCount, unsigned int sampleCount, unsigned int headerCount)
{
	SDataset sd = (traceCount == 0 && sampleCount == 0 && headerCount == 0)
		? SDataset(cp.n_tr, cp.np, cp.nf)
		: SDataset(traceCount, sampleCount, headerCount);

	if (sd.m_traces != nullptr && sd.m_headers != nullptr)
	{
		for (unsigned int i = 0; i < sd.m_traceCount; i++)
		{
			memcpy(sd.m_traces + i * sd.m_sampleCount, cp.tr[i].d, sizeof(float) * sd.m_sampleCount);
			for (unsigned int j = 0; j < sd.m_headerCount; j++)
				sd.m_headers[i * sd.m_headerCount + j] = cp.getHeaderValue(j, i);
		}

		for (unsigned int i = 0; i < sd.m_headerCount; i++)
			sd.m_headerNames[i] = std::string(reinterpret_cast<char*>(cp.de[i].name));

	}
	else if (sd.m_traces != nullptr)
		for (unsigned int i = 0; i < sd.m_traceCount; i++)
			memcpy(sd.m_traces + i * sd.m_sampleCount, cp.tr[i].d, sizeof(float) * sd.m_sampleCount);
	else if (sd.m_headers != nullptr)
	{
		for (unsigned int i = 0; i < sd.m_traceCount; i++)
			for (unsigned int j = 0; j < sd.m_headerCount; j++)
				sd.m_headers[i * sd.m_headerCount + j] = cp.getHeaderValue(j, i);

		for (unsigned int i = 0; i < sd.m_headerCount; i++)
			sd.m_headerNames[i] = std::string(reinterpret_cast<char*>(cp.de[i].name));
	}

	return std::move(sd);
}

SDataset SDataset::fromFile(std::ifstream& traceInput, std::ifstream& headerInput)
{
	unsigned int traceCount1, traceCount2, sampleCount, headerCount;
	traceInput.read(reinterpret_cast<char*>(&traceCount1), sizeof(int));
	traceInput.read(reinterpret_cast<char*>(&sampleCount), sizeof(int));
	headerInput.read(reinterpret_cast<char*>(&traceCount2), sizeof(int));
	headerInput.read(reinterpret_cast<char*>(&headerCount), sizeof(int));

	if (traceCount1 != traceCount2)
		throw std::exception("Error - n_tr in header and trace files are different.");

	SDataset sd = SDataset(traceCount1, sampleCount, headerCount);
	if (sd.m_traces != nullptr)
		traceInput.read(reinterpret_cast<char*>(sd.m_traces), sizeof(float) * traceCount1 * sampleCount);
	if (sd.m_headers != nullptr)
		headerInput.read(reinterpret_cast<char*>(sd.m_headers), sizeof(double) * traceCount1 * headerCount);

	return std::move(sd);
}

// Default Constructor
SDataset::SDataset(unsigned int traceCount, unsigned int sampleCount, unsigned int headerCount)
	: m_traceCount(traceCount),
	m_sampleCount(sampleCount),
	m_headerCount(headerCount),
	m_traces(nullptr),
	m_headers(nullptr)
{
	try
	{
		m_traces = SDataset::memoryAlloc<float>(static_cast<unsigned int>(traceCount * sampleCount));
		m_headers = SDataset::memoryAlloc<double>(static_cast<unsigned int>(traceCount * headerCount));
		std::string empty_str;
		empty_str.reserve(65);
		m_headerNames = std::vector<std::string>(headerCount, empty_str);
	}
	catch (...)
	{
		if (m_traces != nullptr)
			delete[] m_traces;
		if (m_headers != nullptr)
			delete[] m_headers;
		throw;
	}
}

// Copy constructor
SDataset::SDataset(const SDataset& other) : SDataset(other.m_traceCount, other.m_sampleCount, other.m_headerCount)
{
	if (m_traces != nullptr)
		memcpy(m_traces, other.m_traces, sizeof(float) * m_traceCount * m_sampleCount);
	if (m_headers != nullptr)
		memcpy(m_headers, other.m_headers, sizeof(double) * m_traceCount * m_headerCount);
	for (unsigned long i = 0; i < other.m_headerCount; i++)
		m_headerNames[i] = other.m_headerNames[i];
}

// Move constructor
SDataset::SDataset(SDataset && other) noexcept 
	: m_traceCount(other.m_traceCount),
	m_sampleCount(other.m_sampleCount),
	m_headerCount(other.m_headerCount),
	m_headerNames(other.m_headerNames),
	m_traces(other.m_traces),
	m_headers(other.m_headers)
{
	other.m_traceCount = 0;
	other.m_sampleCount = 0;
	other.m_headerCount = 0;
	other.m_headerNames = std::vector<std::string>();
	other.m_traces = nullptr;
	other.m_headers = nullptr;
}

// Copy assignment operator
SDataset& SDataset::operator=(const SDataset& other)
{
	if (this != &other)
	{
		float *m_traces_tmpptr = m_traces;
		double *m_headers_tmpptr = m_headers;
		m_traces = nullptr;
		m_headers = nullptr;
		try
		{
			m_traces = SDataset::memoryAlloc<float>(other.m_traceCount * other.m_sampleCount);
			m_headers = SDataset::memoryAlloc<double>(other.m_traceCount * other.m_headerCount);
		}
		catch (...)
		{
			if (m_traces != nullptr)
				delete[] m_traces;
			if (m_headers != nullptr)
				delete[] m_headers;
			m_traces = m_traces_tmpptr;
			m_headers = m_headers_tmpptr;
			throw;
		}
		if (m_traces != nullptr)
			memcpy(m_traces, other.m_traces, sizeof(float) * other.m_traceCount * other.m_sampleCount);
		if (m_headers != nullptr)
			memcpy(m_headers, other.m_headers, sizeof(double) * other.m_traceCount * other.m_headerCount);
		if (m_traces_tmpptr != nullptr)
			delete[] m_traces_tmpptr;
		if (m_headers_tmpptr != nullptr)
			delete[] m_headers_tmpptr;
		m_traceCount = other.m_traceCount;
		m_sampleCount = other.m_sampleCount;
		m_headerCount = other.m_headerCount;
		m_headerNames = other.m_headerNames;
	}
	return *this;
}

// Move assignment operator
SDataset& SDataset::operator=(SDataset&& other) noexcept
{
	if (this != &other)
	{
		if (m_traces != nullptr)
			delete[] m_traces;
		if (m_headers != nullptr)
			delete[] m_headers;

		m_traceCount = other.m_traceCount;
		m_sampleCount = other.m_sampleCount;
		m_headerCount = other.m_headerCount;
		m_headerNames = other.m_headerNames;
		m_traces = other.m_traces;
		m_headers = other.m_headers;

		other.m_traceCount = 0;
		other.m_sampleCount = 0;
		other.m_headerCount = 0;
		other.m_headerNames = std::vector<std::string>();
		other.m_traces = nullptr;
		other.m_headers = nullptr;
	}
	return *this;
}

// Destructor
SDataset::~SDataset()
{
	if (m_traces != nullptr)
		delete[] m_traces;
	if (m_headers != nullptr)
		delete[] m_headers;
}

void SDataset::toFile(std::ofstream& traceOutput, std::ofstream& headerOutput,
	std::ofstream& headerNamesOutput)
{
	traceOutput.write(reinterpret_cast<char*>(&m_traceCount), sizeof(m_traceCount));
	traceOutput.write(reinterpret_cast<char*>(&m_sampleCount), sizeof(m_sampleCount));
	traceOutput.write(reinterpret_cast<char*>(m_traces), sizeof(float) * m_traceCount * m_sampleCount);

	headerOutput.write(reinterpret_cast<char*>(&m_traceCount), sizeof(m_traceCount));
	headerOutput.write(reinterpret_cast<char*>(&m_headerCount), sizeof(m_headerCount));
	headerOutput.write(reinterpret_cast<char*>(m_headers), sizeof(double) * m_traceCount * m_headerCount);

	for (unsigned long i = 0; i < m_headerCount; i++)
		headerNamesOutput << m_headerNames[i].c_str() << std::endl;
}

void SDataset::toFlow(COMMON_PARAM &cp)
{
	auto stream_id = cp.CreateDataOutput(&cp, DO_SINGLE | DO_NEW, 0);
	cp.np = m_sampleCount;

	OUTTRACE tr;
	tr.np = cp.np;
	for (unsigned int i = 0; i < m_traceCount; i++)
	{
		tr.d = m_traces + i * m_sampleCount;
		tr.h = m_headers + i * m_headerCount;
		cp.TraceOutput(stream_id, &tr);
	}
}