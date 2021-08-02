#pragma once

#include "RunAppParameters.h"

#include "Debug/Assert.h"
#include "MAIN/Common_p.h"

#include <QDialog>
#include <QFileDialog>
#include <qsyntaxhighlighter.h>
#include <QTextCharFormat>
#include <qregexp.h>


#include <memory>



namespace Ui { class RunAppParametersDialog; }

class QRegexHighligher;

class RunAppParametersDialog : public QDialog
{
	Q_OBJECT

public:
	RunAppParametersDialog (const EXTRA_PARAM &extra_parameters, QWidget *p_parent = nullptr);
	~RunAppParametersDialog () override;

	void setParameters (const RunAppParameters &parameters);
	inline const RunAppParameters& getParameters () const;

public slots:
	void accept () override;
	void setApplicationName();
	void editApplicationName(const QString&);
	void editApplicationParameters();

private:
	inline Ui::RunAppParametersDialog& ui () const noexcept;

	void initializeChildControls ();

private:
	std::unique_ptr<Ui::RunAppParametersDialog> m_ui_ptr;

	RunAppParameters m_parameters;
	const EXTRA_PARAM m_extra_parameters;

	QRegexHighligher *m_highlighter;
};


class QRegexHighligher : public QSyntaxHighlighter
{
	Q_OBJECT

private:
	QTextCharFormat _format;
	QRegExp _regex;
public:
	explicit  QRegexHighligher(QObject *parent = 0);

	void highlightBlock(const QString &text);
};

// ---------- Implementation ----------

const RunAppParameters& RunAppParametersDialog::getParameters () const
{
	return m_parameters;
}

Ui::RunAppParametersDialog& RunAppParametersDialog::ui () const noexcept
{
	ASSERT_NOTNULL(m_ui_ptr);
	return *m_ui_ptr;
}
