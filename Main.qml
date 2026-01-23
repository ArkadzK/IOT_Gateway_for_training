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

    ListModel { id: logModel }

    Connections {
        target: modbusController
        function onLogMessage(message) {
            logModel.append({
                time: Qt.formatTime(new Date(), "hh:mm:ss"),
                text: message
            })
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            radius: 8
            color: "#1b1b1f"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Label {
                    text: "Modbus connection settings"
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignHCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    TextField {
                        id: hostField
                        enabled: modbusController.state === ModbusController.Disconnected
                        placeholderText: "IP address"
                        Layout.fillWidth: true
                    }

                    TextField {
                        id: portField
                        enabled: modbusController.state === ModbusController.Disconnected
                        placeholderText: "Port"
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
                        Layout.preferredWidth: 100

                        text: {
                            switch (modbusController.state) {
                            case ModbusController.Disconnected: return "Connect"
                            case ModbusController.Connecting:   return "Connecting..."
                            case ModbusController.Connected:    return "Disconnect"
                            case ModbusController.Error:        return "Retry"
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
                                break

                            case ModbusController.Connected:
                                modbusController.disconnectFromServer()
                                break

                            case ModbusController.Error:
                                modbusController.connectToServer(
                                    hostField.text,
                                    parseInt(portField.text),
                                    parseInt(unitField.text)
                                )
                                break
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#111"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

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
    }

    Connections {
        target: logModel
        function onCountChanged() {
            logView.positionViewAtEnd()
        }
    }
}
