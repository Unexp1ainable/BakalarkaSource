#include "DetectorSettingsDialog.h"
#include <QPushButton>

DetectorSettingsDialog::DetectorSettingsDialog(Configuration& cfg, QWidget* parent) : m_cfg(cfg), QDialog(parent)
{
	ui.setupUi(this);
	update();
	connect(ui.okButton, &QPushButton::clicked, this, &DetectorSettingsDialog::onOkPressed);
	connect(ui.cancelButton, &QPushButton::clicked, this, &DetectorSettingsDialog::onCancelPressed);
}

DetectorSettingsDialog::~DetectorSettingsDialog()
{
}

void DetectorSettingsDialog::update()
{
	ui.angleSpinbox->setValue(m_cfg.portAngle());
	ui.bSpinboxQ1->setValue(m_cfg.beta(Segments::Q1));
	ui.bSpinboxQ2->setValue(m_cfg.beta(Segments::Q2));
	ui.bSpinboxQ3->setValue(m_cfg.beta(Segments::Q3));
	ui.bSpinboxQ4->setValue(m_cfg.beta(Segments::Q4));
	ui.cSpinboxQ1->setValue(m_cfg.alpha(Segments::Q1));
	ui.cSpinboxQ2->setValue(m_cfg.alpha(Segments::Q2));
	ui.cSpinboxQ3->setValue(m_cfg.alpha(Segments::Q3));
	ui.cSpinboxQ4->setValue(m_cfg.alpha(Segments::Q4));
}

void DetectorSettingsDialog::onCancelPressed()
{
	hide();
}

void DetectorSettingsDialog::onOkPressed()
{
	m_cfg.setPortAngle(ui.angleSpinbox->value());
	m_cfg.setAlpha(Segments::Q1, ui.cSpinboxQ1->value());
	m_cfg.setAlpha(Segments::Q2, ui.cSpinboxQ2->value());
	m_cfg.setAlpha(Segments::Q3, ui.cSpinboxQ3->value());
	m_cfg.setAlpha(Segments::Q4, ui.cSpinboxQ4->value());
	m_cfg.setBeta(Segments::Q1, ui.bSpinboxQ1->value());
	m_cfg.setBeta(Segments::Q2, ui.bSpinboxQ2->value());
	m_cfg.setBeta(Segments::Q3, ui.bSpinboxQ3->value());
	m_cfg.setBeta(Segments::Q4, ui.bSpinboxQ4->value());
	emit settingsUpdated();
	hide();
}