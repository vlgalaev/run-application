#include "RunAppAPI.h"
#include "RunAppModule.h"
#include "RunAppParameters.h"
#include "RunAppParametersDialog.h"

#include "ConfControl/ConfControl.h"
#include "DBWrapper/Path.h"
#include "Debug/Assert.h"
#include "Exec.h"
#include "Qt/RemoteQtAPI.h"
#include "RdxModuleDefault.h"
#include "ReLogManager/ReLoggingClass.h"
#include "ReMisc/Exception.h"
#include "ReplicaTable/ReplicaExpr.h"
#include "res/res.h"
#include "ReSmartArrays/ReAutoDMPath.h"
#include "ReStrConvert.h"
#include "../../SYSMOD/DMPFlowDef/DMPCopyLevel.h"

#include <QCoreApplication>
#include <QMutex>

#include <memory>

namespace
{
EXTRA_PARAM *g_p_temp_extra_parameters_holder = nullptr;	// buffer for storing pointer to EXTRA_PARAM structure
															// between calls to ExtraParameters() and EditParameters()
QMutex		g_extra_parameters_guard; // mutex for synchronization of access to parameters' editing functions
}

#ifdef Q_OS_WIN
STDAPI DllRegisterServer ()
{
	return RegisterModule(DLL_DESCRIPTION); // see 'DLL_DESCRIPTION' in project's macros
}
#endif

void RDX_MODULE_CALL ExtraParameters (EXTRA_PARAM *p_extra_parameters)
{
	g_extra_parameters_guard.lock();

	Q_ASSERT(g_p_temp_extra_parameters_holder == nullptr);
	g_p_temp_extra_parameters_holder = p_extra_parameters;
}

inline bool execDialogToEditParameters (
	void *p_parent_window, void *p_packed_parameters,
	int *p_packed_parameters_size_in_bytes, const EXTRA_PARAM *const p_extra_parameters)
{
	if ((p_packed_parameters == nullptr) || (p_packed_parameters_size_in_bytes == nullptr))
		return false;

	if (p_extra_parameters == nullptr)
		return false;

	try
	{
		auto parameters_ptr = 
			::unpackModuleParameters<RunAppParameters>(
				p_packed_parameters, *p_packed_parameters_size_in_bytes);
		ASSERT_NOTNULL(parameters_ptr);

		auto dialog_ptr = std::make_unique<RunAppParametersDialog>(*p_extra_parameters);
		ASSERT_NOTNULL(dialog_ptr);

		dialog_ptr->setParameters(*parameters_ptr);
		dialog_ptr->setWindowTitle(RunAppModule::getTitle());

		if (remoteqt::setTransientParent(dialog_ptr.get(), p_parent_window))
		{
			if (dialog_ptr->exec() == QDialog::Accepted)
			{
				const QByteArray packed_result_parameters = dialog_ptr->getParameters().pack();

				const std::size_t n_max_bytes_to_use =
					static_cast<std::size_t>(std::min(packed_result_parameters.size(), PARAMBUFSIZE));

				std::memcpy(p_packed_parameters, packed_result_parameters.data(), n_max_bytes_to_use);
				*p_packed_parameters_size_in_bytes = static_cast<int>(n_max_bytes_to_use);

				return true;
			}
		}
	}
	catch (...)
	{}

	return false;
}

int RDX_MODULE_CALL editParameters (
	void *p_parent_window, int *p_packed_parameters_size_in_bytes, void *p_packed_parameters)
{
	ASSERT_NOTNULL(p_packed_parameters_size_in_bytes, p_packed_parameters);

	if ((p_packed_parameters_size_in_bytes == nullptr) || (p_packed_parameters == nullptr))
		return 0;

	if (g_extra_parameters_guard.tryLock()) // ExtraParameters() was not called before
	{
		g_p_temp_extra_parameters_holder = nullptr;
		g_extra_parameters_guard.unlock();
		return 0;
	}

	const EXTRA_PARAM *const p_extra_parameters = g_p_temp_extra_parameters_holder;
	g_p_temp_extra_parameters_holder = nullptr;
	g_extra_parameters_guard.unlock();

	if (p_extra_parameters == nullptr)
		return 0;

	bool is_dialog_accepted =
		remoteqt::invokeInGuiThread(
			::execDialogToEditParameters, p_parent_window, p_packed_parameters,
			p_packed_parameters_size_in_bytes, p_extra_parameters);

	return (is_dialog_accepted ? 1 : 0);
}

void RDX_MODULE_CALL GetParamPathList (std::vector<std::string> *p_encoded_paths, char *p_packed_parameters)
{
	ASSERT_NOTNULL(p_encoded_paths);

	auto parameters_ptr = 
		::unpackModuleParameters<RunAppParameters>(p_packed_parameters, PARAMBUFSIZE);
	ASSERT_NOTNULL(parameters_ptr);

//	p_encoded_paths->push_back(parameters_ptr->getPickPath().encoded.toPlainPath().toStdString());
}

