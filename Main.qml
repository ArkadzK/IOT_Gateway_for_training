import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import Modbus 1.0

ApplicationWindow {
    visible: true
    width: 720
    height: 600
    title: "Modbus Master"

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    ListModel { id: logModel }
    ListModel { id: registersModel }

    // Signals handler
    Connections {
        target: modbusController
        function onLogMessage(message) {
            logModel.append({
                time: Qt.formatTime(new Date(), "hh:mm:ss"),
                text: message
            })
        }

        function onHoldingRegistersRead(startAddress, values) {
            registersModel.clear()

            for (var i = 0; i < values.length; ++i) {
                registersModel.append({
                    address: startAddress + i,
                    value: values[i]
                })
            }
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
                    font.pixelSize: 16
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
                                var host = hostField.text
                                var port = parseInt(portField.text)
                                var unit = parseInt(unitField.text)

                                if (!host || isNaN(port) || isNaN(unit)) {
                                    logModel.append({
                                        time: Qt.formatTime(new Date(), "hh:mm:ss"),
                                        text: "Invalid connection parameters"
                                    })
                                    return
                                }

                                modbusController.connectToServer(host, port, unit)
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
        // Holding Registers panel
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            radius: 8
            color: "#1b1b1f"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: "Holding registers"
                    font.pixelSize: 18
                }
                // READ block
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: readAddressField
                        placeholderText: "Start address"
                        inputMethodHints: Qt.ImhDigitsOnly
                        Layout.preferredWidth: 120
                    }

                    TextField {
                        id: readCountField
                        placeholderText: "Count"
                        inputMethodHints: Qt.ImhDigitsOnly
                        Layout.preferredWidth: 80
                    }

                    Button {
                        text: "Read"
                        enabled: modbusController.state === ModbusController.Connected

                        onClicked: {
                            var addr = parseInt(readAddressField.text)
                            var cnt  = parseInt(readCountField.text)
                            if (isNaN(addr) || isNaN(cnt) || cnt <= 0) {
                                logModel.append({
                                    time: Qt.formatTime(new Date(), "hh:mm:ss"),
                                    text: "Invalid read parameters"
                                })
                                return
                            }
                            modbusController.readHoldingRegisters(addr, cnt)
                        }
                    }
                }
                // WRITE block
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: writeAddressField
                        placeholderText: "Address"
                        inputMethodHints: Qt.ImhDigitsOnly
                        Layout.preferredWidth: 120
                    }

                    TextField {
                        id: writeValueField
                        placeholderText: "Value"
                        inputMethodHints: Qt.ImhDigitsOnly
                        Layout.preferredWidth: 120
                    }

                    Button {
                        text: "Write"
                        enabled: modbusController.state === ModbusController.Connected

                        onClicked: {
                            var addr = parseInt(writeAddressField.text)
                            var val  = parseInt(writeValueField.text)
                            if (isNaN(addr) || addr < 0 || isNaN(val)) {
                                logModel.append({
                                    time: Qt.formatTime(new Date(), "hh:mm:ss"),
                                    text: "Invalid write parameters"
                                })
                                return
                            }
                            modbusController.writeHoldingRegister(addr, val)
                        }
                    }
                }
                // REGISTERS list
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: registersModel
                    clip: true

                    delegate: Text {
                        text: "HR[" + address + "] = " + value
                        color: "lightblue"
                        font.pixelSize: 12
                    }
                }
            }
        }
        // LOG panel
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
    // Auto-scroll log
    Connections {
        target: logModel
        function onCountChanged() {
            logView.positionViewAtEnd()
        }
    }
}
