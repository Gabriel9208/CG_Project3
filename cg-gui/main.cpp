#include <memory>
#include <iostream>

#include <App/App.h>

int main()
{
	std::cout << "App start!" << std::endl;

	std::unique_ptr<CG::App> app = std::make_unique<CG::App>();

	app->initialize();

	app->loop();
	
	app->terminate();

	std::cout << "App end!" << std::endl;

	return 0;
}