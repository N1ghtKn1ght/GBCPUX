pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import GBCPUX 

ApplicationWindow {
    id: _root
    
    visible: true
    minimumWidth: 480
    minimumHeight: 300

    menuBar: MenuBar {
        Menu {
            title: "File"

            Action {
                text: "Open"
                onTriggered: {
                    _fileDialog.open();
                    _fileDialog.accepted.connect(function() {
                         _test.load(_fileDialog.selectedFile);
                    })
                }
            }
        }
    }

    FileDialog {
        id: _fileDialog
        nameFilters: ["GB files (*.gb)"]
    }

    GameBoyEmulator {
        id: _test
        anchors.centerIn: parent
        width: 320
        height: 288
    }
}
