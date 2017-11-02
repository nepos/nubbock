/***
  Copyright (c) 2017 Nepos GmbH

  Authors: Daniel Mack <daniel@nepos.io>

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

#include <QDebug>
#include <QObject>
#include <QSocketNotifier>

#include <linux/input.h>
#include "inputdevice.h"

Q_LOGGING_CATEGORY(InputDeviceLog, "InputDevice")

#define BITS_PER_LONG   (sizeof(long) * 8)
#define NBITS(x)        ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)          ((x)%BITS_PER_LONG)
#define BIT(x)          (1UL<<OFF(x))
#define LONG(x)         ((x)/BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

InputDevice::InputDevice(const QString &path, QObject *parent) :
    QObject(parent), device(path)
{
    if (!device.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        qWarning(InputDeviceLog) << "Unable to open file " << path << ":" << device.errorString();
        return;
    }

    auto sn = new QSocketNotifier(device.handle(), QSocketNotifier::Read, this);

    QObject::connect(sn, &QSocketNotifier::activated, [this](){
        struct input_event ev;
        qint64 r;

        r = device.read((char *) &ev, (qint64) sizeof(ev));
        if (r == sizeof(ev))
            update(ev.type, ev.code, ev.value);
        else
            qWarning(InputDeviceLog) << "Short read from device " << device.fileName();
    });
}

void InputDevice::update(int type, int code, int value)
{
    emit inputEvent(type, code, value);
}

void InputDevice::emitCurrent()
{
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)] = { 0 };
    unsigned int type, code;
    int ret;

    if (!device.isOpen())
        return;

    int fd = device.handle();

    ret = ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
    if (ret < 0) {
        qWarning(InputDeviceLog) << "ioctl(EVIOCGBIT) failed for " << device.fileName() << ":" << device.errorString();
        return;
    }

    for (type = 0; type < EV_MAX; type++) {
        if (test_bit(type, bit[0])) {
            switch (type) {
            case EV_ABS:
                ioctl(fd, EVIOCGBIT(type, ABS_MAX), bit[type]);

                for (code = 0; code < ABS_MAX; code++) {
                    if (test_bit(code, bit[type])) {
                        struct input_absinfo abs;
                        ioctl(fd, EVIOCGABS(code), &abs);
                        update(type, code, abs.value);
                    }
                }
            }
        }
    }
}

InputDevice::~InputDevice()
{
    if (device.isOpen())
        device.close();
}
