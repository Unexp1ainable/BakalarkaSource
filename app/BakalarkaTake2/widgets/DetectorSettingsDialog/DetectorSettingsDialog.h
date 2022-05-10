#pragma once
/*****************************************************************//**
 * \file   DetectorSettingsDialog.h
 * \brief  Dialog window managing the detector configuration.
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include <QDialog>
#include "ui_DetectorSettingsDialog.h"
#include "../../engine/Configuration.h"

class DetectorSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	DetectorSettingsDialog(Configuration& cfg, QWidget *parent = nullptr);
	~DetectorSettingsDialog();

	// update dialog with the latest values
	void update();

	// shared context
	Configuration& m_cfg;

protected slots:
	void onOkPressed();
	void onCancelPressed();

private:
	Ui::DetectorSettingsDialog ui;

signals:
	void settingsUpdated();
};
