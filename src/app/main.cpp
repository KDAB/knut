#include <QApplication>

#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	spdlog::info("Hello World!");
	
	return app.exec();
}