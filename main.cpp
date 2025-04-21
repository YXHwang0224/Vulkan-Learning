#include "application.h"


int main() {
		
	FF::Application app;

	try {
		app.Run();
	}
	catch(const std::exception& err){
		std::cout << err.what() << std::endl;
	}
	
	return 0;
}