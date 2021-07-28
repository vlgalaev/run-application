#include "RunAppParameters.h"
#include "RunAppParametersDialog.h"

#include "Debug/Assert.h"
#include "Exec.h"
#include "ReMisc/QDataStreamIO.h"

#include <QByteArray>
#include <QDataStream>
#include <QStringBuilder>

#include <algorithm>
#include <limits>

QByteArray RunAppParameters::pack () const
{
	QByteArray result;
	QDataStream data_stream(&result, QIODevice::WriteOnly);

	data_stream << RunAppParameters::getCurrentVersion();
	data_stream << RunAppParameters::ApplicationName;

	Q_ASSERT(result.size() <= PARAMBUFSIZE); // if this fires, then you should implement compressed packing

	return result;
}

std::size_t RunAppParameters::unpackFrom (const void *p_raw_bytes, const std::size_t n_bytes)
{
	ParamStructVersionType version = 0u;

	if ((p_raw_bytes == nullptr) || (n_bytes <= sizeof(version)))
		return 0u;

	const int n_max_bytes = 
		static_cast<int>(std::min(n_bytes, static_cast<std::size_t>(std::numeric_limits<int>::max())));
	QDataStream data_stream(
		QByteArray::fromRawData(static_cast<const char*>(p_raw_bytes), n_max_bytes)); // there is NO deep copy

	ASSERT_NOTNULL(data_stream.device());

	data_stream >> version;
	
	if (version != RunAppParameters::m_current_version)	// this means older version of parameters
		return 0u;											// process the case instead of returning zero
	data_stream >> RunAppParameters::ApplicationName;

	return static_cast<std::size_t>(data_stream.device()->pos());
}

QString RunAppParameters::makeLogText (const QChar line_separator) const
{
	Q_UNUSED(line_separator);

	// Note: use "RunAppModule::tr()" or "RunAppParametersDialog::tr()" context to translate parameters
	using Dialog = RunAppParametersDialog;

	QString log_text = u'\t' % Dialog::tr("ApplicationName") % u": " % QString(RunAppParameters::getApplicationName());
		//u'\t' % Dialog::tr("_Parameter_1_name_") % u": " % QString("_parameter_1_value_") % line_separator %
		//u'\t' % Dialog::tr("_Parameter_2_name_") % u": " % QString("_parameter_2_value_");

	return log_text;
}

const QString& RunAppParameters::getApplicationName() const
{
	return ApplicationName;
}

void RunAppParameters::setApplicationName(const QString &ApplicationName)
{
	this->ApplicationName = ApplicationName;
}
