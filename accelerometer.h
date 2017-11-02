#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <QtCore/QLoggingCategory>
#include <QObject>
#include "inputdevice.h"

Q_DECLARE_LOGGING_CATEGORY(AccelerometerLog)

class Accelerometer : public InputDevice
{
    Q_OBJECT
public:

    enum Orientation {
        Undefined		= 0,
        Standing		= 1,
        Laying  		= 2,
    };

    explicit Accelerometer(const QString &path, QObject *parent = 0);
    virtual ~Accelerometer();

protected:
    virtual void update(int type, int code, int value) override;

signals:
    void orientationChanged(Orientation);

private:
    Orientation currentOrientation;
};

#endif // ACCELEROMETER_H
