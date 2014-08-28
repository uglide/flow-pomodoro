/*
  This file is part of Flow.

  Copyright (C) 2014 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FLOW_KERNEL_H
#define FLOW_KERNEL_H

#include "task.h"
#include "runtimeconfiguration.h"

#include <QObject>

class Storage;
class Controller;
class PluginModel;
class QQmlEngine;
class QQmlContext;

class Kernel : public QObject
{
    Q_OBJECT
public:
    static Kernel *instance();
    Storage* storage() const;
    Controller *controller() const;
    QQmlContext *qmlContext() const;
    QQmlEngine *qmlEngine() const;
    void setRuntimeConfiguration(const RuntimeConfiguration &); // So unit-tests can use another configuration
    RuntimeConfiguration runtimeConfiguration() const;

private Q_SLOTS:
    void onTaskStatusChanged();

private:
    void loadPlugins();
    void notifyPlugins(TaskStatus newStatus);

    explicit Kernel(QObject *parent = 0);
    Storage *m_storage;
    QQmlEngine *m_qmlEngine;
    Controller *m_controller;
    PluginModel *m_pluginModel;
    RuntimeConfiguration m_runtimeConfiguration;
};

#endif