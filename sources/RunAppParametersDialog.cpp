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

	m_highlighter = new QRegexHighligher(this);

	initializeChildControls();
}

RunAppParametersDialog::~RunAppParametersDialog ()
{
	delete m_highlighter;
	// (Ui::RunAppParametersDialog class is incomplete in .h)
}

void RunAppParametersDialog::initializeChildControls ()
{
	auto &ui = this->ui();

	ASSERTED_CONNECT(ui.pushButton_ApplicationName, &QPushButton::clicked,
		this, &RunAppParametersDialog::setApplicationName);
	ASSERTED_CONNECT(ui.lineEdit_ApplicationName, &QLineEdit::textEdited,
		this, &RunAppParametersDialog::editApplicationName);
	ASSERTED_CONNECT(ui.plainTextEdit_AppParams, &QPlainTextEdit::textChanged,
		this, &RunAppParametersDialog::editApplicationParameters);

	ui.plainTextEdit_AppParams->setPlaceholderText(
		"Type here the application parameters the same like the following: --key=value");
	m_highlighter->setDocument(ui.plainTextEdit_AppParams->document());
	
	Q_UNUSED(ui);
}

void RunAppParametersDialog::setParameters (const RunAppParameters &parameters)
{
	auto &ui = this->ui();

	m_parameters = parameters;

	ui.lineEdit_ApplicationName->setText(m_parameters.getApplicationName());
	ui.plainTextEdit_AppParams->setPlainText(m_parameters.getApplicationParameters());
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

void RunAppParametersDialog::editApplicationParameters()
{
	m_parameters.setApplicationParameters(this->ui().plainTextEdit_AppParams->toPlainText());
}

QRegexHighligher::QRegexHighligher(QObject *parent) : QSyntaxHighlighter(parent)
{
	_regex = QRegExp("(\\s|^)-{2}([A-Za-z_]+\\w*)\\s*=\\s*((\\{@\\w+\\})|(\\S+))(\\s|$)");
	_format = QTextCharFormat();
	_format.setForeground(QColor(0,127,76,204));
	_format.setFontWeight(QFont::Bold);
}

void QRegexHighligher::highlightBlock(const QString &text)
{
	QRegExp whitespace("\\s");
	int index = _regex.indexIn(text);
	while (index >= 0)
	{
		int length = _regex.matchedLength();
		setFormat(index, length, _format);
		if (whitespace.exactMatch(text[index + length - 1]))
			index--;
		index = _regex.indexIn(text, index + length);
	}
}