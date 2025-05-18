//
//  Kinetix.swift
//  KinetixHandControl
//
//  Created by Mikael Alhadeff on 12/07/2024.
//

import Foundation
import CoreBluetooth

class KinetixPeripheral: NSObject {

    /// MARK: - Particle LED services and charcteristics Identifiers

    public static let kinetixServiceUUID     = CBUUID.init(string: "89d60870-9908-4472-8f8c-e5b3e6573cd1")
    public static let movementCharacteristicUUID   = CBUUID.init(string: "39dea685-a63e-44b2-8819-9a202581f8fe")
    public static let configCharacteristicUUID = CBUUID.init(string: "b2a49d41-a2ac-48c3-b6c8-cfd05640654e")

}
