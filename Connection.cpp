#include "Connection.h"

int Connection::write(StringRef buf){
	return write(buf->c_str(), buf->length());
}
int Connection::write(std::string &buf){
	return write(buf.c_str(), buf.length());
}
