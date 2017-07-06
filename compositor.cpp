/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>

#include "compositor.h"
#include "window.h"

#include <QtWaylandCompositor/QWaylandXdgShellV5>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/qwaylandoutput.h>
#include <QtWaylandCompositor/qwaylandseat.h>
#include <QOpenGLFunctions>

QOpenGLTexture *View::getTexture() {
    if (advance())
        m_texture = currentBuffer().toOpenGLTexture();
    return m_texture;
}

bool View::isCursor() const
{
    return surface()->isCursorSurface();
}

Compositor::Compositor(Window *window)
    : QWaylandCompositor()
    , m_window(window)
{
}

Compositor::~Compositor()
{
}

void Compositor::create()
{
    QWaylandOutput *output = new QWaylandOutput(this, m_window);
    QWaylandOutputMode mode(QSize(1280, 800), 60000);
    output->addMode(mode, true);
    QWaylandCompositor::create();
    output->setCurrentMode(mode);

    connect(this, &QWaylandCompositor::surfaceCreated, this, &Compositor::onSurfaceCreated);

    QString accelerometerPath =  QString::fromLocal8Bit(qgetenv("NUBBOCK_ACCELEROMETER_DEV"));

    if (!accelerometerPath.isEmpty()) {
        qInfo() << "USING ACCEL" << accelerometerPath;
        accelerometer = new Accelerometer(accelerometerPath, this);

        QObject::connect(accelerometer, &Accelerometer::orientationChanged, this, [this, output](Accelerometer::Orientation o) {
            qInfo() << "ORIENTATION CHANGED!";
            switch (o) {
            case Accelerometer::Standing:
            default:
                output->setTransform(QWaylandOutput::Transform270);
                break;
            case Accelerometer::Laying:
                output->setTransform(QWaylandOutput::Transform90);
                break;
            }
        });

        accelerometer->emitCurrent();
    }

    output->setTransform(QWaylandOutput::Transform90);
}

void Compositor::onSurfaceCreated(QWaylandSurface *surface)
{
    connect(surface, &QWaylandSurface::surfaceDestroyed, this, &Compositor::onSurfaceDestroyed);
    connect(surface, &QWaylandSurface::redraw, this, &Compositor::triggerRender);
    View *view = new View;
    view->setSurface(surface);
    view->setOutput(outputFor(m_window));
    m_views << view;
    connect(view, &QWaylandView::surfaceDestroyed, this, &Compositor::viewSurfaceDestroyed);
}

void Compositor::onSurfaceDestroyed()
{
    triggerRender();
}

void Compositor::viewSurfaceDestroyed()
{
    View *view = qobject_cast<View*>(sender());
    m_views.removeAll(view);
    delete view;
}

void Compositor::triggerRender()
{
    m_window->requestUpdate();
}

void Compositor::startRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->frameStarted();
}

void Compositor::endRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->sendFrameCallbacks();
}

void Compositor::handleMouseEvent(QWaylandView *target, QMouseEvent *me)
{
    QWaylandSeat *input = defaultSeat();
    QWaylandSurface *surface = target ? target->surface() : nullptr;
    switch (me->type()) {
        case QEvent::MouseButtonPress:
            input->sendMousePressEvent(me->button());
            if (surface != input->keyboardFocus()) {
                if (surface == nullptr
                        || surface->role() == QWaylandWlShellSurface::role()
                        || surface->role() == QWaylandXdgSurfaceV5::role()
                        || surface->role() == QWaylandXdgPopupV5::role()) {
                    input->setKeyboardFocus(surface);
                }
            }
            break;
    case QEvent::MouseButtonRelease:
         input->sendMouseReleaseEvent(me->button());
         break;
    case QEvent::MouseMove:
        input->sendMouseMoveEvent(target, me->localPos(), me->globalPos());
    default:
        break;
    }
}

