#include "logger.h"

using namespace std;

Logger::Logger(QString location)
{

    file = new QFile(location);
    if (file->open(QIODevice::WriteOnly | QIODevice::Text))
        out = new QTextStream(file);
    else
        cout<<"OPENING FAIL!!"<<endl;

}

Logger::~Logger(){
    file->close();
}


void Logger::log(QString str){
    *out << str << endl;
}
