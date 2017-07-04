/***
  Copyright (c) 2017 Nepos GmbH

  Authors: Pascal Huerst <pascal.huerst@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
***/

#include <QtCore/QLoggingCategory>
#include <QtDebug>
#include <math.h>

#include "accelerometer.h"

Q_LOGGING_CATEGORY(AccelerometerLog, "Accelerometer")

Accelerometer::Accelerometer(const QString &path, QObject *parent) :
    InputDevice(path, parent),
    currentOrientation(Orientation::Undefined),
    currentAxes{0},
    oldAxes{0}
{
}

Accelerometer::~Accelerometer()
{}

void Accelerometer::update(int type, int code, int value)
{
    if (type != 3)
        return;

    currentAxes[code] = value;
    evaluate();
}

void Accelerometer::evaluate()
{
    // Device is inclined by 72 degree
    const static qreal inclinationDegree = 72.f;
    // Sensor reports int 1000th of g
    const static int gRefAxis1 = 981 * sin(inclinationDegree / 360.f * 2.f * M_PI);
    const static int gRefAxis2 = 981 * cos(inclinationDegree / 360.f * 2.f * M_PI);

    // Accept 5% deviation
    const static qreal deviation = 0.05;

    const static int upperLimitAxis1 = (1.f + deviation) * gRefAxis1;
    const static int lowerLimitAxis1 = (1.f - deviation) * gRefAxis1;

    const static int upperLimitAxis2 = (1.f + deviation) * gRefAxis2;
    const static int lowerLimitAxis2 = (1.f - deviation) * gRefAxis2;

    //TODO: Put correct axes here.
    //      This is all pretty basic. Do some proper testing with the
    //      real device!
    qreal *a1 = &currentAxes[0];
    qreal *a2= &currentAxes[2];
    qreal *a1Old = &oldAxes[0];
    qreal *a2Old = &oldAxes[2];

    // Add something like a hysteresis
    if (fabs(*a1 - *a1Old) < 98.1 ||
            fabs(*a2 - *a2Old) < 98.1)
        return;

    // Is device standing
    if (*a1 <= upperLimitAxis1 && *a1 >= lowerLimitAxis1 &&
            *a2 <= upperLimitAxis2 && *a2 >= lowerLimitAxis2 &&
            currentOrientation != Orientation::Standing) {
        currentOrientation = Orientation::Standing;
        emit orientationChanged(currentOrientation);
        oldAxes[0] = currentAxes[0];
        oldAxes[1] = currentAxes[1];
        oldAxes[2] = currentAxes[2];
    }

    // is device laying
    if (*a2 <= upperLimitAxis1 && *a2 >=lowerLimitAxis1 &&
            *a1 <= upperLimitAxis2 && *a1 >= lowerLimitAxis2 &&
            currentOrientation != Orientation::Laying) {
        currentOrientation = Orientation::Laying;
        emit orientationChanged(currentOrientation);
        oldAxes[0] = currentAxes[0];
        oldAxes[1] = currentAxes[1];
        oldAxes[2] = currentAxes[2];
    }


}
