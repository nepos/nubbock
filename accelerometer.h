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
        Standing		= 0,
        Laying  		= 1,
        Undefined		= 2	//Sentinell
    };

    explicit Accelerometer(const QString &path, QObject *parent = 0);
    virtual ~Accelerometer();

protected:
    virtual void update(int type, int code, int value) override;

signals:
    void orientationChanged(Orientation);

private:
    void evaluate();

    Orientation currentOrientation;
    qreal currentAxes[3];
    qreal oldAxes[3];
};

#endif // ACCELEROMETER_H
