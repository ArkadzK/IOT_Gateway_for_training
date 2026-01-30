import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
// import ModbusMaster 1.0

import Backend 1.0

ApplicationWindow {
    visible: true
    width: 720
    height: 600
    title: "Modbus Master"

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    // Create service (or receive it from contextProperty)
    AppService {
        id: app
    }

    ListModel { id: logModel }
    ListModel { id: registersModel }
    ListModel { id: coilsModel }


    // Signals handler
    Connections {
        target: app
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

        function onCoilsRead(startAddress, values) {
            coilsModel.clear()

            for (var i = 0; i < values.length; ++i) {
                coilsModel.append({
                    address: startAddress + i,
                    value: values[i]
                })
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10 // отступы со всех сторон по 10 пикселей
        spacing: 12

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
                            text: "MQTT connection settings"
                            font.pixelSize: 16
                            Layout.alignment: Qt.AlignHCenter
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            TextField {
                                id: mqttHostField
                                placeholderText: "MQTT host"
                                Layout.fillWidth: true
                            }

                            TextField {
                                id: mqttPortField
                                placeholderText: "Port"
                                inputMethodHints: Qt.ImhDigitsOnly
                                Layout.preferredWidth: 80
                            }

                            ComboBox {
                                id: mqttQosField
                                Layout.preferredWidth: 100
                                model: [
                                    { text: "QoS 0", value: 0 },
                                    { text: "QoS 1", value: 1 },
                                    { text: "QoS 2", value: 2 }
                                ]
                                textRole: "text"
                            }

                            Button {
                                Layout.preferredWidth: 120
                                text: app.mqttConnected ? "Disconnect" : "Connect"

                                onClicked: {
                                    if (!app.mqttConnected) {
                                        var host = mqttHostField.text
                                        var port = parseInt(mqttPortField.text)
                                        var qos  = mqttQosField.model[mqttQosField.currentIndex].value

                                        if (!host || isNaN(port)) {
                                            logModel.append({
                                                time: Qt.formatTime(new Date(), "hh:mm:ss"),
                                                text: "Invalid MQTT parameters"
                                            })
                                            return
                                        }

                                        app.connectMqtt(host, port, qos)
                                    } else {
                                        app.disconnectMqtt()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Holding Registers + Coils side by side
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            spacing: 16

            // Holding Registers panel
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 160
                radius: 8
                color: "#1b1b1f"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 16
                    // Left panel (READ / WRITE)
                    ColumnLayout {
                        Layout.preferredWidth: 200
                        spacing: 8
                        Label {
                            text: "Holding registers"
                            font.pixelSize: 18
                        }

                        // READ BLOCK
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
                                    var cnt = parseInt(readCountField.text)

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

                        // WRITE BLOCK
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
                                    var val = parseInt(writeValueField.text)

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
                    }
                    // Right panel (table)
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: registersModel
                        clip: true
                        spacing: 1

                        delegate: Row {
                            height: 28

                            Rectangle {
                                width: 120
                                height: parent.height
                                color: (index % 2 === 0) ? "#222" : "#2a2a2a"
                                Text {
                                    anchors.centerIn: parent
                                    color: "white"
                                    text: address
                                }
                            }

                            Rectangle {
                                width: 120
                                height: parent.height
                                color: (index % 2 === 0) ? "#222" : "#2a2a2a"
                                Text {
                                    anchors.centerIn: parent
                                    color: "white"
                                    text: value
                                }
                            }
                        }
                    }
                }
            }

            // Coils panel
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 160
                radius: 8
                color: "#1b1b1f"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 16

                    // Left panel (READ / WRITE)
                    ColumnLayout {
                        Layout.preferredWidth: 200
                        spacing: 8

                        Label {
                            text: "Coils"
                            font.pixelSize: 18
                        }

                        // READ BLOCK
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            TextField {
                                id: readCoilsAddressField
                                placeholderText: "Start address"
                                inputMethodHints: Qt.ImhDigitsOnly
                                Layout.preferredWidth: 120
                            }

                            TextField {
                                id: readCoilsCountField
                                placeholderText: "Count"
                                inputMethodHints: Qt.ImhDigitsOnly
                                Layout.preferredWidth: 80
                            }

                            Button {
                                text: "Read"
                                enabled: modbusController.state === ModbusController.Connected

                                onClicked: {
                                    var addr = parseInt(readCoilsAddressField.text)
                                    var cnt = parseInt(readCoilsCountField.text)

                                    if (isNaN(addr) || isNaN(cnt) || cnt <= 0) {
                                        logModel.append({
                                            time: Qt.formatTime(new Date(), "hh:mm:ss"),
                                            text: "Invalid coil read parameters"
                                        })
                                        return
                                    }

                                    modbusController.readCoils(addr, cnt)
                                }
                            }
                        }

                        // WRITE BLOCK (single coil)
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            TextField {
                                id: writeCoilAddressField
                                placeholderText: "Address"
                                inputMethodHints: Qt.ImhDigitsOnly
                                Layout.preferredWidth: 120
                            }

                            ComboBox {
                                id: writeCoilValueField
                                Layout.preferredWidth: 120
                                model: ["false", "true"]
                            }

                            Button {
                                text: "Write"
                                enabled: modbusController.state === ModbusController.Connected

                                onClicked: {
                                    var addr = parseInt(writeCoilAddressField.text)
                                    var val = (writeCoilValueField.currentText === "true")

                                    if (isNaN(addr) || addr < 0) {
                                        logModel.append({
                                            time: Qt.formatTime(new Date(), "hh:mm:ss"),
                                            text: "Invalid coil write parameters"
                                        })
                                        return
                                    }

                                    modbusController.writeSingleCoil(addr, val)
                                }
                            }
                        }
                    }

                    // Right panel (table)
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: coilsModel
                        clip: true
                        spacing: 1

                        delegate: Row {
                            height: 28

                            Rectangle {
                                width: 120
                                height: parent.height
                                color: (index % 2 === 0) ? "#222" : "#2a2a2a"
                                Text {
                                    anchors.centerIn: parent
                                    color: "white"
                                    text: address
                                }
                            }

                            Rectangle {
                                width: 120
                                height: parent.height
                                color: (index % 2 === 0) ? "#222" : "#2a2a2a"

                                Switch {
                                    anchors.centerIn: parent
                                    checked: value

                                    onToggled: {
                                        modbusController.writeSingleCoil(address, checked)
                                    }
                                }
                            }
                        }
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
