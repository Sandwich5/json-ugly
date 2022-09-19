#include <iostream>
#include <variant>

int main() {
	
	std::variant<
		int,
		double,
		std::string
	> x;
	
	x = 1;
	
	std::cout << "variant: " << x << '\n';
	
	return EXIT_SUCCESS;
}