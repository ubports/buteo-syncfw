#ifndef OOPCLIENTPLUGIN_H
#define OOPCLIENTPLUGIN_H

#include <QObject>

namespace Buteo
{

class OOPClientPlugin : public QObject
{
    Q_OBJECT
public:
    explicit OOPClientPlugin(QObject *parent = 0);
    
signals:
    
public slots:
    
};

}

#endif // OOPCLIENTPLUGIN_H
