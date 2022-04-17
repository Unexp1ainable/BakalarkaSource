#pragma once

#include <QDialog>
#include "ui_DetectorSettingsDialog.h"
#include "../../engine/Configuration.h"

class DetectorSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	DetectorSettingsDialog(Configuration& cfg, QWidget *parent = nullptr);
	~DetectorSettingsDialog();
	void update();

	Configuration& m_cfg;

protected slots:
	void onOkPressed();
	void onCancelPressed();

private:
	Ui::DetectorSettingsDialog ui;

signals:
	void settingsUpdated();
};
