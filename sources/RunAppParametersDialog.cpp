#include "RunAppParametersDialog.h"
#include "ui_RunAppParametersDialog.h"

#include "DBNavigatorQt/SelectObjectDialog.h"
#include "DBWrapper/Database.h"
#include "DBWrapper/Path.h"
#include "Util/DebugConnect.h"

RunAppParametersDialog::RunAppParametersDialog (const EXTRA_PARAM &extra_parameters, QWidget *p_parent):
	QDialog(p_parent),
	m_ui_ptr(std::make_unique<Ui::RunAppParametersDialog>()), 
	m_extra_parameters(extra_parameters)
{
	auto &ui = this->ui();
	ui.setupUi(this);

	initializeChildControls();
}

RunAppParametersDialog::~RunAppParametersDialog ()
{
	// (Ui::RunAppParametersDialog class is incomplete in .h)
}

void RunAppParametersDialog::initializeChildControls ()
{
	auto &ui = this->ui();

	ASSERTED_CONNECT(ui.pushButton_ApplicationName, &QPushButton::clicked,
		this, &RunAppParametersDialog::setApplicationName);
	ASSERTED_CONNECT(ui.lineEdit_ApplicationName, &QLineEdit::textEdited,
		this, &RunAppParametersDialog::editApplicationName);

	Q_UNUSED(ui);
}

void RunAppParametersDialog::setParameters (const RunAppParameters &parameters)
{
	auto &ui = this->ui();

	m_parameters = parameters;

	ui.lineEdit_ApplicationName->setText(m_parameters.getApplicationName());
	// set new values to child controls
	// do smth

	Q_UNUSED(ui);
}

void RunAppParametersDialog::accept ()
{
	auto &ui = this->ui();

	// do smth
	// save parameters (from child controls)

	Q_UNUSED(ui);

	QDialog::accept();
}

void RunAppParametersDialog::setApplicationName()
{
	QString ApplicationName = QFileDialog::getOpenFileName(this, tr("Open an Application"),
														   "c:\\", tr("Application (*.exe)"));
	m_parameters.setApplicationName(ApplicationName);
	ui().lineEdit_ApplicationName->clear();
	ui().lineEdit_ApplicationName->insert(ApplicationName);
}

void RunAppParametersDialog::editApplicationName(const QString& ApplicationName)
{
	m_parameters.setApplicationName(ApplicationName);
}