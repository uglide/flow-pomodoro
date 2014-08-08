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

#include "task.h"
#include "settings.h"
#include "checkabletagmodel.h"
#include "storage.h"
#include <QQmlEngine>

enum {
    TagRole = Qt::UserRole,
    TaskRole
};

Task::Task(const QString &name)
    : QObject()
    , m_summary(name.isEmpty() ? tr("New Task") : name)
    , m_status(TaskStopped)
    , m_staged(false)
    , m_storage(Storage::instance())
{
    m_tags.insertRole("tag", [&](int i) { return QVariant::fromValue<Tag*>(m_tags.at(i).m_tag.data()); }, TagRole);
    m_tags.insertRole("task", [&](int i) { return QVariant::fromValue<Task*>(m_tags.at(i).m_task.data()); }, TaskRole);

    connect(this, &Task::summaryChanged, &Task::changed);
    connect(this, &Task::tagsChanged, &Task::changed);
    connect(this, &Task::descriptionChanged, &Task::changed);
    connect(this, &Task::statusChanged, &Task::changed);
    connect(this, &Task::stagedChanged, &Task::changed);
    QAbstractItemModel *tagsModel = m_tags; // android doesn't build if you use m_tags directly in the connect statement
    connect(tagsModel, &QAbstractListModel::modelReset, this, &Task::tagsChanged);
    connect(tagsModel, &QAbstractListModel::rowsInserted, this, &Task::tagsChanged);
    connect(tagsModel, &QAbstractListModel::rowsRemoved, this, &Task::tagsChanged);
    connect(tagsModel, &QAbstractListModel::layoutChanged, this, &Task::tagsChanged);
    connect(tagsModel, &QAbstractListModel::dataChanged, this, &Task::tagsChanged);

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    auto roleNames = m_storage->tagsModel()->roleNames();
    roleNames.insert(Qt::CheckStateRole, QByteArray("checkState"));
    m_checkableTagModel = new CheckableTagModel(this);
    m_checkableTagModel->setSourceModel(m_storage->tagsModel());
}

Task::Ptr Task::createTask(const QString &name)
{
    Task::Ptr task = Task::Ptr(new Task(name));
    task->setWeakPointer(task);
    return task;
}

bool Task::staged() const
{
   return m_staged;
}

void Task::setStaged(bool staged)
{
    if (m_staged != staged) {
        m_staged = staged;
        emit stagedChanged();
        for (int i = 0; i < m_tags.count(); ++i) {
            Tag::Ptr tag = m_tags.at(i).m_tag;
            int currentCount = tag->archivedTaskCount();
            tag->setArchivedTaskCount(currentCount + (m_staged ? -1 : 1));
        }
    }
}

QString Task::summary() const
{
    return m_summary;
}

void Task::setSummary(const QString &text)
{
    if (m_summary != text) {
        m_summary = text;
        emit summaryChanged();
    }
}

QString Task::description() const
{
    return m_description;
}

void Task::setDescription(const QString &text)
{
    if (m_description != text) {
        m_description = text;
        emit descriptionChanged();
    }
}

bool Task::containsTag(const QString &name) const
{
    return indexOfTag(name) != -1;
}

int Task::indexOfTag(const QString &name) const
{
    QString trimmedName = name.toLower().trimmed();
    for (int i = 0; i < m_tags.count(); ++i) {
        Q_ASSERT(m_tags.at(i).m_tag);
        if (m_tags.at(i).m_tag->name().toLower() == trimmedName)
            return i;
    }

    return -1;
}

TagRef::List Task::tags() const
{
    return m_tags;
}

void Task::setTagList(const TagRef::List &list)
{
    m_tags = list;
}

QAbstractItemModel *Task::tagModel() const
{
    return m_tags;
}

QAbstractItemModel *Task::checkableTagModel() const
{
    return m_checkableTagModel;
}

void Task::addTag(const QString &tagName)
{
    QString trimmedName = tagName.trimmed();
    if (trimmedName.isEmpty())
        return;

    if (!containsTag(trimmedName))
        m_tags.append(TagRef(this, trimmedName));
}

void Task::removeTag(const QString &tagName)
{
    int index = indexOfTag(tagName);
    if (index != -1)
        m_tags.removeAt(index);
}

TaskStatus Task::status() const
{
    return m_status;
}

void Task::setStatus(TaskStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void Task::setCreationDate(const QDateTime &date)
{
    m_creationDate = date;
}

QDateTime Task::creationDate() const
{
    return m_creationDate;
}

bool Task::running() const
{
    return m_status == TaskStarted;
}

bool Task::stopped() const
{
    return m_status == TaskStopped;
}

bool Task::paused() const
{
    return m_status == TaskPaused;
}

Task::Ptr Task::toStrongRef() const
{
    Q_ASSERT(m_this);
    return m_this.toStrongRef();
}

void Task::setWeakPointer(const QWeakPointer<Task> &ptr)
{
    m_this = ptr;
}

QVariantMap Task::toJson() const
{
    QVariantMap map;
    map.insert("summary", m_summary);
    map.insert("staged", m_staged);
    map.insert("description", m_description);
    QVariantList tags;
    for (int i = 0; i < m_tags.count(); ++i)
        tags << m_tags.at(i).m_tag->name();
    map.insert("tags", tags);

    return map;
}

Task::Ptr Task::fromJson(const QVariantMap &map)
{
    QString name = map.value("summary").toString();
    if (name.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "emptu task summary";
        return Task::Ptr();
    }

    Task::Ptr task = Task::Ptr(new Task(name));
    task->setWeakPointer(task.toWeakRef());

    QString description = map.value("description").toString();
    task->setDescription(description);
    task->setStaged(map.value("staged", false).toBool());
    QVariantList tagsVariant = map.value("tags").toList();
    TagRef::List tags;
    foreach (const QVariant &tag, tagsVariant) {
        tags << TagRef(task.data(), tag.toString());
    }
    task->setTagList(tags);
    return task;
}

QDataStream &operator<<(QDataStream &out, const Task::Ptr &task)
{
    Q_ASSERT(task);
    quint32 version = SerializerVersion2;
    out << version << task->summary();

    TagRef::List tags = task->tags();
    out << tags.count();
    for (int i = 0; i < tags.count(); ++i)
        out << tags.at(i).m_tag->name();

    out << task->description() << task->staged();
    return out;
}

QDataStream &operator>>(QDataStream &in, Task::Ptr &task)
{
    if (!task)
        task = Task::createTask();

    quint32 version = 0;
    in >> version;

    bool staged = false;
    QString summary;
    QString description;
    TagRef::List tags;

    switch (version) {
    case SerializerVersion1:
    case SerializerVersion2:
        int tagCount;
        in >> summary >> tagCount;
        for (int i = 0; i < tagCount; i++) {
            QString name;
            in >> name;
            if (Storage::instance()->deletedTagName() != name) {
                // QSettings reads before saving, which invokes this deserializer
                // Sometimes we're deleted a tag and it would get recreated because
                // it was loaded when we were saving
                // TODO: We're not using QSettings to save data anymore, remove this
                tags << TagRef(task.data(), name);
            }
        }
        break;

    default:
        Q_ASSERT(false);
    }

    if (version == SerializerVersion2)
        in >> description >> staged;

    task->setStaged(staged);
    task->setSummary(summary);
    task->setDescription(description);
    task->setTagList(tags);
    return in;
}
