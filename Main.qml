import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import Modbus 1.0

ApplicationWindow {
    visible: true
    width: 720
    height: 480
    title: "Modbus Master"

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    // Log model
    ListModel{
        id: logModel
    }

    // Receive log messages from C++
    Connections {
        target: modbusController

        function onLogMessage(message) {
            logModel.append({
                time: Qt.formatTime(new Date(), "hh:mm:ss"),
                text: message
            })
        }
    }

    // Main vertical layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        //Top panel — Connection settings. Auto-height (NO fixed height!)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120 // fix height 120
            radius: 8
            color: "#1b1b1f"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 20 // 12

                Label {
                    text: "Modbus connection settings"
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignHCenter
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    // IP address
                    TextField {
                        id: hostField
                        enabled: modbusController.state === ModbusController.Disconnected
                        placeholderText: "IP address (e.g. 192.168.0.10)"
                        Layout.fillWidth: true   // главное поле — шире остальных
                    }
                    // Port
                    TextField {
                        id: portField
                        enabled: modbusController.state === ModbusController.Disconnected
                        placeholderText: "Port (e.g. 502)"
                        inputMethodHints: Qt.ImhDigitsOnly
                        Layout.preferredWidth: 100
                    }

                    TextField {
                        id: unitField
                        enabled: modbusController.state === ModbusController.Disconnected
                        placeholderText: "Unit ID"
                        inputMethodHints: Qt.ImhDigitsOnly
                        Layout.preferredWidth: 100
                    }
                    Button {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 100

                        text: {
                                switch (modbusController.state) {
                                case ModbusController.Disconnected:
                                    return "Connect"
                                case ModbusController.Connecting:
                                    return "Connecting..."
                                case ModbusController.Connected:
                                    return "Disconnect"
                                case ModbusController.Error:
                                    return "Retry"
                                }
                        }

                        enabled: modbusController.state !== ModbusController.Connecting

                        onClicked: {
                            switch (modbusController.state) {
                            case ModbusController.Disconnected:
                                modbusController.connectToServer(
                                                hostField.text,
                                                parseInt(portField.text),
                                                parseInt(unitField.text)
                                )
                                break;
                            case ModbusController.Connected:
                                modbusController.disconnectFromServer()
                                break;
                            case ModbusController.Error:
                                        // позже: retry
                                break;

                            }
                        }
                    }
        }
                // Button {
                //     Layout.alignment: Qt.AlignHCenter
                //     Layout.preferredWidth: 100

                //     text: {
                //             switch (modbusController.state) {
                //             case ModbusController.Disconnected:
                //                 return "Connect"
                //             case ModbusController.Connecting:
                //                 return "Connecting..."
                //             case ModbusController.Connected:
                //                 return "Disconnect"
                //             case ModbusController.Error:
                //                 return "Retry"
                //             }
                //     }

                //     enabled: modbusController.state !== ModbusController.Connecting

                //     onClicked: {
                //         switch (modbusController.state) {
                //         case ModbusController.Disconnected:
                //             modbusController.connectToServer(
                //                             hostField.text,
                //                             parseInt(portField.text),
                //                             parseInt(unitField.text)
                //             )
                //             break;
                //         case ModbusController.Connected:
                //             modbusController.disconnectFromServer()
                //             break;
                //         case ModbusController.Error:
                //                     // позже: retry
                //             break;

                //         }
                //     }
                // }
            }
        }

        // Bottom: Log panel. Takes all remaining space
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true // all space
            radius: 8
            color: "#111"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Header //title + clear button
                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "Log"
                        color: "lightgray"
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Clear"
                        onClicked: logModel.clear()
                    }
                }

                // List of msgs
                ListView {
                    id: logView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: logModel
                    clip: true

                    delegate: Text {
                        text: model.time + " " + model.text
                        color: "lightgreen"
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }

                    ScrollBar.vertical: ScrollBar { }
                }
            }
        }

        // Status Bar
        // Text {
        //     text: {
        //         switch (modbusController.state) {
        //         case ModbusController.Disconnected: return "🔌 Disconnected"
        //         case ModbusController.Connecting:   return "⏳ Connecting"
        //         case ModbusController.Connected:    return "✅ Connected"
        //         case ModbusController.Error:        return "❌ Error"
        //         }
        //     }
        // }
    }

    // Auto-scroll log to bottom
    Connections {
        target: logModel

        function onCountChanged() {
            if (logModel.count > 0) {
                logView.positionViewAtEnd()
            }
        }
    }
}

