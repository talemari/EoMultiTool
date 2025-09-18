#include "MainWindow.h"
#include <qapplication.h>

int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	MainWindow mainWindow( "EoMultiTool", "0.01" );

	mainWindow.show();
	return app.exec();
}