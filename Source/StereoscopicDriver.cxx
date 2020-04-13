#include <QApplication>
#include "Stereoscopic.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Stereoscopic stereoscopic;
	stereoscopic.showMaximized();

	return app.exec();
}
