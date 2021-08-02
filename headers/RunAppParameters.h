#pragma once

#include "ParamStructVersion.h"

#include <QString>

#include <cstddef>

class QByteArray;

class RunAppParameters
{
public:
	//! @brief Current version of the parameter structure
	inline static ParamStructVersionType getCurrentVersion ();

public:
	//! @brief   Packs members of the structure to raw sequence of bytes and returns resulting sequence
	//! @retutn  Resulting sequence  
	//! @warning It can throw only std::bad_alloc
	QByteArray pack () const;

	//! @brief       Reads members of the structure from raw sequence of bytes;
	//! @description Unaffected members (if any) will be initialized with default (!) values
	//! @return      Number of read (used) bytes  
	std::size_t unpackFrom (const void *p_raw_bytes, const std::size_t n_bytes);

	//! @brief  Generates a text with parameters for output to the log.
	//! @return Text with parameters divided by 'line_separator'
	QString makeLogText (const QChar line_separator = QLatin1Char('\n')) const;

	const QString& getApplicationName() const;
	void setApplicationName(const QString &ApplicationName);

	const QString& getApplicationParameters() const;
	void setApplicationParameters(const QString &ApplicationParameters);

private:
	static const ParamStructVersionType m_first_version = ParamStructVersion<28, 7, 2020>::value;
	static const ParamStructVersionType m_current_version = RunAppParameters::m_first_version;

	QString ApplicationName = "";
	QString ApplicationParameters = "";

};

// ---------- Implementation ----------

ParamStructVersionType RunAppParameters::getCurrentVersion ()
{
	return m_current_version;
}