/*
ApplicationWindow {
    visible: true
    width: 720
    height: 480
    title: "Modbus Master"

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    // Log model
    ListModel{
        id: logModel
    }

    // Receive log messages from C++
    Connections {
        target: modbusController

        function onLogMessage(message) {
            logModel.append({
                time: Qt.formatTime(new Date(), "hh:mm:ss"),
                text: message
            })
        }
    }

    // Main vertical layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        //Top panel — Connection settings. Auto-height (NO fixed height!)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120 // fix height 120
            radius: 8
            color: "#1b1b1f"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 20 // 12

                Label {
                    text: "Modbus connection settings"
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignHCenter
                }

                TextField {
                    id: hostField
                    enabled: modbusController.state === ModbusController.Disconnected
                    placeholderText: "IP address (e.g. 192.168.0.10)"
                    // Layout.preferredWidth: 250
                }

                TextField {
                    id: portField
                    enabled: modbusController.state === ModbusController.Disconnected
                    placeholderText: "Port (e.g. 502)"
                    inputMethodHints: Qt.ImhDigitsOnly
                    // Layout.preferredWidth: 250
                }

                TextField {
                    id: unitField
                    enabled: modbusController.state === ModbusController.Disconnected
                    placeholderText: "Unit ID"
                    inputMethodHints: Qt.ImhDigitsOnly
                    // Layout.preferredWidth: 250
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 140

                    text: {
                            switch (modbusController.state) {
                            case ModbusController.Disconnected:
                                return "Connect"
                            case ModbusController.Connecting:
                                return "Connecting..."
                            case ModbusController.Connected:
                                return "Disconnect"
                            case ModbusController.Error:
                                return "Retry"
                            }
                    }

                    enabled: modbusController.state !== ModbusController.Connecting

                    onClicked: {
                        switch (modbusController.state) {
                        case ModbusController.Disconnected:
                            modbusController.connectToServer(
                                            hostField.text,
                                            parseInt(portField.text),
                                            parseInt(unitField.text)
                            )
                            break;
                        case ModbusController.Connected:
                            modbusController.disconnectFromServer()
                            break;
                        case ModbusController.Error:
                                    // позже: retry
                            break;

                        }
                    }
                }
            }
        }

        // Bottom: Log panel. Takes all remaining space
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true // all space
            radius: 8
            color: "#111"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Header //title + clear button
                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "Log"
                        color: "lightgray"
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Clear"
                        onClicked: logModel.clear()
                    }
                }

                // List of msgs
                ListView {
                    id: logView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: logModel
                    clip: true

                    delegate: Text {
                        text: model.time + " " + model.text
                        color: "lightgreen"
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }

                    ScrollBar.vertical: ScrollBar { }
                }
            }
        }

        // Status Bar
        // Text {
        //     text: {
        //         switch (modbusController.state) {
        //         case ModbusController.Disconnected: return "🔌 Disconnected"
        //         case ModbusController.Connecting:   return "⏳ Connecting"
        //         case ModbusController.Connected:    return "✅ Connected"
        //         case ModbusController.Error:        return "❌ Error"
        //         }
        //     }
        // }
    }

    // Auto-scroll log to bottom
    Connections {
        target: logModel

        function onCountChanged() {
            if (logModel.count > 0) {
                logView.positionViewAtEnd()
            }
        }
    }
}
*/
