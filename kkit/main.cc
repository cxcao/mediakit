#include <stdio.h>
#include <iostream>
#include "buffer.hh"
int main()
{
	std::string str("0123456789");
	kkit::bufferOffset<std::string>::ptr buf = std::make_shared<kkit::bufferOffset<std::string>>(str, 0, 8);

	auto[p,n] = buf->data_z();

	std::cout << n << "    " << p << std::endl;
	printf("Hello kkit !!!!\n");
	return 0;
}