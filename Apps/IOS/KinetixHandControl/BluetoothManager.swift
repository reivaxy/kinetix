//
//  BluetoothManager.swift
//  KinetixHandControl
//
//  Created by Mikael Alhadeff on 12/07/2024.
//

import Foundation
import CoreBluetooth
import SwiftUI

class BluetoothManager: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var isBluetoothEnabled = false
    @Published var discoveredPeripherals: [CBPeripheral] = []
    @Published var connectedPeripheral: CBPeripheral?
    @Published var discoveredServices: [CBService] = []
    @Published var discoveredCharacteristics: [CBCharacteristic] = []
    @Published var movementCharacteristic: CBCharacteristic?
    
    private var centralManager: CBCentralManager!
    
    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    // Look for available Kinetix bluetooth devices
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            isBluetoothEnabled = true
            centralManager.scanForPeripherals(withServices: [KinetixPeripheral.kinetixServiceUUID], options: nil)
        default:
            isBluetoothEnabled = false
        }
    }
    
    // Add found devices to the discoveredPeripherals list
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if !discoveredPeripherals.contains(peripheral) {
            discoveredPeripherals.append(peripheral)
        }
    }
    
    // Connect to a specific device (will be stored as connectedPeripheral)
    func connect(to peripheral: CBPeripheral) {
        centralManager.stopScan()
        centralManager.connect(peripheral, options: nil)
        connectedPeripheral = peripheral
        peripheral.delegate = self
    }
    
    // Discover Kinetix service when connecting to the device
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        peripheral.discoverServices([KinetixPeripheral.kinetixServiceUUID])
    }
    
    // Discover Kinetix characteristics when connecting to the device
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let services = peripheral.services {
            discoveredServices = services
            for service in services {
                peripheral.discoverCharacteristics([KinetixPeripheral.movementCharacteristicUUID, KinetixPeripheral.configCharacteristicUUID], for: service)
            }
        }
    }
    
    // Add found characteristics to the discoveredCharacteristics list
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let characteristics = service.characteristics {
            discoveredCharacteristics.append(contentsOf: characteristics)
            
            for characteristic in characteristics {
                print("Found characteristic: \(characteristic.uuid)")
                
                if (characteristic.properties.contains(.broadcast)) {
                    print("Can broadcast")
                }
                    
                if (characteristic.properties.contains(.read)) {
                    print("Can read")
                }
                        
                if (characteristic.properties.contains(.writeWithoutResponse)) {
                    print("Can write without response")
                }
                
                if (characteristic.properties.contains(.write)) {
                    print("Can write")
                }
                
                if (characteristic.properties.contains(.notify)) {
                    print("Can notify")
                }
                
                if (characteristic.properties.contains(.indicate)) {
                    print("Can indicate")
                }
                
                if (characteristic.uuid == KinetixPeripheral.configCharacteristicUUID) {
                    // Get current config value
                    peripheral.readValue(for: characteristic)
                }
                
                if (characteristic.uuid == KinetixPeripheral.movementCharacteristicUUID) {
                    movementCharacteristic = characteristic
                }
            }
        }
    }
    
    // Get notified that the new characteristic value is ready to be read
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        let newValue = String(data: characteristic.value!, encoding: .utf8)
        print("New value for \(characteristic.uuid): \(newValue!)")
    }
    
    // Write a value to the specified characteristic
    func writeValueToChar( withCharacteristic characteristic: CBCharacteristic, withValue value: Data) {
        // Check if it has the write property
        if connectedPeripheral != nil {
            connectedPeripheral?.writeValue(value, for: characteristic, type: .withResponse)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        print("Value written")
    }
    
    
    func readValueFromChar( withCharacteristic characteristic: CBCharacteristic) {
        // Check if it has the write property
        if connectedPeripheral != nil {
            connectedPeripheral?.readValue(for: characteristic)
        }
    }
}
