#pragma once

#include <map>

#include <QDomDocument>
#include <QFile>

enum class Segments {
	Q1,
	Q2,
	Q3,
	Q4
};

class Configuration {
public:
	Configuration();

	double portAngle() { return m_portAngle; };
	void setPortAngle(double angle);
	double alpha(Segments seg) { return m_alphas[seg]; };
	double alpha(int seg) { return m_alphas[m_segVec[seg]]; };
	void setAlpha(Segments seg, double alpha);
	double beta(Segments seg) { return m_betas[seg]; };
	double beta(int seg) { return m_betas[m_segVec[seg]]; };
	void setBeta(Segments seg, double beta);

protected:
	void load();
	void createDefault();
	void save() const;

	void errMsg(QString msg);

	QDomDocument m_xml;

protected:
	static constexpr const double DEFAULT_PORT_ANGLE = 0.;
	static constexpr const double DEFAULT_ALPHA = 1.;
	static constexpr const double DEFAULT_BETA = 0.;

	double m_portAngle = DEFAULT_PORT_ANGLE;
	std::map<Segments, double> m_alphas = {
											{Segments::Q1 ,DEFAULT_ALPHA},
											{Segments::Q2 ,DEFAULT_ALPHA},
											{Segments::Q3 ,DEFAULT_ALPHA},
											{Segments::Q4 ,DEFAULT_ALPHA},
	};

	std::map<Segments, double> m_betas = {
											{Segments::Q1 ,DEFAULT_BETA},
											{Segments::Q2 ,DEFAULT_BETA},
											{Segments::Q3 ,DEFAULT_BETA},
											{Segments::Q4 ,DEFAULT_BETA},
	};


	std::map<QString, Segments> m_str2seg = {
											{EL_Q1, Segments::Q1},
											{EL_Q2, Segments::Q2},
											{EL_Q3, Segments::Q3},
											{EL_Q4, Segments::Q4},
	};
	std::map<Segments, QString> m_seg2str = {
											{Segments::Q1, EL_Q1},
											{Segments::Q2, EL_Q2},
											{Segments::Q3, EL_Q3},
											{Segments::Q4, EL_Q4},
	};

	std::vector<Segments> m_segVec = { Segments::Q1,Segments::Q2,Segments::Q3,Segments::Q4 };

private:
	static inline const constexpr char* CFG_FILE_NAME = "config.xml";

	static inline const constexpr char* EL_ROOT = "root";
	static inline const constexpr char* EL_PORT_ANGLE = "port-angle";
	static inline const constexpr char* EL_Q1 = "Q1";
	static inline const constexpr char* EL_Q2 = "Q2";
	static inline const constexpr char* EL_Q3 = "Q3";
	static inline const constexpr char* EL_Q4 = "Q4";
	static inline const constexpr char* EL_ALPHA = "alpha";
	static inline const constexpr char* EL_BETA = "beta";

};