void RDX_MODULE_CALL SetParamPathList (
	DM_DATAHANDLE dh, const std::vector<std::string> *p_encoded_paths, char *p_packed_parameters)
{
	ASSERT_NOTNULL(p_encoded_paths, p_packed_parameters);

	if (p_encoded_paths->empty())
		return;

	ReAutoDMPath path_guard(dh);

	auto parameters_ptr = 
		::unpackModuleParameters<RunAppParameters>(p_packed_parameters, PARAMBUFSIZE);
	ASSERT_NOTNULL(parameters_ptr);

//	char plain_encoded_pick_path[MAX_DB_PATH_LEN] = { 0 };
//	char plain_readable_pick_path[MAX_DB_NAME_LEN] = { 0 };

//	auto format = database::ReadableLocationPath::PlainPathFormat::SystemNative;
//	copyQStringToCharArray(parameters_ptr->getPickPath().readable.toPlainPath(format), plain_readable_pick_path);

//	GetNewPathName(
//		dh, plain_encoded_pick_path, int(std::size(plain_encoded_pick_path)),
//		plain_readable_pick_path, int(std::size(plain_readable_pick_path)), p_encoded_paths->front());

//	database::ObjectPath new_pick_path;
//	new_pick_path.encoded.setPlainPath(DB_ENCODED_NAME_FROM_ASCII(plain_encoded_pick_path));
//	new_pick_path.readable.setPlainPath(toQtString(plain_readable_pick_path));

//	parameters_ptr->setPickPath(new_pick_path);

	std::memset(p_packed_parameters, 0, PARAMBUFSIZE);

	QByteArray new_packed_array = parameters_ptr->pack();
	std::memcpy(p_packed_parameters, new_packed_array.data(), std::min(new_packed_array.size(), PARAMBUFSIZE));
}

char* RDX_MODULE_CALL getSuffix (int packed_parameters_size_in_bytes, void *p_packed_parameters)
{
	ASSERT_NOTNULL(p_packed_parameters);

	static char suffix_holder[MAX_MODULE_TITLE_SUFFIX_LENGTH] = {};

	auto parameters_ptr = 
		::unpackModuleParameters<RunAppParameters>(p_packed_parameters, packed_parameters_size_in_bytes);
	ASSERT_NOTNULL(parameters_ptr);

	QString suffix_string = QLatin1String(" <- "); //- when input parameters are more important to display
						   // = QLatin1String(" -> "); - when output parameters are more important to display
	suffix_string += parameters_ptr->getApplicationName();

	std::strncpy(suffix_holder, qUtf8Printable(suffix_string), std::size(suffix_holder) - 1);

	return suffix_holder;
}

unsigned long RDX_MODULE_CALL runRunApp (COMMON_PARAM *p_cp, const char *p_packed_parameters)
{
	ASSERT_NOTNULL(p_cp, p_packed_parameters);

	unsigned long result = MP_FATALERR;
	try
	{
		const std::unique_ptr<RunAppParameters> parameters_ptr =
			::unpackModuleParameters<RunAppParameters>(p_packed_parameters, PARAMBUFSIZE);
		ASSERT_NOTNULL(parameters_ptr);

		auto module_ptr = p_cp->popModulePreservedDataPtr<RunAppModule>();
		
		if (module_ptr == nullptr) // first frame
		{
			IReLog::logReport<RunAppModule>(
				RunAppModule::tr("Started..."), p_cp->pLog);
			module_ptr = std::make_unique<RunAppModule>(*p_cp, *parameters_ptr);
			ASSERT_NOTNULL(module_ptr);
			module_ptr->logParameters();
		}
		
		try
		{
			module_ptr->processFrame(*p_cp);
		}
		catch (Util::UserInterruptException &)
		{
			module_ptr->stop();
		}
		p_cp->pushModulePreservedDataPtr(std::move(module_ptr));

		result = MP_OK;

	}
	catch (Util::UserInterruptException &)
	{
		result = MP_TERMINATED;
	}
	catch (std::exception &exception)
	{
		IReLog::logError<RunAppModule>(toQtString(exception.what()), p_cp->pLog);
	}
	catch (...)
	{
		IReLog::logError<RunAppModule>(
			RunAppModule::tr("Unknown exception caught"), p_cp->pLog);
	}
	IReLog::logFrameResult<RunAppModule>(result, p_cp->pLog);

	return result;
}

unsigned long RDX_MODULE_CALL SignalTerminating (COMMON_PARAM *p_cp, void *p_packed_parameters, ULONG termination_code)
{
	ASSERT_NOTNULL(p_cp);
	Q_UNUSED(p_packed_parameters);

	auto module_ptr = p_cp->popModulePreservedDataPtr<RunAppModule>();

	if (termination_code == MP_OK)
		IReLog::logModuleResult<RunAppModule>(termination_code, p_cp->pLog);

	return termination_code;
}

int RDX_MODULE_CALL isModuleTemplatized (const char *p_packed_parameters)
{
	auto parameters_ptr = unpackModuleParameters<RunAppParameters>(p_packed_parameters, PARAMBUFSIZE);
	ASSERT_NOTNULL(parameters_ptr);	

	bool isTemplatized = false;
	if (ReplicaExpr::checkIfExprListHasVariable({ parameters_ptr->getApplicationName() }) |
		ReplicaExpr::checkIfExprListHasVariable({ ParametersString(
			parameters_ptr->getApplicationParameters()).format() }))
		isTemplatized = true;

	return isTemplatized;
}