/*
  This file is part of Flow.

  Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Sérgio Martins <sergio.martins@kdab.com>

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

#include "quickview.h"
#include "controller.h"
#include "taskmodel.h"
#include "pluginmodel.h"
#include "plugininterface.h"
#include "controller.h"
#include "settings.h"

#include <QQmlContext>
#include <QString>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QQmlEngine>
#include <QFileSystemWatcher>
#include <QDir>
#include <QtQml>
#include <QPluginLoader>

QuickView::QuickView(bool developerMode, QWindow *parent)
    : QQuickView(parent)
    , m_developerMode(developerMode)
{
    TaskModel *taskModel = new TaskModel(this);
    m_pluginModel = new PluginModel(this);

    m_controller = new Controller(taskModel, this);
    rootContext()->setContextProperty("_controller", m_controller);
    rootContext()->setContextProperty("_taskModel", taskModel);
    rootContext()->setContextProperty("_pluginModel", m_pluginModel);
    rootContext()->setContextProperty("_window", this);

    qmlRegisterUncreatableType<Controller>("Controller",
                                           1, 0, "Controller",
                                           QStringLiteral("Controller is not creatable"));

    if (developerMode) {
        // So that F5 reloads QML without having to restart the application
        setSource(QUrl::fromLocalFile("qml/Main.qml"));
    } else {
        setSource(QUrl("qrc:/qml/Main.qml"));
    }

    setResizeMode(QQuickView::SizeViewToRootObject);
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setColor(Qt::transparent);

    const int width = 400; // TODO: it's hardcoded

    QSize screenSize = qApp->primaryScreen()->size();
    setGeometry(screenSize.width()/2 - width/2, 0, width, height());

    connect(m_controller, SIGNAL(taskStatusChanged()), SLOT(onTaskStatusChanged()));
    connect(engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    loadPlugins();
}

void QuickView::onTaskStatusChanged()
{
    notifyPlugins(m_controller->taskStatus());
}

void QuickView::reloadQML()
{
    qDebug() << "Reloading QML ...";
    engine()->clearComponentCache();
    setSource(source());
}

void QuickView::notifyPlugins(TaskStatus newStatus)
{
    PluginInterface::List plugins = m_pluginModel->plugins();
    foreach (PluginInterface *plugin, plugins) {
        plugin->setTaskStatus(newStatus);
    }
}

void QuickView::loadPlugins()
{
    QStringList paths = QCoreApplication::libraryPaths();

    foreach (const QString &path, paths) {
        QString candidatePath = path;
        if (path == qApp->applicationDirPath()) {
            candidatePath += QStringLiteral("/plugins/");
        } else {
            candidatePath += QStringLiteral("/flow/");
        }

        QDir pluginsDir = QDir(candidatePath);

        foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *pluginObject = loader.instance();
            if (pluginObject) {
                PluginInterface *pluginInterface = qobject_cast<PluginInterface*>(pluginObject);
                if (pluginInterface) {
                    pluginInterface->setTaskStatus(TaskStopped);
                    const QString pluginName = pluginObject->metaObject()->className();
                    const bool enabled = Settings::instance()->value(pluginName + ".enabled", /**defaul=*/true).toBool();
                    pluginInterface->setEnabled(enabled);
                    m_pluginModel->addPlugin(pluginInterface);
                }
            }
        }
    }

    const int count = m_pluginModel->rowCount();
    qDebug() << "Loaded" << count << (count == 1 ? "plugin" : "plugins");
}

void QuickView::keyReleaseEvent(QKeyEvent *event)
{
    if (m_developerMode && event->key() == Qt::Key_F5) {
        event->accept();
        reloadQML();
    } else {
        QQuickView::keyReleaseEvent(event);
    }
}

