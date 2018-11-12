#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QTextStream>
#include <iostream>

class Logger
{
public:
    Logger(QString location);
    ~Logger();
    void log(QString str);

private:
    QFile* file;
    QTextStream* out;
};

#endif // LOGGER_H
