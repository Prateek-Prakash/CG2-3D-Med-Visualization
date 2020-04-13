#ifndef Stereoscopic_H
#define Stereoscopic_H

#include <QMainWindow>
#include <QVTKWidget.h>

class Ui_Stereoscopic;

class Stereoscopic : public QMainWindow
{
	Q_OBJECT

public:
	Stereoscopic();
	~Stereoscopic() {};
	void setFileOutputWindow();
	void getRenderWindow();
	void getInteractor();
	void setTrackballInteractor();
	void setJoystickInteractor();
	void setRenderers();
	void setCameras();
	void readData();
	void createVolume();
	void setTransform();
	void createOTFChart();
	void setOTFValues();
	void addOTFPoint(double intensity, double transparency, int index);

	QVTKWidget *opacityWidget = new QVTKWidget();

public slots:
	virtual void slotExit();
	virtual void slotTrackball();
	virtual void slotJoystick();
	virtual void slotFull();
	virtual void slotSelected();
	virtual void slotOTF();

private:
	Ui_Stereoscopic *ui;
};

#endif
