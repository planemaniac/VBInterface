#include <QtCore/QCoreApplication>

#include <QDebug>
#include <VBInterface>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	
	VBInterface* vb = new VBInterface;
	vb->login();
	qInfo() << "\nStarting tests\n";

	qInfo() << "Current output device: " << vb->getOutputDevice( VBInterface::BUS1 ).toString();
	qInfo() << "Current output volume: "  << vb->getVolume( VBInterface::BUS1 ) << "dB";

	qInfo() << "\nFinished Tests\n";
	vb->disconnect();

	delete vb;

	return a.exec();
}
