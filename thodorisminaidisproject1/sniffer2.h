#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <map>
#include <iterator>
#include <cstring>

using namespace std;

string clean_buffer(string filename);
map<string,int> parser(string filename);
