#include "routersvr/router_svr.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	RouterAppSgl::mutable_instance().Init(argc, argv);
	RouterAppSgl::mutable_instance().Run();
	return 0;
}