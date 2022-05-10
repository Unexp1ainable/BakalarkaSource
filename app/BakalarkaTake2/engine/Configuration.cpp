/*****************************************************************//**
 * \file   Configuration.cpp
 * \brief  Implementation of the class
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include "Configuration.h"

#include <QFile>
#include <QMessageBox>
#include <iostream>
#include <fstream>


Configuration::Configuration()
{
	load();
}

void Configuration::setPortAngle(double angle)
{
	m_portAngle = angle;
	auto els = m_xml.documentElement().elementsByTagName(EL_PORT_ANGLE);
	if (els.count() == 0) {
		errMsg("No port angle data in the saved configuration.");
		return;
	}
	QDomElement el = els.item(0).toElement();

	QDomElement newEl = m_xml.createElement(EL_PORT_ANGLE);
	QDomText newNodeText = m_xml.createTextNode(QString::number(angle));
	newEl.appendChild(newNodeText);
	el.parentNode().replaceChild(newEl, el);
}

void Configuration::setAlpha(Segments seg, double alpha)
{
	m_alphas[seg] = alpha;
	auto name = m_seg2str[seg];
	auto els = m_xml.documentElement().elementsByTagName(name);
	if (els.count() == 0) {
		errMsg(QString("Missing data of segment %1.").arg(name));
		return;
	}
	QDomElement el = els.item(0).toElement();
	
	els = el.elementsByTagName(EL_ALPHA);
	if (els.count() == 0) {
		errMsg(QString("Missing alpha data of segment %1.").arg(name));
		return;
	}
	el = els.item(0).toElement();

	QDomElement newEl = m_xml.createElement(EL_ALPHA);
	QDomText newNodeText = m_xml.createTextNode(QString::number(alpha));
	newEl.appendChild(newNodeText);
	el.parentNode().replaceChild(newEl, el);
}

void Configuration::setBeta(Segments seg, double beta)
{
	m_betas[seg] = beta;
	auto name = m_seg2str[seg];
	auto els = m_xml.documentElement().elementsByTagName(name);
	if (els.count() == 0) {
		errMsg(QString("Missing data of segment %1.").arg(name));
		return;
	}
	QDomElement el = els.item(0).toElement();

	els = el.elementsByTagName(EL_BETA);
	if (els.count() == 0) {
		errMsg(QString("Missing beta data of segment %1.").arg(name));
		return;
	}
	el = els.item(0).toElement();

	QDomElement newEl = m_xml.createElement(EL_BETA);
	QDomText newNodeText = m_xml.createTextNode(QString::number(beta));
	newEl.appendChild(newNodeText);
	el.parentNode().replaceChild(newEl, el);
}

void Configuration::load()
{
	m_xml = QDomDocument();
	QFile file("config.xml");
	if (!file.open(QIODevice::ReadOnly))
		createDefault();
	else if (!m_xml.setContent(&file)) {
		createDefault();
	}
	file.close();

	auto root = m_xml.documentElement();
	auto els = root.elementsByTagName(EL_PORT_ANGLE);
	if (els.count() == 0) {
		errMsg("No port angle data in the saved configuration.");
		return;
	}
	QDomElement el = els.item(0).toElement();
	m_portAngle = el.text().toDouble();

	for (const auto& SEG : { EL_Q1, EL_Q2, EL_Q3, EL_Q4 }) {
		auto els = root.elementsByTagName(SEG);
		if (els.count() == 0) {
			errMsg(QString("Missing data of segment %1.").arg(SEG));
			return;
		}
		QDomElement segment = els.item(0).toElement();

		auto alphas = segment.elementsByTagName(EL_ALPHA);
		if (els.count() == 0) {
			errMsg(QString("Missing data alpha of segment %1.").arg(SEG));
			return;
		}
		QDomElement alpha = alphas.item(0).toElement();
		m_alphas[m_str2seg[SEG]] = alpha.text().toDouble();

		auto betas = segment.elementsByTagName(EL_BETA);
		if (els.count() == 0) {
			errMsg(QString("Missing data beta of segment %1.").arg(SEG));
			return;
		}
		QDomElement beta = betas.item(0).toElement();
		m_betas[m_str2seg[SEG]] = beta.text().toDouble();
	}
}


void Configuration::createDefault()
{
	auto proc = m_xml.createProcessingInstruction(
		u8"xml", u8"version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
	m_xml.appendChild(proc);
	QDomElement root = m_xml.createElement(EL_ROOT);
	m_xml.appendChild(root);

	QDomElement angle = m_xml.createElement(EL_PORT_ANGLE);
	root.appendChild(angle);

	QDomText t = m_xml.createTextNode(QString::number(DEFAULT_PORT_ANGLE));
	angle.appendChild(t);

	for (const auto& el : { EL_Q1, EL_Q2, EL_Q3, EL_Q4 }) {
		QDomElement Q = m_xml.createElement(el);
		root.appendChild(Q);
		QDomElement alpha = m_xml.createElement(EL_ALPHA);
		Q.appendChild(alpha);
		t = m_xml.createTextNode(QString::number(DEFAULT_ALPHA));
		alpha.appendChild(t);
		QDomElement beta = m_xml.createElement(EL_BETA);
		Q.appendChild(beta);
		t = m_xml.createTextNode(QString::number(DEFAULT_BETA));
		beta.appendChild(t);
	}

	save();
}

void Configuration::save() const
{
	auto a = m_xml.toString();
	std::ofstream out(CFG_FILE_NAME);
	out << a.toStdString();
}

void Configuration::errMsg(QString msg)
{
	QMessageBox::critical(nullptr, "Error", msg);
}


