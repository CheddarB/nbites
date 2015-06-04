#include "ParamReader.h"
#include <iostream>
#include <sys/stat.h>

namespace man{

ParamReader::ParamReader(std::string filename)
{
	if(FILE *file = fopen(filename.c_str(),"r")) {
		fclose(file);
		boost::property_tree::read_json(filename, tree);
	} else {
		std::cout<<"[ERR] File Does Not Exist"<<std::endl;
	}
}
}
