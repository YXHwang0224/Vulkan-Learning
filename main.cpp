#include "application.h"


int main() {
		
	std::shared_ptr<FF::Application> app = std::make_shared<FF::Application>();

	try {
		app->run();
	}
	catch(const std::exception& err){
		std::cout << err.what() << std::endl;
	}
	
	return 0;
}