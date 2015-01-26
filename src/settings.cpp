/*
  This file is part of Flow.

  Copyright (C) 2013-2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "settings.h"
#include "task.h"
#include <QCoreApplication>
#include <QMetaType>

enum {
    DefaultPomodoroDuration = 25
};

Settings::Settings(QObject *parent)
    : QSettings("KDAB", "flow-pomodoro", parent)
    , m_syncScheduled(false)
    , m_needsSync(false)
    , m_defaultPomodoroDuration(DefaultPomodoroDuration)
    , m_pomodoroFunctionalityDisabled(false)
    , m_keepScreenOnDuringPomodoro(false)
    , m_syncAtStartup(true)
    , m_hideEmptyTags(false)
    , m_showAllTasksView(false)
    , m_useSystray(false)
    , m_showTaskAge(false)
    , m_stickyWindow(true)
    , m_showContextMenuAfterAdd(true)
{
    init();
}

Settings::Settings(const QString &filename, QObject *parent)
    : QSettings(filename, QSettings::IniFormat, parent)
    , m_syncScheduled(false)
    , m_needsSync(false)
{
    init();
}

void Settings::init()
{
    m_defaultPomodoroDuration = value("defaultPomodoroDuration", /*default=*/ DefaultPomodoroDuration).toInt();
    m_pomodoroFunctionalityDisabled = value("pomodoroFunctionalityDisabled", /*default=*/ false).toBool();
    m_syncAtStartup = value("syncAtStartup", /*default=*/ false).toBool();
    setKeepScreenOnDuringPomodoro(value("keepScreenOnDuringPomodoro", /*default=*/ true).toBool());
    m_hideEmptyTags = value("hideEmptyTags", /*default=*/ false).toBool();
    m_useSystray = value("useSystray", /*default=*/ true).toBool();
    m_stickyWindow = value("stickyWindow", /*default=*/ true).toBool();
    m_showTaskAge = value("showTaskAge", /*default=*/ false).toBool();
    m_showAllTasksView = value("showAllTasksView", /*default=*/ false).toBool();
    m_showContextMenuAfterAdd = value("showContextMenuAfterAdd", true).toBool();

    const Position defaultPosition = PositionTop;
    m_initialPosition = static_cast<Settings::Position>(value("windowInitialPosition", defaultPosition).toInt());
    if (m_initialPosition < PositionNone || m_initialPosition >= MaxPositions) {
        setInitialPosition(defaultPosition);
    }

    const GeometryType defaultGeometry = GeometryStandard;
    m_geometryType = static_cast<GeometryType>(value("windowGeometryType", defaultGeometry).toInt());
    if (m_geometryType < GeometryStandard || m_geometryType >= MaxGeometryTypes) {
        setGeometryType(defaultGeometry);
    }
}

Settings::~Settings()
{
    // qDebug() << "~Settings";
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
    m_needsSync = true;
}

void Settings::scheduleSync()
{
    if (!m_syncScheduled) {
        //qDebug() << Q_FUNC_INFO;
        m_syncScheduled = true;
        m_needsSync = false;
        QMetaObject::invokeMethod(this, "doSync", Qt::QueuedConnection);
    }
}

bool Settings::needsSync() const
{
    return m_needsSync;
}

void Settings::doSync()
{
    //qDebug() << Q_FUNC_INFO;
    m_syncScheduled = false;
    QSettings::sync();
}


void Settings::setHideEmptyTags(bool hide)
{
    if (hide != m_hideEmptyTags) {
        m_hideEmptyTags = hide;
        setValue("hideEmptyTags", QVariant(hide));
        emit hideEmptyTagsChanged();
    }
}

bool Settings::hideEmptyTags() const
{
    return m_hideEmptyTags;
}

void Settings::setUseSystray(bool use)
{
    if (use != m_useSystray) {
        m_useSystray = use;
        setValue("useSystray", QVariant(use));
        emit useSystrayChanged();
    }
}

