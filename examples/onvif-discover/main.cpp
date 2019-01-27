/* Copyright (C) 2019 Casper Meijn <casper@meijn.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "onvifdiscover.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Starting ONVIF discovery for 5 seconds";
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);

    OnvifDiscover * onvifDiscover = new OnvifDiscover(&app);
    onvifDiscover->start();

    return app.exec();
}