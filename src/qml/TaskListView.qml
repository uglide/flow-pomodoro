import QtQuick 2.0

import Controller 1.0

ListView {
    id: root
    property alias emptyText: emptyTextLabel.text
    clip: true
    highlightFollowsCurrentItem: true
    highlightMoveVelocity: 20000
    spacing: 3 * _controller.dpiFactor
    visible: _controller.expanded
    currentIndex: _controller.selectedTaskIndex
    onCountChanged: {
        // HACK: For some reason the first inserted element takes more than 1 event loop.
        // It doesn't go immediately into the list view after we insert it into the model in controller.cpp
        // that event loop run breaks focus, so restore it here.
        // Make the newly inserted task visible

        _controller.forceFocusOnTaskBeingEdited()
    }

    delegate: Task {
        taskObj: task
        id: taskItem
        objectName: "taskItem"
        buttonsVisible: _controller.editMode === Controller.EditModeNone && (hasMouseOver || _controller.isMobile)
        modelIndex: index

        onDeleteClicked: {
            _controller.removeTask(taskObj)
        }
    }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 300 }
    }

    remove: Transition {
        enabled: _style.deleteAnimationEnabled
        NumberAnimation { property: "opacity"; to: 0; duration: 500 }
    }

    Text {
        id: emptyTextLabel
        renderType: _controller.textRenderType
        horizontalAlignment: Text.AlignHCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -height
        text: _controller.currentTabTag == null ? qsTr("No archived untagged tasks found.")
                                                : qsTr("No archived tasks found with tag %1").arg(_controller.currentTabTag.name)
        visible: parent.model && parent.model.count === 0
        wrapMode: Text.WordWrap
        anchors.leftMargin: _style.marginSmall
        anchors.rightMargin: _style.marginSmall
        font.pixelSize: 13 * _controller.dpiFactor
        fontSizeMode: Text.Fit
    }

    contentItem.z: 2

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (_controller.taskBeingEdited !== null) {
                _controller.editTask(null, Controller.EditModeNone)
            } else {
                _controller.expanded = false
            }
        }
    }
}
