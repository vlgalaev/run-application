#pragma once

#include "RunAppParameters.h"

#include "ReLogManager/ReLoggingClass.h"

#include <QCoreApplication>


struct COMMON_PARAM;

class QString;

class SDataset;

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

class SDataset
{
private:
	unsigned int m_traceCount;
	unsigned int m_sampleCount;
	unsigned int m_headerCount;

	std::vector<std::string> m_headerNames;
	float *m_traces;
	double *m_headers;

private:
	template<typename T>
	inline static T* memoryAlloc(unsigned int count);

public:
	SDataset(unsigned int traceCount=0, unsigned int sampleCount=0, unsigned int headerCount=0); // Default Constructor
	SDataset(const SDataset& other); // Copy constructor
	SDataset(SDataset&& other) noexcept; // Move constructor
	SDataset& operator=(const SDataset& other); // Copy assigment operator
	SDataset& operator=(SDataset&& other) noexcept; // Move assigment operator
	~SDataset(); // Destructor

	void toFlow(COMMON_PARAM &cp);
	static SDataset fromFlow(COMMON_PARAM &cp, unsigned int traceCount=0, unsigned int sampleCount=0, unsigned int headerCount=0);

	void toFile(std::ofstream&, std::ofstream&, std::ofstream&);
	static SDataset fromFile(std::ifstream&, std::ifstream&);
};

// ----- Implementation -----

bool RunAppModule::areParametersValid () const
{
	return RunAppModule::validateParameters(m_parameters);
}

template<typename T>
T* SDataset::memoryAlloc(unsigned int count)
{
	T *ptr = nullptr;
	if (count == 0)
		return ptr;
	else
		ptr = new T[count]();
	return ptr;
}