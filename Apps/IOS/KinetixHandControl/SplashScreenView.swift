//
//  SplashScreenView.swift
//  KinetixHandControl
//
//  Created by Mikael Alhadeff on 05/07/2024.
//

import SwiftUI

struct SplashScreenView: View {
    @State private var isActive = false
    
    var body: some View {
        if isActive {
            ContentView()
        }
        else
        {
            Image("SplashScreen")
                .resizable()
                .scaledToFill()
                .edgesIgnoringSafeArea(.all)
                .onAppear {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
                        withAnimation {
                            self.isActive = true
                        }
                    }
                }
        }
        
    }
}

#Preview {
    SplashScreenView()
}