bool Settings::useSystray() const
{
    return m_useSystray;
}

bool Settings::stickyWindow() const
{
    return m_stickyWindow;
}

void Settings::setStickyWindow(bool sticky)
{
    if (sticky != m_stickyWindow) {
        m_stickyWindow = sticky;
        setValue("stickyWindow", QVariant(sticky));
        emit stickyWindowChanged();
    }
}

void Settings::setShowTaskAge(bool showTaskAge)
{
    if (showTaskAge != m_showTaskAge) {
        m_showTaskAge = showTaskAge;
        setValue("showTaskAge", QVariant(showTaskAge));
        emit showTaskAgeChanged();
    }
}

bool Settings::showTaskAge() const
{
    return m_showTaskAge;
}

void Settings::setShowAllTasksView(bool showAllTasksView)
{
    if (showAllTasksView != m_showAllTasksView) {
        m_showAllTasksView = showAllTasksView;
        setValue("showAllTasksView", QVariant(showAllTasksView));
        emit showAllTasksViewChanged();
    }
}

bool Settings::showAllTasksView() const
{
    return m_showAllTasksView;
}

void Settings::setDefaultPomodoroDuration(int duration)
{
    if (m_defaultPomodoroDuration != duration && duration > 0 && duration < 59) {
        m_defaultPomodoroDuration = duration;
        setValue("defaultPomodoroDuration", QVariant(duration));
        emit defaultPomodoroDurationChanged();
    }
}

int Settings::defaultPomodoroDuration() const
{
    return m_defaultPomodoroDuration;
}

void Settings::setPomodoroFunctionalityDisabled(bool disable)
{
    if (disable != m_pomodoroFunctionalityDisabled) {
        m_pomodoroFunctionalityDisabled = disable;
        setValue("pomodoroFunctionalityDisabled", QVariant(disable));
        emit pomodoroFunctionalityDisabledChanged();
    }
}

bool Settings::pomodoroFunctionalityDisabled() const
{
    return m_pomodoroFunctionalityDisabled;
}

void Settings::setKeepScreenOnDuringPomodoro(bool keep)
{
    if (keep != m_keepScreenOnDuringPomodoro) {
        m_keepScreenOnDuringPomodoro = keep;
        setValue("keepScreenOnDuringPomodoro", keep);
        emit keepScreenOnDuringPomodoroChanged();
    }
}

bool Settings::keepScreenOnDuringPomodoro() const
{
    return m_keepScreenOnDuringPomodoro;
}

bool Settings::syncAtStartup() const
{
    return m_syncAtStartup;
}

void Settings::setSyncAtStartup(bool sync)
{
    if (m_syncAtStartup != sync) {
        m_syncAtStartup = sync;
        setValue("syncAtStartup", sync);
        emit syncAtStartupChanged();
    }
}

void Settings::setGeometryType(GeometryType geometryType)
{
    if (geometryType != m_geometryType) {
        m_geometryType = geometryType;
        setValue("windowGeometryType", geometryType);
        emit geometryTypeChanged();
    }
}

Settings::GeometryType Settings::geometryType() const
{
    return m_geometryType;
}

Settings::Position Settings::initialPosition() const
{
    return m_initialPosition;
}

void Settings::setInitialPosition(Position position)
{
    if (m_initialPosition != position) {
        m_initialPosition = position;
        setValue("windowInitialPosition", position);
    }
}

void Settings::setShowContextMenuAfterAdd(bool showContextMenuAfterAdd)
{
    if (showContextMenuAfterAdd != m_showContextMenuAfterAdd) {
        m_showContextMenuAfterAdd = showContextMenuAfterAdd;
        setValue("showContextMenuAfterAdd", showContextMenuAfterAdd);
        emit showContextMenuAfterAddChanged();
    }
}

bool Settings::showContextMenuAfterAdd() const
{
    return m_showContextMenuAfterAdd;
}
