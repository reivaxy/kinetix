//
//  ContentView.swift
//  KinetixHandControl
//
//  Created by Mikael Alhadeff on 05/07/2024.
//

import SwiftUI

struct ContentView: View {
    
    @ObservedObject var bluetoothManager = BluetoothManager()
    
    var body: some View {
        VStack {
            if bluetoothManager.isBluetoothEnabled {
                List(bluetoothManager.discoveredPeripherals, id: \.identifier) { peripheral in
                    Button(action: {
                        bluetoothManager.connect(to: peripheral)
                    }) {
                        Text(peripheral.name ?? "Unknown")
                    }
                    
                    if let connectedPeripheral = bluetoothManager.connectedPeripheral {
                        Section(header: Text("Services for \(connectedPeripheral.name ?? "Unknown")")) {
                            ForEach(bluetoothManager.discoveredServices, id: \.uuid) { service in
                                VStack(alignment: .leading) {
                                    Text("Service: \(service.uuid)")
                                    ForEach(service.characteristics ?? [], id: \.uuid) { characteristic in
                                        Text("Characteristic: \(characteristic.uuid)")
                                    }
                                }
                                .padding(.leading)
                            }
                        }
                        
                        Button(action: {
                            let command = "fist"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Fist")
                        }
                        
                        Button(action: {
                            let command = "five"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Open")
                        }
                                              
                        Button(action: {
                            let command = "openPinch"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Open Pinch")
                        }
                        
                        Button(action: {
                            let command = "closePinch"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Close Pinch")
                        }
                        
                        Button(action: {
                            let command = "one"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("One")
                        }
                        
                        Button(action: {
                            let command = "two"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Two")
                        }
                        
                        Button(action: {
                            let command = "three"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Three")
                        }
                        
                        Button(action: {
                            let command = "four"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Four")
                        }
                        
                        Button(action: {
                            let command = "ok"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Ok")
                        }
                        
                        Button(action: {
                            let command = "scratch"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Scratch")
                        }
                        
                        Button(action: {
                            let command = "come"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Come")
                        }
                        
                        Button(action: {
                            let command = "rock"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Rock")
                        }
                        
                        Button(action: {
                            let command = "love"
                            var commandData = command.data(using: .utf8)
                            
                            bluetoothManager.writeValueToChar(withCharacteristic: bluetoothManager.movementCharacteristic!, withValue: commandData!)
                        }) {
                            Text("Love")
                        }
                    }
                }
            } else {
                Text("Bluetooth is not enabled")
            }
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
