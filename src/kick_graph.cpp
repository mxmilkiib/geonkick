/**
 * File name: kick_graph.cpp
 * Project: Geonkick (A kick synthesizer)
 *
 * Copyright (C) 2017 Iurie Nistor (http://geontime.com)
 *
 * This file is part of Geonkick.
 *
 * GeonKick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "kick_graph.h"
#include "geonkick_api.h"
#include "globals.h"

KickGraph::KickGraph(QObject *parent, GeonkickApi *api)
        : QObject(parent),
          geonkickApi(api),
          kickBuffer(48000 * geonkickApi->kickLength() / 1000)
{
        connect(api, SIGNAL(kickUpdated()), this, SLOT(updateGraphBuffer()));
}

KickGraph::~KickGraph()
{
}

void KickGraph::draw(QPainter &painter)
{
        if (!cacheGraphImage.isNull()) {
                painter.drawImage(drawingArea, cacheGraphImage);
        }
}

void KickGraph::setDrawingArea(const QRect &rect)
{
        drawingArea = rect;
        cacheGraphImage = QImage(drawingArea.size(),  QImage::Format_ARGB32_Premultiplied);
        cacheGraphImage.fill(Qt::transparent);
        geonkickApi->getKickBuffer(kickBuffer);
        drawKickGraph();
}

void KickGraph::updateGraphBuffer()
{
        kickBuffer.resize(48000 * geonkickApi->kickLength() / 1000);
        geonkickApi->getKickBuffer(kickBuffer);
        drawKickGraph();
}

void KickGraph::drawKickGraph()
{
        if (kickBuffer.empty()) {
                return;
        }

        cacheGraphImage.fill(Qt::transparent);
        QPainter painter(&cacheGraphImage);
        QPen pen(QColor(59, 130, 4, 230));
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        int w = drawingArea.width();
        int h = drawingArea.height();
        painter.setRenderHints(QPainter::Antialiasing, true);

        QPolygonF graphPoints;
        gkick_real k = static_cast<gkick_real>(w) / kickBuffer.size();
        for (decltype(kickBuffer.size()) i = 0; i < kickBuffer.size(); i++) {
                graphPoints << QPointF(k * i, h * (0.5 - kickBuffer[i]));
        }
        painter.drawPolyline(graphPoints);
        painter.end();
        emit graphUpdated();
}